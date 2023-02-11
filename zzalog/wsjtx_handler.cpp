#include "wsjtx_handler.h"
#include "status.h"
#include "import_data.h"
#include "menu.h"
#include "../zzalib/utils.h"
#include "dxa_if.h"
#include "prefix.h"
#include "pfx_data.h"
#include "toolbar.h"
#include "qso_manager.h"

#include <stdio.h>
#include <WS2tcpip.h>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#ifndef _WIN32
#include <fnctl.h>
#endif

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>

using namespace zzalog;
using namespace zzalib;
using namespace std;

extern status* status_;
extern import_data* import_data_;
extern void cb_error_message(status_t level, const char* message);
extern menu* menu_;
extern Fl_Preferences* settings_;
extern dxa_if* dxa_if_;
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
	dxa_if_->clear_dx_loc();
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
	// Stop any extant update and wait for it to complete
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::wait();
	import_data_->load_stream(adif, import_data::update_mode_t::DATAGRAM);
	// Wait for the import to finish
	while (import_data_->size()) Fl::wait();
	status_->misc_status(ST_NOTE, "WSJT-X: Logged QSO");
	// Clear DX locator flag
	dxa_if_->clear_dx_loc();
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
	// Change display if addressed to user
	vector<string> words;
	split_line(decode.message, words, ' ');
	//// Display if this is my call or my hashed call
	//if (words[0] == my_call_ || words[0] == my_bracketed_call_) {
	//	// If this is my_call their_call grid add to DxAtlas map as DX location
	//	if (regex_match(words[2], basic_regex<char>("[A-R][A-R][0-9][0-9]")) && words[2] != "RR73") {
	//		dxa_if_->set_dx_loc(words[2]);
	//	}
	//	// Display in status bar and beep if message addressed to user
	//	status_->misc_status(ST_NOTIFY, message);
	//}
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
			// Get the grid location of the prefix centre - only if 1 prefix matches the callsign.
			record* dx_record = qso_manager_->dummy_qso();
			dx_record->item("CALL", status.dx_call);
			vector<prefix*> prefixes;
			// Get prefix(es), If only 1 and the DXCC Code != 0 (i.e. not /MM)
			if (pfx_data_->all_prefixes(dx_record, &prefixes, false) && prefixes.size() == 1) {
				// Get DXCC prefix record for the DXCC number
				prefix* dxcc_prefix = prefixes[0];
				while (dxcc_prefix->parent_ != nullptr) {
					dxcc_prefix = dxcc_prefix->parent_;
				}
				if (dxcc_prefix->dxcc_code_ != 0) {
					// Use the actual prefixes location rather than the DXCC's
					lat_long_t location = { prefixes[0]->latitude_, prefixes[0]->longitude_ };
					dxa_if_->set_dx_loc(latlong_to_grid(location, 6), status.dx_call);
				}
				else {
					// Not a DXCC entity - clear any existing DX Location
					char message[100];
					snprintf(message, 100, "WSJT-X: Cannot locate %s - not in a DXCC entity", status.dx_call.c_str());
					status_->misc_status(ST_WARNING, message);
					dxa_if_->clear_dx_loc();
				}
			}
			else {
				// Too many prefixes to automatically parse - clear any existing DX Location
				char message[100];
				snprintf(message, 100, "WSJT-X: Cannot parse %s - %d matching prefixes found", status.dx_call.c_str(), prefixes.size());
				status_->misc_status(ST_WARNING, message);
				dxa_if_->clear_dx_loc();
			}
			toolbar_->search_text(status.dx_call);
		}
	}
	else if (!status.dx_call.length()) {
		// Can clear the Dx Location by clearing the DX Call field
		dxa_if_->clear_dx_loc();
	}
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