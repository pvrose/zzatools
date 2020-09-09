#include "wsjtx_handler.h"
#include "status.h"
#include "version.h"
#include "import_data.h"
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

wsjtx_handler* wsjtx_handler::that_ = nullptr;


// Constructor: 
wsjtx_handler::wsjtx_handler()
{
	that_ = this;
	server_ = nullptr;
	run_server();
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
	printf("\n%s: MN: %08x, sch: %d, ", ts.c_str(), magic_number_, schema_);
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

// For now print the raw hex of the datagram
int wsjtx_handler::handle_default(stringstream& ss, uint32_t type) {
	printf("Type %d: \n", type);
	int i = 0;
	while (ss.good()) {
		unsigned char c;
		ss >> c;
		printf("%02x ", c);
		i++;
		if (i % 16 == 15) printf("\n");
	}
	return 0;
}

// For now print heartbeat message
int wsjtx_handler::handle_hbeat(stringstream& ss) {
	print_hbeat(ss);
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
	
	// Now go back to the start of the stream to print debug
	ss.seekg(0, ios::beg);
	string ts = now(false, "%H:%M:%S");
	magic_number_ = get_uint32(ss);
	schema_ = get_uint32(ss);
	uint32_t dummy = get_uint32(ss);
	printf("\n%s: MN: %08x, sch: %d, ", ts.c_str(), magic_number_, schema_);
	print_hbeat(ss);

	ss.seekg(0, ios::beg);
	return server_->send_response(ss);
}

void wsjtx_handler::print_hbeat(stringstream& ss) {
	printf("Heartbeat: ");
	string id = get_utf8(ss);
	printf("Id: %s, ", id.c_str());
	uint32_t schema = get_uint32(ss);
	printf("Max schema %d, ", schema);
	id = get_utf8(ss);
	printf("Version %s, ", id.c_str());
	id = get_utf8(ss);
	printf("Revision %s", id.c_str());
}

// For now print close message and close the socket (indirectly by returning 1)
int wsjtx_handler::handle_close(stringstream& ss) {
	printf("Close: ");
	string id;
	id = get_utf8(ss);
	printf("Id: %s\n", id.c_str());
	server_->close_server();
	delete server_;
	server_ = nullptr;
	return 1;
}

// Handle the logged ADIF datagram.
int wsjtx_handler::handle_log(stringstream& ss) {
	printf("Logged ADIF: ");
	string id = get_utf8(ss);
	printf("Id: %s\n", id.c_str());
	id = get_utf8(ss);
	printf("%s\n", id.c_str());

	stringstream adif;
	adif.str(id);
	// Stop any extant update and wait for it to complete
	import_data_->stop_update(LM_OFF_AIR, false);
	while (!import_data_->update_complete()) Fl::wait();
	import_data_->load_stream(adif, import_data::update_mode_t::DATAGRAM);
	// Wait for the import to finish
	while (import_data_->size()) Fl::wait();
	status_->misc_status(ST_NOTE, "WSJTX: Logged QSO");
	return 0;
}

// handle decode
/*Decode        Out       2                      quint32
* Id(unique key)        utf8
* New                    bool
* Time                   QTime
* snr                    qint32
* Delta time(S)         float(serialized as double)
* Delta frequency(Hz)   quint32
* Mode                   utf8
* Message                utf8
* Low confidence         bool
* Off air                bool
*
13:01 : 12 : MN : adbccbda, sch : 2, Type 2 :
	00 00 00 06 57 53 4a 54 2d 58 
	01 
	02 cb 06 e0 
	00 00 00 13 
	bf e0 00 00 00 00 00 00 
	00 00 04 25 
	00 00 00 01 7e 
	00 00 00 10 47 4d 34 46 56 4d 20 4f 48 36 4e 4d 59 20 37 33 00 00
	*/
int wsjtx_handler::handle_decode(stringstream& ss) {
	printf("Decode: ");
	string sv = get_utf8(ss);
	printf("ID: %s,", sv.c_str());
	bool bv = get_bool(ss);
	printf("New: %d, ", bv);
	uint32_t iv = get_uint32(ss);
	printf("Time: %08x, ", iv);
	iv = get_uint32(ss);
	printf("dB: %d, ", iv);
	double dv = get_double(ss);
	printf("DT: %f, ", dv);
	iv = get_uint32(ss);
	printf("Freq: %d, ", iv);
	sv = get_utf8(ss);
	printf("Mode: %s, ", sv.c_str());
	sv = get_utf8(ss);
	printf("Message: %s, ", sv.c_str());
	bv = get_bool(ss);
	printf("Conf: %d, ", bv);
	bv = get_bool(ss);
	printf("Off air: %d", bv);
	return 0;
}

// handle status
/* Status        Out       1                      quint32
* Id(unique key)        utf8
* Dial Frequency(Hz)    quint64
* Mode                   utf8
* DX call                utf8
* Report                 utf8
* Tx Mode                utf8
* Tx Enabled             bool
* Transmitting           bool
* Decoding               bool
* Rx DF                  quint32
* Tx DF                  quint32
* DE call                utf8
* DE grid                utf8
* DX grid                utf8
* Tx Watchdog            bool
* Sub - mode               utf8
* Fast mode              bool
* Special Operation Mode quint8
* Frequency Tolerance    quint32
* T / R Period             quint32
* Configuration Name     utf8
15:47:50: MN: adbccbda, sch: 2, Type 1:
00 00 00 06 57 53 4a 54 2d 58 00 00 00 00 01 14
3e c0 00 00 00 03 4a 54 39 00 00 00 06 47 53 33
5a 5a 41 00 00 00 03 2d 31 35 00 00 00 03 4a 54
39 00 00 01 00 00 02 e7 00 00 05 dc 00 00 00 06
47 4d 33 5a 5a 41 00 00 00 06 49 4f 38 35 46 55
ff ff ff ff 00 ff ff ff ff 00 00 ff ff ff ff ff
ff ff ff 00 00 00 07 44 65 66 61 75 6c 74*/
int wsjtx_handler::handle_status(stringstream& ss) {
	printf("Status: ");
	string id;
	id = get_utf8(ss);
	printf("Id: %s, ", id.c_str());
	uint64_t freq = get_uint64(ss);
	printf("Freq: %lld, ", freq);
	id = get_utf8(ss);
	printf("Mode: %s, ", id.c_str());
	id = get_utf8(ss);
	printf("DX Call %s, ", id.c_str());
	id = get_utf8(ss);
	printf("Rpt: %s, ", id.c_str());
	id = get_utf8(ss);
	printf("TX Mode: %s, ", id.c_str());
	bool bv;
	bv = get_bool(ss); printf("TX En: %d, ", bv);
	bv = get_bool(ss); printf("TX: %d, ", bv);
	bv = get_bool(ss); printf("Dec: %d, ", bv);
	uint32_t iv;
	iv = get_uint32(ss); printf("RX DF: %d, ", iv);
	iv = get_uint32(ss); printf("TX DF: %d, ", iv);
	id = get_utf8(ss); printf("Call: %s, ", id.c_str());
	id = get_utf8(ss); printf("Grid: %s, ", id.c_str());
	id = get_utf8(ss); printf("DX Grid: %s, ", id.c_str());
	bv = get_bool(ss); printf("TX WD: %d, ", bv);
	id = get_utf8(ss); printf("Sub: %s, ", id.c_str());
	bv = get_bool(ss); printf("Fast: %d, ", bv);
	uint8_t i8v = get_uint8(ss); printf("Spec op mode: %d, ", i8v);
	iv = get_uint32(ss); printf("DF: %d, ", iv);
	iv = get_uint32(ss); printf("T/R: %d, ", iv);
	id = get_utf8(ss); printf("Conf name: %s, ", id.c_str());
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
	return (server_ != nullptr);
}

void wsjtx_handler::run_server() {
	if (server_) {
		server_->run_server();
	}
	else {
		server_ = new socket_server(socket_server::UDP, 2237);
		server_->callback(rcv_request, cb_error_message);
	}
}

void wsjtx_handler::close_server() {
	if (server_) {
		server_->close_server();
		delete server_;
		server_ = nullptr;
	}
}