#include "wsjtx_handler.h"
#include "status.h"
#include "menu.h"
#include "utils.h"
#include "dxa_if.h"
#include "prefix.h"
#include "pfx_data.h"
#include "toolbar.h"
#include "qso_manager.h"
#include "adi_reader.h"

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#ifdef _WIN32
#include <WS2tcpip.h>
#else
//#include <fnctl.h>
#endif

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>



using namespace std;

extern status* status_;
extern void cb_error_message(status_t level, const char* message);
extern menu* menu_;
extern Fl_Preferences* settings_;
#ifdef _WIN32
extern dxa_if* dxa_if_;
#endif
extern pfx_data* pfx_data_;
extern toolbar* toolbar_;
extern qso_manager* qso_manager_;
extern string PROGRAM_ID;
extern string VERSION;

wsjtx_handler* wsjtx_handler::that_ = nullptr;

// Constructor: 
wsjtx_handler::wsjtx_handler()
{
	that_ = this;
	server_ = nullptr;
	qso_ = nullptr;
	run_server();
	new_heartbeat_ = false;
	status_rcvd_ = 0;
	// Get logged callsign from spec_data
	my_call_ = qso_manager_->get_default(qso_manager::CALLSIGN);
	my_bracketed_call_ = "<" + my_call_ + ">";
}

// Destructor
wsjtx_handler::~wsjtx_handler() {
	if (server_) {
		server_->close_server();
		delete server_;
	}
};

// This callback must be static
int wsjtx_handler::rcv_request(stringstream& ss) {
	return that_->rcv_dgram(ss);
}

// Receive the datagram. Decide which one an go to the individual decode methods
int wsjtx_handler::rcv_dgram(stringstream & ss) {
	// The first few objects are fixed for all datagrams
	magic_number_ = get_uint32(ss);
	schema_ = get_uint32(ss);
	uint32_t dgram_type = get_uint32(ss);
	// Check the magic number and schema are supported
	if (magic_number_ != expected_magic_ || schema_ < minimum_schema_) {
		char message[256];
		snprintf(message, 256, "datagram had wrong magic number (%08x) or unsupported schema (%d)", magic_number_, schema_);
		status_->misc_status(ST_WARNING, message);
		return 1;
	}
	// Select method to interpret datagram
	switch (dgram_type) {
	case 0:
		// Heartbeat
		return handle_hbeat(ss);
	case 1:
		// Status
		return handle_status(ss);
	case 2:
		return handle_decode(ss);
	case 6:
		return handle_close(ss);
	case 12:
		return handle_log(ss);
	default:
		return handle_default(ss, dgram_type);
	}
	return 0;
}

// Ignore the datagrams that do not interest us
int wsjtx_handler::handle_default(stringstream& ss, uint32_t type) {
	char message[128];
	snprintf(message, 128, "WSJT-X: Ignored type %d datagram", type);
	status_->misc_status(ST_LOG, message);
	return 0;
}

// Handle the heartbeat - send one back
int wsjtx_handler::handle_hbeat(stringstream& ss) {
	new_heartbeat_ = true;
	send_hbeat();
	return 0;
}

// Send a heartbeat
int wsjtx_handler::send_hbeat() {
	stringstream ss;
	// Add the magic number, schema and function number
	put_uint32(ss, magic_number_);
	put_uint32(ss, schema_);
	put_uint32(ss, 0);
	// Add the ID
	put_utf8(ss, PROGRAM_ID);
	// Add the max schema
	put_uint32(ss, 3);
	// Add the version
	put_utf8(ss, VERSION);
	// Add the revision ""
	put_uint32(ss, (uint32_t)(~0));
	
	// Now go back to the start of the stream to send it
	ss.seekg(0, ios::beg);
	return server_->send_response(ss);
}

// Close datagram: shut the server down 
int wsjtx_handler::handle_close(stringstream& ss) {
	status_->misc_status(ST_NOTE, "WSJT-X: Received Closing down");
#ifdef _WIN32
	dxa_if_->clear_dx_loc();
#endif
	menu_->update_items();
	return 1;
}

// Handle the logged ADIF datagram. Send it to the logger
int wsjtx_handler::handle_log(stringstream& ss) {
	status_->misc_status(ST_LOG, "WSJT-X: Received Log ADIF datagram");
	// Ignore Id filed
	string utf8 = get_utf8(ss);
	// Get ADIF string
	utf8 = get_utf8(ss);
	// Convert it to a record
	stringstream adif;
	adif.str(utf8);
	adi_reader* reader = new adi_reader;
	// The stream received from WSJT-X is header and record so create a book from it
	book* rcvd_book = new book();
	if (qso_) {
		reader->load_book(rcvd_book, adif);
		record* log_qso = rcvd_book->get_record(0, false);
		qso_->merge_records(log_qso);
		qso_->item("QSO_COMPLETE", string(""));
		qso_manager_->update_modem_qso(qso_);
		status_->misc_status(ST_NOTE, "WSJT-X: Logged QSO");
		delete rcvd_book;
		qso_ = nullptr;
	}
#ifdef _WIN32
	// Clear DX locator flag
	dxa_if_->clear_dx_loc();
#endif
	return 0;
}

// handle decode - display and beep if it contains the user's callsign
int wsjtx_handler::handle_decode(stringstream& ss) {
	decode_dg decode;
	decode.id = get_utf8(ss);
	decode.new_decode = get_bool(ss);
	decode.time = get_uint32(ss);
	decode.snr = get_uint32(ss);
	decode.d_time = get_double(ss);
	decode.d_freq = get_uint32(ss);
	decode.mode = get_utf8(ss);
	decode.message = get_utf8(ss);
	decode.low_confidence = get_bool(ss);
	decode.off_air = get_bool(ss);
	// display ID, time and message
	double seconds = decode.time / 1000.0;
	unsigned int minutes = (unsigned int)seconds / 60;
	seconds = seconds - (minutes * 60.0);
	unsigned int hours = minutes / 60;
	minutes = minutes - (hours * 60);
	char message[256];
	snprintf(message, 256, "WSJT-X: Decode %02d:%02d:%02.0f: %s", hours, minutes, seconds, decode.message.c_str());
	add_rx_message(decode);
	return 0;
}

// handle status - display it if the DX Call has changed - indicates that user
// has called a new station
int wsjtx_handler::handle_status(stringstream& ss) {
	// Debug code
	string datagram = ss.str();

	status_dg status;
	// ID
	status.id = get_utf8(ss);
	// Frequency
	status.dial_freq = get_uint64(ss);
	// Mode
	status.mode = get_utf8(ss);
	// DX Call
	status.dx_call = get_utf8(ss);
	// Report 
	status.report = get_utf8(ss);
	// Transmit mode
	status.tx_mode = get_utf8(ss);
	// Transmit enabled
	status.tx_eanbled = get_bool(ss);
	// Transmit
	status.transmitting = get_bool(ss);
	// Decoding
	status.decoding = get_bool(ss);
	// Received frequency - delta from VFO
	status.rx_offset = get_uint32(ss);
	// Transmit frequency
	status.tx_offset = get_uint32(ss);
	// My call
	status.own_call = get_utf8(ss);
	// My grid
	status.own_grid = get_utf8(ss);
	// DX Grid
	status.dx_grid = get_utf8(ss);
	// Transmit ?
	status.tx_watchdog = get_bool(ss);
	// Submode
	status.submode = get_utf8(ss);
	// FAst decode
	status.fast_mode = get_bool(ss); 
	// Special operation mode
	status.special_op = get_uint8(ss); 
	// Frequency tolerance
	status.freq_tolerance = get_uint32(ss); 
	// ????
	status.tx_rx_period = get_uint32(ss);
	// Configuration name
	status.config_name = get_utf8(ss); 
	// TX Message
	status.tx_message = get_utf8(ss);
	// Create qso
	add_tx_message(status);
#ifdef _WIN32
	if (dxa_if_) {
		if (status.dx_call.length() && status.dx_grid.length() && status.transmitting) {
			// Use the actual grid loaction - and put it into the cache
			dxa_if_->set_dx_loc(status.dx_grid, status.dx_call);
			grid_cache_[status.dx_call] = status.dx_grid;
			toolbar_->search_text(status.dx_call);
		}
		else if (status.dx_call.length() && status.transmitting) {
			// Look in location cache
			if (grid_cache_.find(status.dx_call) != grid_cache_.end()) {
				// Use the remembered grid loaction
				dxa_if_->set_dx_loc(grid_cache_[status.dx_call], status.dx_call);
				toolbar_->search_text(status.dx_call);
			}
			else {
				dxa_if_->set_dx_loc(status.dx_call);
				toolbar_->search_text(status.dx_call);
			}
		}
		else if (!status.dx_call.length()) {
			// Can clear the Dx Location by clearing the DX Call field
			dxa_if_->clear_dx_loc();
		}
	}
#endif
	prev_status_ = status;
	return 0;
}

// Get an bool from the next byte of datagram 
bool wsjtx_handler::get_bool(stringstream& ss) {
	unsigned char c;
	ss.get((char&)c);
	bool b = c ? true : false;
	return b;
}

// Get an unsigned integer from the next byte of datagram 
uint8_t wsjtx_handler::get_uint8(stringstream& ss) {
	unsigned char c;
	ss.get((char&)c);
	uint8_t i = c;
	return i;
}

// Get an unsigned integer from the next 4 bytes of dgram 
uint32_t wsjtx_handler::get_uint32(stringstream& ss) {
	unsigned char c = 0;
	uint32_t i = 0;
	for (int ix = 0; ix < 4; ix++) {
		ss.get((char&)c);
		i = (i << 8) + c;
	}
	return i;
}

// Get 64-bit unsigned integer from the next 8 bytes of datagram 
uint64_t wsjtx_handler::get_uint64(stringstream& ss) {
	unsigned char c = 0;
	uint64_t i = 0LL;
	for (int ix = 0; ix < 8; ix++) {
		ss.get((char&)c);
		i = (i << 8) + c;
	}
	return i;
}

// Get double from the next 8-bytes of datagram
double wsjtx_handler::get_double(stringstream& ss) {
	// I'm making the assumption that the double has been directly serialised in its bit pattern
	uint64_t uv = get_uint64(ss);
	double* d = reinterpret_cast<double*>(&uv);
	return *d;
}

// Get a string from the QByteArray (4 byte length + that number of bytes)
string wsjtx_handler::get_utf8(stringstream& ss) {
	// Get 4-byte length
	uint32_t len = get_uint32(ss);
	if (len == ~(0)) {
		// Length is all 1's - represents a null string
		return "";
	}
	else {
		// Create a string long enough to receive the data
		string s = "";
		s.reserve(len + 1);
		for (uint32_t i = 0; i < len; i++) {
			// Copy the string 1 byte at a timme
			char c;
			ss.get(c);
			s += c;
		}
		return s;
	}
}

// Put an integer as 4 bytes in the next 4 bytes of dgram
void wsjtx_handler::put_uint32(stringstream& ss, uint32_t i) {
	for (int ix = 24; ix >= 0; ix-=8) {
		unsigned char c = (i >> ix) & 0xFF;
		ss.put(c);
	}
}

// Put a string as QByteArray (see above) into the dgram
void wsjtx_handler::put_utf8(stringstream& ss, string s) {
	put_uint32(ss, s.length());
	for (unsigned int ix = 0; ix < s.length(); ix++) {
		ss.put(s[ix]);
	}
}

// Return that the server is there
bool wsjtx_handler::has_server() {
	return (server_ != nullptr && server_->has_server());
}

// Start the server
void wsjtx_handler::run_server() {
	if (!server_) {
		status_->misc_status(ST_NOTE, "WSJT-X: Creating new socket");
		server_ = new socket_server(socket_server::UDP, 2237);
		server_->callback(rcv_request, cb_error_message);
	}
	if (!server_->has_server()) {
		status_->misc_status(ST_NOTE, "WSJT-X: Starting socket");
		server_->run_server();
	}
	menu_->update_items();
}

// Close the server
void wsjtx_handler::close_server() {
	if (server_) {
		status_->misc_status(ST_OK, "WSJT-X: Application closing");
	}
}

void wsjtx_handler::add_tx_message(const status_dg& status) {
	if (status.transmitting) {
		// If the call changes use a different record - qso_data will handle it
		if (qso_ == nullptr || qso_->item("CALL") != status.dx_call) {
			if (qso_ && qso_->item("QSO_COMPLETE") == "Y") qso_->item("QSO_COMPLETE", string(""));
			qso_ = new record();
		}
		// Can get all the required fields off status
		qso_->item("CALL", status.dx_call);
		qso_->item("GRIDSQUARE", status.dx_grid);
		double freq = (status.dial_freq + status.tx_offset) / 1000000.0;
		char cfreq[15];
		snprintf(cfreq, sizeof(cfreq), "%0.6f", freq);
		qso_->item("FREQ", string(cfreq));
		qso_->item("MODE", status.mode);
		qso_->item("RST_SENT", status.report);
		qso_->item("STATION_CALLSIGN", status.own_call);
		if (check_message(qso_, status.tx_message, true)) {
			qso_manager_->update_modem_qso(qso_);
			if (qso_->item("QSO_COMPLETE").length() == 0) {
				qso_ = nullptr;
			}
		}
		else {
			char msg[128];
			snprintf(msg, sizeof(msg), "WSJT-X: Mismatch in decoding status for %s", status.dx_call.c_str());
			status_->misc_status(ST_WARNING, msg);
		}
	}
}

void wsjtx_handler::add_rx_message(const decode_dg& decode) {
	// Parse the message
	vector<string> words;
	split_line(decode.message, words, ' ');
	if (words[0] != my_call_ && words[0] != my_bracketed_call_) {
		// Not interested as its not for me
		return;
	}

	const string& call = words[1];
	if (qso_ == nullptr) {
		// Someone is calling me - wait until I reply
		return;
	}

	if (check_message(qso_, decode.message, false)) {
		qso_manager_->update_modem_qso(qso_);
		if (qso_->item("QSO_COMPLETE").length() == 0) {
			qso_ = nullptr;
		}
	}
	else {
		char msg[128];
		snprintf(msg, sizeof(msg), "WSJT-X: Mismatch in decoding decode for %s", call.c_str());
		status_->misc_status(ST_WARNING, msg);
	}
}

bool wsjtx_handler::check_message(record* qso, string message, bool tx) {
	// Now parse the exchange
	vector<string> words;
	split_line(message, words, ' ');
	string report = words.back();
	while (report.length() == 0) {
		words.pop_back();
		report = words.back();
	}
	string call = words[0];
	string qso_call = qso->item("CALL");
	if (call != qso_call && call != ("<" + qso_call + ">") && call != "CQ") {
		return false;
	}
	if (report == "RR73" || report == "RRR") {
		// If we've seen the R-00 then mark the QSO complete, otherwise mark in provisional until we see the 73
		if (qso->item("QSO_COMPLETE") == "?") {
			qso->item("QSO_COMPLETE", string("Y"));
		}
		else if (qso->item("QSO_COMPLETE") == "N") {
			qso->item("QSO_COMPLETE", string("?"));
		}
	}
	else if (report == "73") {
		// A 73 definitely indicates QSO compplete
		qso->item("QSO_COMPLETE", string(""));
	}
	else if (report[0] == 'R') {
		// The first of the rogers
		if (qso->item("QSO_COMPLETE") == "N") {
			qso->item("QSO_COMPLETE", string("?"));
		}
		// Update reports if they've not been provided
		if (tx && !qso->item_exists("RST_SENT")) {
			qso->item("RST_SENT", report.substr(1));
		}
		else if (tx && qso->item("RST_SENT") != report.substr(1)) {
			char msg[128];
			snprintf(msg, sizeof(msg), "WSJTX: Mismatch for %s RST_SENT is changing", qso->item("CALL").c_str());
			status_->misc_status(ST_WARNING, msg);
			return false;
		}
		else if (!tx && !qso->item_exists("RST_RCVD")) {
			qso->item("RST_RCVD", report.substr(1));
		}
	}
	else if (!tx && !qso->item_exists("GRIDSQUARE")) {
		// Update gridsquare if it's not been provided
		qso->item("GRIDSQUARE", report);
	}
	else if (!tx && qso->item("GRIDSQUARE") != report) {
		char msg[128];
		snprintf(msg, sizeof(msg), "WSJTX: Mismatch for %s GRIDSQUARE is changing", qso->item("CALL").c_str());
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	else if (report[0] == '-' || report[0] == '+' || (report[0] >= '0' && report[0] <= '9')) {
		// Numeric report
		if (tx && !qso->item_exists("RST_SENT")) {
			qso->item("RST_SENT", report);
		}
		else if (!tx && !qso->item_exists("RST_RCVD")) {
			qso->item("RST_RCVD", report);
		}
	}
	else {
		qso->item("QSO_COMPLETE", string("N"));
	}
	return true;
}