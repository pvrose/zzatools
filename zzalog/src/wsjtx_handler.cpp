#include "wsjtx_handler.h"
#include "status.h"
#include "menu.h"
#include "utils.h"
#include "dxa_if.h"
#include "prefix.h"
#include "toolbar.h"
#include "qso_manager.h"
#include "adi_reader.h"
#include "menu.h"
#include "regices.h"
#include "spec_data.h"

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
extern toolbar* toolbar_;
extern qso_manager* qso_manager_;
extern spec_data* spec_data_;
extern string PROGRAM_ID;
extern string VERSION;

wsjtx_handler* wsjtx_handler::that_ = nullptr;

// Constructor: 
wsjtx_handler::wsjtx_handler()
{
	that_ = this;
	server_ = nullptr;
	qsos_.clear();
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
	// Clear any WSJT-X related items
#ifdef _WIN32
	if (dxa_if_) dxa_if_->clear_dx_loc();
#endif
	qso_manager_->update_modem_qso(nullptr);
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
	book* rcvd_book = new book(OT_NONE);
	reader->load_book(rcvd_book, adif);
	record* log_qso = rcvd_book->get_record(0, false);
	record* qso = qso_call(log_qso->item("CALL"));
	qso->merge_records(log_qso);
	qso->item("QSO_COMPLETE", string(""));
	qso_manager_->update_modem_qso(qso);
	status_->misc_status(ST_NOTE, "WSJT-X: Logged QSO");
	delete rcvd_book;
#ifdef _WIN32
	// Clear DX locator flag
	if (dxa_if_) dxa_if_->clear_dx_loc();
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
	char t[20];
	snprintf(t, sizeof(t), "%02d%02d%02.0f", hours, minutes, seconds);
	record* qso = update_qso(false, string(t), (double)decode.d_freq, decode.message);
	if (qso) qso_manager_->update_modem_qso(qso);
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
	// Save frequency and mode
	dial_frequency_ = (double)status.dial_freq / 1000000.0;
	mode_ = status.mode;
	// Create qso
	if (status.transmitting) {
		record* qso = update_qso(true, now(false, "%H%M%S"), (double)status.tx_offset, status.tx_message);
		if (qso) qso_manager_->update_modem_qso(qso);
	}
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

//void wsjtx_handler::add_tx_message(const status_dg& status) {
//	if (status.transmitting && status.dx_call.length()) {
//		// If the call changes use a different record - qso_data will handle it
//		if (qso_ == nullptr || qso_->item("CALL") != status.dx_call) {
//			if (qso_ && qso_->item("QSO_COMPLETE") == "Y") qso_->item("QSO_COMPLETE", string(""));
//			qso_ = new record();
//			qso_->item("QSO_COMPLETE", string("N"));
//		}
//		// Can get all the required fields off status
//		qso_->item("CALL", status.dx_call);
//		qso_->item("GRIDSQUARE", status.dx_grid);
//		double freq = (status.dial_freq + status.tx_offset) / 1000000.0;
//		char cfreq[15];
//		snprintf(cfreq, sizeof(cfreq), "%0.6f", freq);
//		qso_->item("FREQ", string(cfreq));
//		qso_->item("MODE", status.mode);
//		//qso_->item("RST_SENT", status.report);
//		qso_->item("STATION_CALLSIGN", status.own_call);
//
//		if (check_message(qso_, status.tx_message, true)) {
//			qso_manager_->update_modem_qso(qso_);
//			if (qso_->item("QSO_COMPLETE").length() == 0) {
//				qso_ = nullptr;
//			}
//		}
//		else {
//			char msg[128];
//			snprintf(msg, sizeof(msg), "WSJT-X: Mismatch in decoding status for %s", status.dx_call.c_str());
//			status_->misc_status(ST_WARNING, msg);
//		}
//	}
//}
//
//void wsjtx_handler::add_rx_message(const decode_dg& decode) {
//	// Parse the message
//	vector<string> words;
//	split_line(decode.message, words, ' ');
//	if (words[0] != my_call_ && words[0] != my_bracketed_call_) {
//		// Not interested as its not for me
//		return;
//	}
//
//	const string& call = words[1];
//	if (qso_ == nullptr) {
//		// Someone is calling me - wait until I reply
//		return;
//	}
//
//	if (check_message(qso_, decode.message, false)) {
//		qso_manager_->update_modem_qso(qso_);
//		if (qso_->item("QSO_COMPLETE").length() == 0) {
//			qso_ = nullptr;
//		}
//	}
//	else {
//		char msg[128];
//		snprintf(msg, sizeof(msg), "WSJT-X: Mismatch in decoding decode for %s", call.c_str());
//		status_->misc_status(ST_WARNING, msg);
//	}
//}
//
//bool wsjtx_handler::check_message(record* qso, string message, bool tx) {
//	// Now parse the exchange - 
//	// TODO: Currently assuming non F/H exchange
//	string exchange = "";
//	string sender = "";
//	string target = "";
//	int state = 0;
//	// exchange is the last word, sender last but one and target the rest
//	for (auto it = message.rbegin(); it != message.rend(); it++) {
//		switch (state) {
//		case 0: 
//			if (*it!= ' ') {
//				state = 1;
//				exchange = *it;
//			}
//			break;
//		case 1:
//			if (*it == ' ') {
//				state = 2;
//			} else {
//				exchange = *it + exchange;
//			}
//			break;
//		case 2: 
//			if (*it!= ' ') {
//				state = 3;
//				sender = *it;
//			}
//			break;
//		case 3:
//			if (*it == ' ') {
//				state = 4;
//			} else {
//				sender = *it + sender;
//			}
//			break;
//		case 4: 
//			if (*it!= ' ') {
//				state = 5;
//				target = *it;
//			}
//			break;
//		case 5:
//			target = *it + target;
//			break;
//		}
//	}
//	// Sort out my call
//	if (tx) {
//		// Not me sending - should not happen
//		if (sender != my_call_) {
//			return false;
//		}
//		// Target is CQ
//		if (target.substr(0, 2) == "CQ") {
//			if (qso_ && qso_->item("CALL").length()) {
//				// A CQ is replacing a call to another station so replace that call with null string
//				qso_->item("CALL", string(""));
//			}
//			else {
//				// Ignore single CQs
//				return false;
//			}
//		}
//		// Over with another station
//		else {
//			// Set QSO->CALL to the target field - replacing cached callsigns eg "<GM3ZZA>" with "GM3ZZA"
//			string their_call = qso->item("CALL");
//			if (target[0] == '<') {
//				target = target.substr(1, target.length() - 2);
//			}
//			if (their_call != target) {
//				char msg[128];
//				snprintf(msg, sizeof(msg), "WSJT-X: Changing target call from %s to %s", their_call.c_str(), target.c_str());
//				status_->misc_status(ST_WARNING, msg);
//			}
//			// Set the QSO call to the target
//			qso->item("CALL", target);
//		}
//	}
//	else {
//		// Am I the target?
//		if (target != my_call_ && target != my_bracketed_call_) {
//			// return false if not
//			return false;
//		}
//		// save sender
//		qso->item("CALL", sender);
//	}
//	if (exchange == "RR73" || exchange == "RRR") {
//		// If we've seen the R-00 then mark the QSO complete, otherwise mark in provisional until we see the 73
//		if (qso->item("QSO_COMPLETE") == "?") {
//			// It's complete as far as I am concerned, but partner may disagree - leave QSO active
//			qso->item("QSO_COMPLETE", string("Y"));
//		}
//		else if (qso->item("QSO_COMPLETE") == "N") {
//			qso->item("QSO_COMPLETE", string("?"));
//		}
//	}
//	else if (exchange == "73") {
//		// A 73 definitely indicates QSO compplete - but only qso_manager can clear it on logging event
//		qso->item("QSO_COMPLETE", string("Y"));
//	}
//	else if (exchange[0] == 'R') {
//		// The first of the rogers
//		if (qso->item("QSO_COMPLETE") == "N") {
//			// Half the exchange is complete
//			qso->item("QSO_COMPLETE", string("?"));
//		}
//		// Update reports if they've not been provided 
//		if (tx && !qso->item_exists("RST_SENT")) {
//			qso->item("RST_SENT", exchange.substr(1));
//		}
//		else if (tx && qso->item("RST_SENT") != exchange.substr(1)) {
//			char msg[128];
//			snprintf(msg, sizeof(msg), "WSJTX: Mismatch for %s RST_SENT is changing from %s to %s", 
//				qso->item("CALL").c_str(), qso->item("RST_SENT").c_str(), exchange.c_str());
//			status_->misc_status(ST_WARNING, msg);
//			return false;
//		}
//		else if (!tx && !qso->item_exists("RST_RCVD")) {
//			qso->item("RST_RCVD", exchange.substr(1));
//		}
//	}
//	else if (!tx && !qso->item_exists("GRIDSQUARE")) {
//		// Update gridsquare if it's not been provided
//		qso->item("GRIDSQUARE", exchange);
//	}
//	else if (!tx && qso->item("GRIDSQUARE") != exchange) {
//		char msg[128];
//		snprintf(msg, sizeof(msg), "WSJTX: Mismatch for %s GRIDSQUARE is changing from %s to %s",
//			qso->item("CALL").c_str(), qso->item("GRIDSQUARE").c_str(), exchange.c_str());
//		status_->misc_status(ST_WARNING, msg);
//		return false;
//	}
//	else if (exchange[0] == '-' || exchange[0] == '+' || isdigit(exchange[0])) {
//		// Numeric report
//		if (tx && !qso->item_exists("RST_SENT")) {
//			qso->item("RST_SENT", exchange);
//		}
//		else if (!tx && !qso->item_exists("RST_RCVD")) {
//			qso->item("RST_RCVD", exchange);
//		}
//		qso->item("QSO_COMPLETE", string("N"));
//	}
//	else {
//		qso->item("QSO_COMPLETE", string("N"));
//	}
//	return true;
//}

wsjtx_handler::decoded_msg wsjtx_handler::decode_message(string message) {
	// TODO: Currently assuming non F/H exchange
	decoded_msg decode;
	vector<string> words;
	// Prune the trailing spaces from message
	size_t ix = message.length();
	while(message[--ix] == ' ');
	string pruned = message.substr(0, ix+1);
	split_line(pruned, words, ' ');

	string test_exch = words.back();
	const char* test = test_exch.c_str();
	if (regex_match(test, REGEX_GRIDSQUARE4)) {
		// TX1 or TX6
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		if (regex_match(decode.target.c_str(), REGEX_CQ)) {
			decode.type = TX6;
		}
		else {
			decode.type = TX1;
		}
	}
	else if (regex_match(test, REGEX_REPORT)) {
		// TX2
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX2;
	}
	else if (regex_match(test, REGEX_ROGER)) {
		// TX3
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX3;
	}
	else if (strcmp(test, "RRR") == 0) {
		// TX4
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX4;
	}
	else if (strcmp(test, "RR73") == 0) {
		// TX4A
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX4A;
	}
	else if (strcmp(test, "73") == 0) {
		// TX5
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX5;
	}
	else {
		// No exchange - unusual call either TX1A or TX6A
		decode.exchange = "";
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		if (regex_match(decode.target.c_str(), REGEX_CQ)) {
			decode.type = TX6A;
		}
		else {
			decode.type = TX1A;
		}
	}
	return decode;
}


// Update QSO - returns true if updated and let qso_manager know
record* wsjtx_handler::update_qso(bool tx, string time, double audio_freq, string message) {
	decoded_msg decode = decode_message(message);
	string sender = decode.sender[0] == '<' ? decode.sender.substr(1, decode.sender.length() - 2) : decode.sender;
	string target = decode.target[0] == '<' ? decode.target.substr(1, decode.target.length() - 2) : decode.target;
	string today = now(false, "%Y%m%d");
//	printf("Message: %s %s %g %s\n", tx?"TX":"RX", time.c_str(), audio_freq, message.c_str());
	char msg[100];
	if (tx) {
		if (sender != my_call_) {
			char msg[100];
			snprintf(msg, sizeof(msg), "WSJTX: TX decode not for user %s: %s", my_call_.c_str(), message.c_str());
			status_->misc_status(ST_ERROR, msg);
			return nullptr;
		}
		else {
			record* qso = qso_call(target);
			switch (decode.type) {
			case TX1:
				// <THEM> <ME> <GRID>
			case TX1A:
				// <THEM> <ME> 
			{
				// I am starting a new call - set date/ time/freq/mode
				qso->item("QSO_DATE", today);
				qso->item("TIME_ON", time);
				char f[20];
				double freq = dial_frequency_ + (audio_freq / 1000000.0);
				snprintf(f, sizeof(f), "%0.6f", freq);
				qso->item("FREQ", string(f));
				qso->item("MODE", mode_);
				return qso;
			}
			case TX2:
			{
				// <THEM> <ME> <Report>
				// Starting a new record
				snprintf(msg, sizeof(msg), "WSJT-X: Calling %s, already in QSO with %s", target.c_str(), qso->item("CALL").c_str());
				status_->misc_status(ST_WARNING, msg);
				qso->item("QSO_DATE", today);
				qso->item("TIME_ON", time);
				char f[20];
				double freq = dial_frequency_ + (audio_freq / 1000000.0);
				snprintf(f, sizeof(f), "%0.6f", freq);
				qso->item("FREQ", string(f));
				qso->item("MODE", mode_);
				// Set grid square: from cache or no value
				if (grid_cache_.find(target) != grid_cache_.end()) {
					qso->item("GRIDSQUARE", grid_cache_.at(target));
				}
				else {
					qso->item("GRIDSQUARE", string(""));
				}
				// Add report to existing QSO
				qso->item("RST_SENT", decode.exchange);
				return qso;
			}
			case TX3:
			{
				// <THEM> <ME> R<report>
				qso->item("RST_SENT", decode.exchange.substr(1));
				return qso;
			}
			case TX4:
			{
				// <THEM> <ME> RRR
				qso->item("QSO_COMPLETE", string("?"));
				return qso;
			}
			case TX4A:
				// <THEM> <ME> RR73
			case TX5:
			{
				// <THEM><ME> 73
				qso->item("QSO_COMPLETE", string("Y"));
				return qso;
			}
			case TX6:
			case TX6A:
			default:
				return nullptr;
			}
		}
	}
	else {
		// RX
		if (target != my_call_) {
			// Not for me
			switch (decode.type) {
			case TX1:
				// Call - capture in grid
				grid_cache_[sender] = decode.exchange;
				return nullptr;
			case TX6:
				grid_cache_[sender] = decode.exchange;
				// TODO send to new object wsjtx_runner
				return nullptr;
			case TX4:
			case TX4A:
			case TX5:
				// TODO send to new object wsjtx_runner
				return nullptr;
			default:
				return nullptr;
			}
		}
		else {
			record* qso = qso_call(sender);
			switch (decode.type) {
			case TX1:
				// <ME> <CALL> <GRID>
			case TX1A:
				// <ME> <CALL>
				// Wait until I send the call before starting log entry as may get >1 per decode period
				return nullptr;
			case TX2:
				// <ME> <THEM> <report>
				if (qso->item("GRIDSQUARE").length() == 0) {
					if (grid_cache_.find(target) != grid_cache_.end()) {
						qso->item("GRIDSQUARE", grid_cache_.at(target));
					}
					else {
						qso->item("GRIDSQUARE", string(""));
					}
				}
				qso->item("RST_RCVD", decode.exchange);
				return qso;
			case TX3:
				// <ME> <THEM> R<report>
				qso->item("RST_RCVD", decode.exchange.substr(1));
				return qso;
			case TX4:
				// <ME> <THEM> RRR
				qso->item("QSO_COMPLETE", string("?"));
				return qso;
			case TX4A:
				// <ME> <THEM> RR73
			case TX5:
				// <ME> <THEM> 73
				qso->item("QSO_COMPLETE", string("Y"));
				return qso;
			default:
				return nullptr;
			}
		}
	}
}

// Get a QSO record for this callsign
record* wsjtx_handler::qso_call(string call) {
	record* qso = nullptr;
	if (qsos_.find(call) == qsos_.end()) {
		qso = new record();
		qso->item("CALL", call);
		qso->item("QSO_COMPLETE", string("N"));
		qsos_[call] = qso;
	}
	else {
		qso = qsos_.at(call);
		string band = spec_data_->band_for_freq(dial_frequency_);
		if (qso->item("MODE") == mode_ && qso->item("BAND") == band) {
			if (qso->item("TIME_OFF") != "") {
				time_t now = time(nullptr);
				time_t qso_ts = qso->timestamp(true);
				if (difftime(now, qso_ts) > (5 * 60)) {
					switch (
						fl_choice("Received update to QSO %s %s %s, is this...", "the same QSO??", "a new QSO?", nullptr,
							call.c_str(), band.c_str(), mode_.c_str())) {
					case 0:
						break;
					case 1:
						qso = new record();
						qso->item("CALL", call);
						qso->item("QSO_COMPLETE", string("N"));
						qsos_[call] = qso;
						break;
					}
				}
			}
		}
		else {
			qso = new record();
			qso->item("CALL", call);
			qso->item("QSO_COMPLETE", string("N"));
			qsos_[call] = qso;
		}
	}
	return qso;
}

// Clear existing QSO record
void wsjtx_handler::clear_qsos() {
	for (auto ix = qsos_.begin(); ix != qsos_.end(); ix++) {
		record* qso = ix->second;
		if (qso->item("QSO_COMPLETE") == "Y") {
			// Complete but not logged
			qso->item("QSO_COMPLETE", string(""));
			qso_manager_->update_modem_qso(qso);
		}
		else if (qso->item("QSO_COMPLETE") != "") {
			delete qso;
		}
	}
}
