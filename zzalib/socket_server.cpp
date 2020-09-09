#include "socket_server.h"

#include <stdio.h>
#include <WS2tcpip.h>
#include <sstream>
#include <iostream>
#include <vector>

#ifndef _WIN32
#include <fnctl.h>
#endif

#include <FL/Fl.H>

using namespace zzalib;

// Constructor
socket_server::socket_server(protocol_t protocol, int port_num) :
	server_(INVALID_SOCKET),
	client_(INVALID_SOCKET),
	protocol_(protocol),
	port_num_(port_num)
{

}

// Create and start the server
void socket_server::run_server() {
	closing_ = false;
	// create server
	int result = create_server();
	if (result) {
		switch (protocol_) {
		case UDP:
			status(ST_SEVERE, "SOCKET: Unable to listen for WSJT-X datagrams");
			break;
		case HTTP:
			status(ST_SEVERE, "SOCKET: Unable to listen for FLDIGI XML-RPC requests");
			break;
		}
	}

	// Start listening for packets - will set a timer to the listen after that
	result = rcv_packet();
}

// Destructor
socket_server::~socket_server() {
};

// Close the socket and clean up winsock
void socket_server::close_server() {
	closing_ = true;
	if (server_ != INVALID_SOCKET) {
		printf("Closing....");
		SOCKADDR_IN server_addr;
		int len_server_addr = sizeof(server_addr);
		char message[256];
		getsockname(server_, (SOCKADDR*)&server_addr, &len_server_addr);
		snprintf(message, 256, "SOCKET: Closing socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
		status(ST_OK, message);
		closesocket(server_);
		WSACleanup();
		server_ = INVALID_SOCKET;
	}
}

// Create the server after initialising winsock and opening socket
int socket_server::create_server() {
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
		status(ST_LOG, "SOCKET: Initialised winsock.");
	}
	// Check the version of winsock loaded is 2.2
	if (ws_data.wVersion != MAKEWORD(2, 2)) {
		snprintf(message, 256, "Incompatible version of winsock %d.%d found", LOBYTE(ws_data.wVersion), HIBYTE(ws_data.wVersion));
		handle_error(message);
		return 1;
	}


	// Create the socket
	switch (protocol_) {
	case UDP:
		server_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		break;
	case HTTP:
		server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	}
	if (server_ == INVALID_SOCKET) {
		handle_error("Unable to create the socket");
		return 1;
	}
	else {
		status(ST_LOG, "SOCKET: Created socket");
	}

	// Define the port we listen on and addresses we listen for
	SOCKADDR_IN server_addr;
	int len_server_addr = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_num_);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// Associate the socket with this address data
	result = bind(server_, (SOCKADDR*)&server_addr, len_server_addr);
	if (result == SOCKET_ERROR) {
		handle_error("Unable to bind the socket");
		return result;
	}
	else {
		getsockname(server_, (SOCKADDR*)&server_addr, &len_server_addr);
		snprintf(message, 256, "SOCKET: Connected socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
		status(ST_OK, message);
	}

	// TODO - need to implement this and then use Client socket for receiving TCP/IP
	if (protocol_ == HTTP) {
		result = listen(server_, SOMAXCONN);
		if (result == SOCKET_ERROR) {
			handle_error("Unable to establich connection");
			return result;
		}
		else {
			getsockname(server_, (SOCKADDR*)&server_addr, &len_server_addr);
			snprintf(message, 256, "SOCKET: Listening socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
			status(ST_OK, message);
		}

		//iResult = listen(ListenSocket, SOMAXCONN);
		//if (iResult == SOCKET_ERROR) {
		//	printf("listen failed with error: %d\n", WSAGetLastError());
		//	closesocket(ListenSocket);
		//	WSACleanup();
		//	return 1;
		//}

		int len_client_addr = sizeof(client_addr_);
		client_ = accept(server_, (SOCKADDR*)&client_addr_, &len_client_addr);
		if (client_ == INVALID_SOCKET) {
			handle_error("Unable to accept connection");
			return 1;
		}
		else {
			snprintf(message, 256, "SOCKET: Accepted socket %s:%d", inet_ntoa(client_addr_.sin_addr), htons(client_addr_.sin_port));
			status(ST_OK, message);
		}
		//// Accept a client socket
		//ClientSocket = accept(ListenSocket, NULL, NULL);
		//if (ClientSocket == INVALID_SOCKET) {
		//	printf("accept failed with error: %d\n", WSAGetLastError());
		//	closesocket(ListenSocket);
		//	WSACleanup();
		//	return 1;
		//}
	}


#ifdef _WIN32
	unsigned long nonblocking = 1;
	if (server_ != INVALID_SOCKET && ioctlsocket(server_, FIONBIO, &nonblocking)) {
		handle_error("Unable to make socket non-blocking");
		return 1;
	}
	if (client_ != INVALID_SOCKET && ioctlsocket(client_, FIONBIO, &nonblocking)) {
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

const int MAX_SOCKET = 10240;

int socket_server::rcv_packet() {
	char* buffer = new char[MAX_SOCKET];
	int buffer_len = MAX_SOCKET;
	int bytes_rcvd = 0;
	// Generate a set of socket descriptors for use in select()
	FD_SET set_sockets;
	FD_ZERO(&set_sockets);
	FD_SET(server_, &set_sockets);

	bool packet_complete = false;
	stringstream ss;
	int payload_size = 0;
	string resource = "";
	string function = "";
	int pos_payload = 0;
	double wait_time = 0.0;
	int len_client_addr = sizeof(client_addr_);
	printf("Listening %s: ", protocol_ == UDP ? "UDP" : "HTTP");
	switch (protocol_) {
	case UDP:
		bytes_rcvd = recvfrom(server_, buffer, buffer_len, 0, (SOCKADDR*)&client_addr_, &len_client_addr);
		break;
	case HTTP:
		bytes_rcvd = recv(client_, buffer, buffer_len, 0);
		break;
	}
	if (bytes_rcvd > 0) {
		printf("Got payload\n");
		string s(buffer, bytes_rcvd);
		ss << s;
		do_request(ss);
		// Receiving data - set wait time short
		wait_time = 0.01;
	}
	else if (WSAGetLastError() == WSAEWOULDBLOCK) {
		printf("Would block\n");
		// Wait two seconds before trying again - wait_repeat_ will get cleared in the 
		wait_time = 2.0;
	} 
	else if (protocol_ == HTTP && WSAGetLastError() == WSAENOTCONN) {
		// Trying to connect a cocket  that's not there
		printf("Not connected\n"); 
		wait_time = 0.0;
	}
	else {
		handle_error("Unable to read from client");
		return 1;
	}
	// Allow event queue to dissipate
	Fl::wait();
	// Now see if we have another 
	if (protocol_ == UDP) {
		Fl::add_timeout(wait_time, cb_timer_udp, this);
	}
	else {
		Fl::add_timeout(wait_time, cb_timer_http, this);
	}

	return 0;
}

int socket_server::send_response(istream& response) {
	// calculate the data size
	streampos startpos = response.tellg();
	response.seekg(0, ios::end);
	streampos endpos = response.tellg();
	int resp_size = (int)endpos - (int)startpos;
	response.seekg(0, ios::beg);
	// 
	string data = "";
	data.reserve(resp_size);
	while (response.good()) {
		string line;
		getline(response, line);
		dump(line);
		data += line + '\n';
	}

	// Send the response packet
	int result;
	if (protocol_ == UDP) {
		result = sendto(server_, data.c_str(), resp_size, 0, (SOCKADDR*)&client_addr_, sizeof(client_addr_));
	}
	else {
		result = send(client_, data.c_str(), resp_size, 0);
	}
	if (result == SOCKET_ERROR) {
		handle_error("Unable to send to");
		return result;
	}
	return 0;
}

// Call back to restart attempt to restart datagram
void socket_server::cb_timer_udp(void* v) {
	socket_server* that = (socket_server*)v;
	that->rcv_packet();
}
void socket_server::cb_timer_http(void* v) {
	socket_server* that = (socket_server*)v;
	that->rcv_packet();
}

// Has a server
bool socket_server::has_server() {
	return (server_ != INVALID_SOCKET);
}

// Handle error - display error message
void socket_server::handle_error(const char* phase) {
	char message[1028];
	DWORD error_code = WSAGetLastError();
	if (error_code) {
		char error_msg[1028];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, error_msg, sizeof(error_msg), nullptr);
		snprintf(message, 1028, "SOCKET: %s: %s", phase, error_msg);
	}
	else {
		snprintf(message, 1028, "SOCKET: %s", phase);
	}
	status(ST_ERROR, message);
	closesocket(server_);
	WSACleanup();
}

// Set handlers
void socket_server::callback(int(*request)(stringstream&), void(*message)(status_t, const char*)) {
	status = message;
	do_request = request;
}

// Diagnostic print
void socket_server::dump(string data) {
	string escaped = "";
	bool newline = false;
	// For every data byte
	for (auto it = data.begin(); it != data.end(); it++) {

		if (*it >= 7 && *it <= 13) {
			// Convert control characters to escaped 
			switch (*it) {
			case '\a':
				escaped += "\\a";
				break;
			case '\b':
				escaped += "\\b";
				break;
			case '\t':
				escaped += "\\t";
				break;
			case '\n':
				escaped += "\\n";
				newline = true;
				break;
			case '\v':
				escaped += "\\v";
				newline = true;
				break;
			case '\f':
				escaped += "\\f";
				newline = true;
				break;
			case '\r':
				escaped += "\\r";
				newline = true;
				break;
			}
		}
		else if (*it < 32 || *it > 0x7F) {
			// Convert other non-printable or non-ASCII characters to hex escaped form
			if (newline) {
				escaped += '\n';
				newline = false;
			}
			char hex[10];
			snprintf(hex, 10, "\\x%02x", *it);
		}
		else if (*it == '\\') {
			if (newline) {
				escaped += '\n';
				newline = false;
			}
			// Convert '\' to '\\'
			escaped += "\\\\";
		}
		else
		{
			if (newline) {
				escaped += '\n';
				newline = false;
			}
			escaped += *it;
		}
	}
	printf("%s", escaped.c_str());
}