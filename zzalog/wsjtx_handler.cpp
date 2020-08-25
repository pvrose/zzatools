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

// Constructor: 
wsjtx_handler::wsjtx_handler() :
	wait_flag_(false),
	server_(INVALID_SOCKET)
{
}

// Create and start the server
void wsjtx_handler::run_server() {
	// create server
	int result = create_server();
	if (result) {
		status_->misc_status(ST_SEVERE, "WSJTX: Unable to listen for WSJT-X datagrams");
	}

	// Now continually listen for datagrams
	wait_flag_ = false;
	received_datagram_ = false;
	while (!result) {
		result = rcv_dgram();
		// Let user get in between datagrams
		Fl::wait();
	}
	if (server_ != INVALID_SOCKET) {
		close_socket();
	}
}

// Destructor
wsjtx_handler::~wsjtx_handler() {
};

// Close the socket and clean up winsock
void wsjtx_handler::close_socket() {
	if (server_ != INVALID_SOCKET) {
		SOCKADDR_IN server_addr;
		int len_server_addr = sizeof(server_addr);
		char message[256];
		getsockname(server_, (SOCKADDR*)&server_addr, &len_server_addr);
		snprintf(message, 256, "WSJTX: Closing socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
		status_->misc_status(ST_OK, message);
		closesocket(server_);
		WSACleanup();
		server_ = INVALID_SOCKET;
	}
}
// Create the server after initialising winsock and opening socket
int wsjtx_handler::create_server() {
	// Used for status messages
	char message[256];

	// Initailise Winsock
	WSADATA ws_data;
	int result = WSAStartup(MAKEWORD(2, 2), &ws_data);
	if (result) {
		handle_error("Unable to initialise winsock");
		return result;
	}
	else {
		status_->misc_status(ST_LOG, "WSJTX: Initialised winsock.");
	}
	// Check the version of winsock loaded is 2.2
	if (ws_data.wVersion != MAKEWORD(2, 2)) {
		snprintf(message, 256, "Incompatible version of winsock %d.%d found", LOBYTE(ws_data.wVersion), HIBYTE(ws_data.wVersion));
		handle_error(message);
		return 1;
	}


	// Create the socket
	server_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server_ == INVALID_SOCKET) {
		handle_error("Unable to create the socket");
		return 1;
	}
	else {
		status_->misc_status(ST_LOG, "WSJTX: Created socket");
	}

	// Define the port we listen on and addresses we listen for
	SOCKADDR_IN server_addr;
	int len_server_addr = sizeof(server_addr);
	const int port = 2237;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// Associate the socket with this address data
	result = bind(server_, (SOCKADDR*)&server_addr, len_server_addr);
	if (result == SOCKET_ERROR) {
		handle_error("Unable to bind the socket");
		return result;
	}
	else {
		getsockname(server_, (SOCKADDR*)&server_addr, &len_server_addr);
		snprintf(message, 256, "WSJTX: Connected socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
		status_->misc_status(ST_OK, message);
	}

#ifdef _WIN32
	unsigned long nonblocking = 1;
	if (ioctlsocket(server_, FIONBIO, &nonblocking)) {
		handle_error("Unable to make socket non-blocking");
		return 1;
	}
#else
	// TODO find fnctl.h
	int flags = fcntl(server_, F_GETFL, 0);
	// TODO add status call
	if (flags == -1) return false;
	flags = (flags | O_NONBLOCK);
	// TODO add status call if fail
	return (fcntl(fd, F_SETFL, flags) == 0) ? 0 : 1;
#endif
	return 0;		
}

const int MAX_WSJTX_DGRAM = 1024;

int wsjtx_handler::rcv_dgram() {
	char* buffer = new char[MAX_WSJTX_DGRAM];
	int buffer_len = MAX_WSJTX_DGRAM;
	int bytes_rcvd = 0;
	// Generate a set of socket descriptors for use in select()
	FD_SET set_sockets;
	FD_ZERO(&set_sockets);
	FD_SET(server_, &set_sockets);

	bool got_data = false;
	// Number of retries
	unsigned int number_retries = 0;
	// WSJT-X currently has the the heartbeat set to 15 s, add 1 for luck
	//timeval to_16s = { 16, 0 };
	//while (!got_data) {
	//	// Wait at most 10 s for data to appear on the socket
	//	int sockets = select(0, &set_sockets, nullptr, nullptr, &to_16s);
	//	if (sockets == 0) {
	//		status_->misc_status(ST_LOG, "WSJTX: No data available on socket after 16 s - abandoning as missing heartbeat");
	//		return 1;
	//	}
	//	else if (sockets > 0) {
	//		got_data = true;
	//	}
	//	else if (sockets == SOCKET_ERROR) {
	//		char message[256];
	//		snprintf(message, 256, "WSJTX: Error accessing socket - code %d - abandoning", WSAGetLastError());
	//		status_->misc_status(ST_ERROR, message);
	//		return 1;
	//	}
	//}
	// Now fetch the real datagram and process it
	while (!got_data) {
		int len_client_addr = sizeof(client_addr_);
		bytes_rcvd = recvfrom(server_, buffer, buffer_len, 0, (SOCKADDR*)&client_addr_, &len_client_addr);
		if (bytes_rcvd > 0) {
			char message[256];
			getpeername(server_, (SOCKADDR*)&client_addr_, &len_client_addr);
			snprintf(message, 256, "WSJTX: %d bytes received from %s:%d", bytes_rcvd, inet_ntoa(client_addr_.sin_addr), htons(client_addr_.sin_port));
			status_->misc_status(ST_LOG, message);
			string ts = now(false, "%H:%M:%S");
			char* pos = buffer;
			pos = get_uint(pos, &magic_number_);
			pos = get_uint(pos, &schema_);
			printf("\n%s: MN: %08x, sch: %d, ", ts.c_str(), magic_number_, schema_);
			if (magic_number_ != expected_magic_ || schema_ < minimum_schema_) {
				char message[256];
				snprintf(message, 256, "datagram had wrong magic number (%08x) or unsupported schema (%d)", magic_number_, schema_);
				WSASetLastError(0);
				handle_error(message);
				return 1;
			}
			switch (buffer[11]) {
			case 0:
				// Heartbeat
				return handle_hbeat(buffer, bytes_rcvd);
			case 6:
				return handle_close(buffer, bytes_rcvd);
			case 12:
				return handle_log(buffer, bytes_rcvd);
			default:
				return handle_default(buffer, bytes_rcvd);
			}
			got_data = true;
		}
		else if (WSAGetLastError() == WSAEWOULDBLOCK) {
			// Wait two seconds before trying again - wait_flag_ will get cleared in the 
			wait_flag_ = true;
			Fl::add_timeout(2.0, cb_timer_wait, (void*)&wait_flag_); 
			while (wait_flag_) Fl::wait();
			number_retries++;
			if (number_retries > 15) {
				status_->misc_status(ST_WARNING, "WSJTX: WSJT-X is not present - closing server");
				return 1;
			}
		}
		else {
			handle_error("Unable to read from client");
			return 1;
		}
		Fl::wait();
	}
	return 0;
}

// For now print the raw hex of the datagram
int wsjtx_handler::handle_default(char* const dgram, const int len) {
	printf("Type %d: \n", dgram[11]);
	for (int i = 12; i < len; i++) {
		printf("%02x ", ((unsigned)dgram[i] & 0xFF));
		if ((i - 12) % 16 == 15) {
			printf("\n");
		}
	}
	return 0;
}

// For now print heartbeat message
int wsjtx_handler::handle_hbeat(char* const dgram, const int len) {
	print_hbeat(dgram);
	send_hbeat();
	return 0;
}

int wsjtx_handler::send_hbeat() {
	char* dgram = new char[256];
	memset(dgram, 0, 256);
	// Add the magic number, schema and function number
	char* pos = put_uint(dgram, magic_number_);
	pos = put_uint(pos, schema_);
	pos = put_uint(pos, 0);
	// Add the ID
	pos = put_utf8(pos, PROGRAM_ID);
	// Add the max schema
	pos = put_uint(pos, 3);
	// Add the version
	pos = put_utf8(pos, VERSION);
	// Add the revision ""
	pos = put_uint(pos, -1);
	
	string ts = now(false, "%H:%M:%S");
	pos = get_uint(dgram, &magic_number_);
	pos = get_uint(pos, &schema_);
	printf("\n%s: MN: %08x, sch: %d, ", ts.c_str(), magic_number_, schema_);
	print_hbeat(dgram);

	return send_dgram(dgram, 256);
}

void wsjtx_handler::print_hbeat(char* const dgram) {
	printf("Heartbeat: ");
	char* pos = dgram;
	string id;
	pos = get_utf8(dgram + 12, &id);
	printf("Id: %s, ", id.c_str());
	unsigned int schema;
	pos = get_uint(pos, &schema);
	printf("Max schema %d, ", schema);
	pos = get_utf8(pos, &id);
	printf("Version %s, ", id.c_str());
	pos = get_utf8(pos, &id);
	printf("Revision %s\n", id.c_str());
}

int wsjtx_handler::send_dgram(const char* dgram, const int len) {
	// Send the dgram
	int result = sendto(server_, dgram, 256, 0, (SOCKADDR*)&client_addr_, sizeof(client_addr_));
	if (result == SOCKET_ERROR) {
		handle_error("Unable to send to");
		return result;
	}
	return 0;
}

// For now print close message and close the socket (indirectly by returning 1)
int wsjtx_handler::handle_close(char* const dgram, const int len) {
	printf("Close: ");
	char* pos = dgram;
	string id;
	pos = get_utf8(dgram + 12, &id);
	printf("Id: %s\n", id.c_str());
	return 1;
}

// Handle the logged ADIF datagram.
int wsjtx_handler::handle_log(char* const dgram, const int len) {
	printf("Logged ADIF: ");
	char* pos = dgram;
	string id;
	pos = get_utf8(dgram + 12, &id);
	printf("Id: %s\n", id.c_str());
	pos = get_utf8(pos, &id);
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

// Get an integer from the first 4 bytes of dgram 
char* wsjtx_handler::get_uint(char* dgram, unsigned int* i) {
	*i = 0;
	for (int ix = 0; ix < 4; ix++) {
		*i = (*i << 8) + (*(dgram + ix) & 0xff);
	}
	return dgram + 4;
}

// Get a string from the QByteArray (4 byte length + that number of bytes)
char* wsjtx_handler::get_utf8(char* dgram, string* s) {
	unsigned int len;
	char* pos = get_uint(dgram, &len);
	if (len == ~(0)) {
		*s = "";
	}
	else {
		*s = string(pos, len);
	}
	return pos + len;
}

// Put an integer as 4 bytes in the first 4 bytes of dgram
char* wsjtx_handler::put_uint(char* dgram, unsigned int i) {
	unsigned int rem = i;
	for (int ix = 3; ix >= 0; ix--) {
		*(dgram + ix) = rem & 0xff;
		rem = rem >> 8;
	}
	return dgram + 4;
}

// Put a string as QByteArray (see above) into the dgram
char* wsjtx_handler::put_utf8(char* dgram, string s) {
	char* pos = put_uint(dgram, s.length());
	for (unsigned int ix = 0; ix < s.length(); ix++) {
		*(pos + ix) = s[ix];
	}
	return pos + s.length();
}

// Call back to restart attempt to restart datagram
void wsjtx_handler::cb_timer_wait(void* v) {
	bool* flag = (bool*)v;
	*flag = false;
}

// Has a server
bool wsjtx_handler::has_server() {
	return (server_ != INVALID_SOCKET);
}

// Handle error - display error message
void wsjtx_handler::handle_error(const char* phase) {
	char message[1028];
	DWORD error_code = WSAGetLastError();
	if (error_code) {
		char error_msg[1028];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, error_msg, sizeof(error_msg), nullptr);
		snprintf(message, 1028, "WSJTX: %s: %s", phase, error_msg);
	}
	else {
		snprintf(message, 1028, "WSJTX: %s", phase);
	}
	status_->misc_status(ST_ERROR, message);
	closesocket(server_);
	WSACleanup();
}