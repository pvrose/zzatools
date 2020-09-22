#include "wsjtx_handler.h"
#include "status.h"
#include "version.h"
#include "import_data.h"
#include "menu.h"
#include "../zzalib/utils.h"

#include <stdio.h>
#include <WS2tcpip.h>
#include <sstream>
#ifndef _WIN32
#include <fnctl.h>
#endif

#include <FL/Fl.H>

using namespace zzalog;
using namespace zzalib;

extern status* status_;
extern import_data* import_data_;
extern void cb_error_message(status_t level, const char* message);
extern menu* menu_;

wsjtx_handler* wsjtx_handler::that_ = nullptr;


// Constructor: 
wsjtx_handler::wsjtx_handler()
{
	that_ = this;
	server_ = nullptr;
	run_server();
	last_decode_time_ = 0;
	new_heartbeat_ = false;
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

int wsjtx_handler::rcv_dgram(stringstream & ss) {
	string ts = now(false, "%H:%M:%S");
	magic_number_ = get_uint32(ss);
	schema_ = get_uint32(ss);
	uint32_t dgram_type = get_uint32(ss);
	if (magic_number_ != expected_magic_ || schema_ < minimum_schema_) {
		char message[256];
		snprintf(message, 256, "datagram had wrong magic number (%08x) or unsupported schema (%d)", magic_number_, schema_);
		status_->misc_status(ST_WARNING, message);
		return 1;
	}
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

// 
int wsjtx_handler::handle_default(stringstream& ss, uint32_t type) {
	char message[128];
	snprintf(message, 128, "WSJT-X: Ignored type %d datagram", type);
	status_->misc_status(ST_LOG, message);
	return 0;
}

// 
int wsjtx_handler::handle_hbeat(stringstream& ss) {
	status_->misc_status(ST_LOG, "WSJT-X: Received heartbeat");
	new_heartbeat_ = true;
	send_hbeat();
	return 0;
}

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

// For now print close message 
int wsjtx_handler::handle_close(stringstream& ss) {
	status_->misc_status(ST_NOTE, "WSJT-X: Received Close");
	server_->close_server();
	menu_->update_items();
	return 1;
}

// Handle the logged ADIF datagram.
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
	import_data_->stop_update(LM_OFF_AIR, false);
	while (!import_data_->update_complete()) Fl::wait();
	import_data_->load_stream(adif, import_data::update_mode_t::DATAGRAM);
	// Wait for the import to finish
	while (import_data_->size()) Fl::wait();
	status_->misc_status(ST_NOTE, "WSJT-X: Logged QSO");
	return 0;
}

// handle decode
int wsjtx_handler::handle_decode(stringstream& ss) {
	string sv = get_utf8(ss);
	bool bv = get_bool(ss);
	uint32_t iv = get_uint32(ss);
	if (iv != last_decode_time_) {
		char message[128];
		snprintf(message, 128, "WSJT-X: Received %d decodes at time %d.", decodes_rcvd_, last_decode_time_);
		status_->misc_status(ST_LOG, message);
		last_decode_time_ = iv;
		decodes_rcvd_ = 1;
	}
	else {
		decodes_rcvd_++;
	}
	iv = get_uint32(ss);
	double dv = get_double(ss);
	iv = get_uint32(ss);
	sv = get_utf8(ss);
	sv = get_utf8(ss);
	bv = get_bool(ss);
	bv = get_bool(ss);
	return 0;
}

// handle status
int wsjtx_handler::handle_status(stringstream& ss) {
	if (new_heartbeat_) {
		char message[128];
		snprintf(message, 128, "WSJT-X: Received %d status reports in last heartbeat", status_rcvd_);
		status_rcvd_ = 1;
		new_heartbeat_ = false;
	}
	else {
		status_rcvd_++;
	}
	string id;
	// ID
	id = get_utf8(ss);
	// Frequency
	uint64_t freq = get_uint64(ss);
	// Mode
	id = get_utf8(ss);
	// DX Call
	id = get_utf8(ss);
	id = get_utf8(ss);
	// Report 
	id = get_utf8(ss);
	// Transmit mode
	bool bv;
	// Transmit enabled
	bv = get_bool(ss);
	// Transmit
	bv = get_bool(ss);
	// Decoding
	bv = get_bool(ss);
	uint32_t iv;
	// Received frequency - delta from VFO
	iv = get_uint32(ss);
	// Transmit frequency
	iv = get_uint32(ss);
	// My call
	id = get_utf8(ss);
	// My grid
	id = get_utf8(ss);
	// DX Grid
	id = get_utf8(ss);
	// Transmit ?
	bv = get_bool(ss);
	// Submode
	id = get_utf8(ss);
	// FAst decode
	bv = get_bool(ss); 
	// Special operation mode
	uint8_t i8v = get_uint8(ss); 
	// ???
	iv = get_uint32(ss); 
	// ????
	iv = get_uint32(ss);
	// Configuration name
	id = get_utf8(ss); 
	return 0;
}


// Get an bool from the first  bytes of dgram 
bool wsjtx_handler::get_bool(stringstream& ss) {
	unsigned char c;
	ss.get((char&)c);
	bool b = c ? true : false;
	return b;
}

// Get an integer from the first 4 bytes of dgram 
uint8_t wsjtx_handler::get_uint8(stringstream& ss) {
	unsigned char c;
	ss.get((char&)c);
	uint8_t i = c;
	return i;
}

// Get an integer from the first 4 bytes of dgram 
uint32_t wsjtx_handler::get_uint32(stringstream& ss) {
	unsigned char c = 0;
	uint32_t i = 0;
	for (int ix = 0; ix < 4; ix++) {
		ss.get((char&)c);
		i = (i << 8) + c;
	}
	return i;
}

// Get 64-bit unsigned integer
uint64_t wsjtx_handler::get_uint64(stringstream& ss) {
	unsigned char c = 0;
	uint64_t i = 0LL;
	for (int ix = 0; ix < 8; ix++) {
		ss.get((char&)c);
		i = (i << 8) + c;
	}
	return i;
}

// Get double 
double wsjtx_handler::get_double(stringstream& ss) {
	// I'm making the assumption that the double has been directly serialised in its bit pattern
	uint64_t uv = get_uint64(ss);
	double* d = reinterpret_cast<double*>(&uv);
	return *d;
}

// Get a string from the QByteArray (4 byte length + that number of bytes)
string wsjtx_handler::get_utf8(stringstream& ss) {
	uint32_t len = get_uint32(ss);
	if (len == ~(0)) {
		return "(null)";
	}
	else {
		string s;
		s.resize(len + 1, 0);
		for (uint32_t i = 0; i < len; i++) {
			ss.get(s[i]);
		}
		return s;
	}
}

// Put an integer as 4 bytes in the first 4 bytes of dgram
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

// Call back to restart attempt to restart datagram
void wsjtx_handler::cb_timer_wait(void* v) {
	bool* flag = (bool*)v;
	*flag = false;
}

bool wsjtx_handler::has_server() {
	return (server_ != nullptr && server_->has_server());
}

void wsjtx_handler::run_server() {
	if (!server_) {
		server_ = new socket_server(socket_server::UDP, 2237);
		server_->callback(rcv_request, cb_error_message);
	}
	if (!server_->has_server()) {
		server_->run_server();
	}
	menu_->update_items();
}

void wsjtx_handler::close_server() {
	if (server_) {
		server_->close_server();
		delete server_;
		server_ = nullptr;
	}
}