#include "socket_server.h"
#include "status.h"

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <WS2tcpip.h>
#else
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1
#define SOCKADDR sockaddr

#endif

#include <FL/Fl.H>

#ifdef _WIN32
#define LEN_SOCKET_ADDR int
#else
#define LEN_SOCKET_ADDR unsigned int
#endif

extern status *status_;

// Constructor
socket_server::socket_server(protocol_t protocol, string address, int port_num) : 
	server_(INVALID_SOCKET),
	client_(INVALID_SOCKET),
    protocol_(protocol),
    port_num_(port_num),
    th_socket_(nullptr)
{
	if (address.length())
	{
		address_ = address;
	}
	else
	{
		address_ = "0.0.0.0";
	}
}

// Create and start the server
void socket_server::run_server()
{
	closing_ = false;
	// create server
	int result = create_server();
	if (result)
	{
		switch (protocol_)
		{
		case UDP:
			status_->misc_status(ST_ERROR, "SOCKET: Unable to listen for WSJT-X datagrams");
			break;
		case HTTP:
			status_->misc_status(ST_ERROR, "SOCKET: Unable to listen for FLDIGI XML-RPC requests");
			break;
		}
	}
	// Start listening for packets - will set a timer to the listen after that
	th_socket_ = new thread(thread_run, this);
}

// Destructor
socket_server::~socket_server(){};

// Close the socket and clean up winsock
void socket_server::close_server(bool external)
{
	// WAit for rcv_packet to tidy up
	closing_ = true;
	if (external) {
		while (!closed_) this_thread::yield();
	}
	if (th_socket_)
		th_socket_->detach();
	delete th_socket_;
	th_socket_ = nullptr;
	// Fl::remove_timeout(cb_timer_acc, this);
	// Fl::remove_timeout(cb_timer_rcv, this);
	if (server_ != INVALID_SOCKET)
	{
		SOCKADDR_IN server_addr;
		LEN_SOCKET_ADDR len_server_addr = sizeof(server_addr);
		char message[256];
		getsockname(server_, (SOCKADDR *)&server_addr, &len_server_addr);
		snprintf(message, 256, "SOCKET: Closing socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
		status_->misc_status(ST_OK, message);
#ifdef _WIN32
		closesocket(server_);
		WSACleanup();
#else
		shutdown(server_, 2);
		close(server_);
#endif
		server_ = INVALID_SOCKET;
	}
}

// Create the server after initialising winsock and opening socket
int socket_server::create_server()
{
	int result;
	// Used for status messages
	char message[256];
#ifdef _WIN32
	// Initailise Winsock
	WSADATA ws_data;
	result = WSAStartup(MAKEWORD(2, 2), &ws_data);
	if (result)
	{
		handle_error("Unable to initialise winsock");
		return result;
	}
	else
	{
		status_->misc_status(ST_LOG, "SOCKET: Initialised winsock.");
	}
	// Check the version of winsock loaded is 2.2
	if (ws_data.wVersion != MAKEWORD(2, 2))
	{
		snprintf(message, 256, "Incompatible version of winsock %d.%d found", LOBYTE(ws_data.wVersion), HIBYTE(ws_data.wVersion));
		handle_error(message);
		return 1;
	}
#endif

	// Create the socket
	switch (protocol_)
	{
	case UDP:
#ifdef _WIN32
// Windows does not support SOCK_NONBLOCK
		server_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
		server_ = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
#endif
		break;
	case HTTP:
#ifdef _WIN32
		server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
		server_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
#endif
		break;
	}
	if (server_ == INVALID_SOCKET)
	{
		handle_error("Unable to create the socket");
		return 1;
	}
	else
	{
		switch (protocol_)
		{
		case UDP:
			status_->misc_status(ST_LOG, "SOCKET: Created UDP socket");
			break;
		case HTTP:
			status_->misc_status(ST_LOG, "SOCKET: Created HTTP socket");
			break;
		}
	}

	// Define the port we listen on and addresses we listen for
	SOCKADDR_IN server_addr;
	LEN_SOCKET_ADDR len_server_addr = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_num_);
	server_addr.sin_addr.s_addr = inet_addr(address_.c_str());

	// Multicast addresses 224.0.0.0 to 239.255.255.255 = NB byte order
	bool multicast = (server_addr.sin_addr.s_addr & (unsigned)0x000000F0) == (unsigned)0x000000E0;
	if (multicast) {
#ifdef _WIN32
		handle_error("Multicast not yet implemented in app on Windows");
		return -1;
#else
		int set_option_on = 1;
		// Allow address to be reused
		result = setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, (char*)&set_option_on,
			sizeof(set_option_on));
		if (result < 0) {
			handle_error("Unable to set socket reusable");
			return result;
		}
		// Apply to join the multicast group
		switch (protocol_) {
			case UDP: {
				// Set Multicast loop
				char loop = 1;
				result = setsockopt(server_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
				if (result < 0) {
					handle_error("Cannot set multicast loopback");
					return result;
				}
				// Set Multicast all
				char mall = 1;
				result = setsockopt(server_, IPPROTO_IP, IP_MULTICAST_ALL, &mall, sizeof(mall));
				if (result < 0) {
					handle_error("Canmnot set multicast all");
					return result;
				}
				ip_mreq mreq = { server_addr.sin_addr, INADDR_ANY };
				result = setsockopt(server_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
				if (result < 0) {
					handle_error("Unable to join multicast group");
					return result;
				}
				else {
					snprintf(message, sizeof(message), "SOCKET: Joined multicast %s", inet_ntoa(server_addr.sin_addr));
					status_->misc_status(ST_OK, message);
				}
				break;
			}
			case HTTP:
				snprintf(message, sizeof(message), "SOCKET: Cannot set multicast %s for HTTP", inet_ntoa(server_addr.sin_addr));
				status_->misc_status(ST_WARNING, message);
				break;
		}
#endif
	} else {
		int set_option_on = 1;
		// Allow address to be reused - it may be hanging around from a previous
		result = setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, (char*)&set_option_on,
			sizeof(set_option_on));
		if (result < 0) {
			handle_error("Unable to set socket reusable");
			return result;
		}
	}
	// Associate the socket with this address data
	result = bind(server_, (SOCKADDR *)&server_addr, len_server_addr);
	if (result < 0)
	{
		handle_error("Unable to bind the socket");
		return result;
	}
	else
	{
		getsockname(server_, (SOCKADDR *)&server_addr, &len_server_addr);
		snprintf(message, 256, "SOCKET: Connected socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
		status_->misc_status(ST_OK, message);
	}

 #ifdef _WIN32
	// Windows way of supporting non-blocking operations
 	unsigned long nonblocking = 1;
 	if (server_ != INVALID_SOCKET && ioctlsocket(server_, FIONBIO, &nonblocking))
 	{
 		handle_error("Unable to make socket non-blocking");
 		return 1;
 	}
 #endif

	// Set socket into listening mode
	if (protocol_ == HTTP)
	{
		result = listen(server_, SOMAXCONN);
		if (result < 0)
		{
			handle_error("Unable to establich connection");
			return result;
		}
		else
		{
			getsockname(server_, (SOCKADDR *)&server_addr, &len_server_addr);
			snprintf(message, 256, "SOCKET: Listening socket %s:%d", inet_ntoa(server_addr.sin_addr), htons(server_addr.sin_port));
			status_->misc_status(ST_OK, message);
		}

 #ifdef _WIN32
 		// Make the client non-blocking as well
 		unsigned long nonblocking = 1;
 		if (client_ != INVALID_SOCKET && ioctlsocket(client_, FIONBIO, &nonblocking))
 		{
 			handle_error("Unable to make client socket non-blocking");
 			return 1;
 		}
#endif
	}
	return 0;
}

// Accept any client asking to connect, use non-blocking call to avoid locking out other code
socket_server::client_status socket_server::accept_client()
{
	char message[256];
	LEN_SOCKET_ADDR len_client_addr = sizeof(client_addr_);
	client_ = accept(server_, (SOCKADDR *)&client_addr_, &len_client_addr);
	if (client_ == INVALID_SOCKET)
	{
#ifdef _WIN32
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			// status_->misc_status(ST_LOG, "SOCKET: No client socket to accept");
			//  Non-blocking accept would have blocked - let other events get handled and try again
			return BLOCK;
		}
		else
		{
			handle_error("Unable to accept connection");
			return NG;
		}
#else
		return BLOCK;
#endif
	}
	else
	{
		return OK;
	}
}
const int MAX_SOCKET = 10240;

int socket_server::rcv_packet()
{
	closed_ = false;
	char *buffer = new char[MAX_SOCKET];
	int buffer_len = MAX_SOCKET;
	int bytes_rcvd = 0;
	// Generate a set of socket descriptors for use in select()
//#ifdef _WIN32
//	FD_SET set_sockets;
//#else
	fd_set set_sockets;
//#endif
	FD_ZERO(&set_sockets);
	FD_SET(server_, &set_sockets);

	// Now wait until we see a client
	if (protocol_ == HTTP)
	{
		client_ = INVALID_SOCKET;
		client_status result = BLOCK;
		while (client_ == INVALID_SOCKET && result == BLOCK && !closing_)
		{
			result = accept_client();
			this_thread::yield();
		}
		if (result == NG)
		{
			return 1;
		}
	}

	bool packet_complete = false;
	int payload_size = 0;
	string resource = "";
	string function = "";
	int pos_payload = 0;
	LEN_SOCKET_ADDR len_client_addr = sizeof(client_addr_);
	do
	{
		// Keep processing packets while they keep coming
		switch (protocol_)
		{
		case UDP:
			bytes_rcvd = recvfrom(server_, buffer, buffer_len, 0, (SOCKADDR *)&client_addr_, &len_client_addr);
			break;
		case HTTP:
			bytes_rcvd = recv(client_, buffer, buffer_len, 0);
			break;
		}
		if (bytes_rcvd > 0)
		{
			dump(string(buffer, bytes_rcvd));
			mu_packet_.lock();
			string s = string(buffer, bytes_rcvd);
			q_packet_.push(s);
			mu_packet_.unlock();
			Fl::awake(cb_th_packet, this);
		}
#ifdef _WIN32
		else if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			// Try again immediately after letting FLTK in
			this_thread::yield();
		}
		else if (WSAGetLastError() == WSAENOTSOCK && closing_)
		{
			// We can get here through a race between closing and turning the timers off
			delete[] buffer;
			buffer = nullptr;
		}
		else
		{
			handle_error("Unable to read from client");
			delete[] buffer;
			buffer = nullptr;
		}
#else
		else
		{
			// Try again after checking for any fltk events
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
#endif
	} while (!closing_);
	// Now see if we have another - the timer goes on the scheduling queue so other tasks will get in
	closed_ = true;
	this_thread::yield();
	if (buffer) delete[] buffer;
	return 0;
}

int socket_server::send_response(istream &response)
{
	// calculate the data size
	streampos startpos = response.tellg();
	response.seekg(0, ios::end);
	streampos endpos = response.tellg();
	int resp_size = (int)endpos - (int)startpos;
	response.seekg(0, ios::beg);
	//
	char *buffer = new char[resp_size + 1];
	memset(buffer, '\0', resp_size + 1);
	response.read(buffer, resp_size);
	dump(string(buffer));

	// Send the response packet
	int result;
	if (protocol_ == UDP)
	{
		result = sendto(server_, buffer, resp_size, 0, (SOCKADDR *)&client_addr_, sizeof(client_addr_));
	}
	else
	{
		result = send(client_, buffer, resp_size, 0);
	}
	if (result < 0)
	{
		handle_error("Unable to send to");
		return result;
	}
	return 0;
}

// // Call back to restart attempt to restart datagram
// void socket_server::cb_timer_rcv(void *v)
// {
// 	socket_server *that = (socket_server *)v;
// 	that->rcv_packet();
// }

// // Callback to restart attempt to accept client
// void socket_server::cb_timer_acc(void *v)
// {
// 	socket_server *that = (socket_server *)v;
// 	that->accept_client();
// }

// Has a server
bool socket_server::has_server()
{
	return (server_ != INVALID_SOCKET);
}

// Handle error - display error message
void socket_server::handle_error(const char *phase)
{
	char message[1028];
#ifdef _WIN32
	DWORD error_code = WSAGetLastError();
	if (error_code)
	{
		char error_msg[1028];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, error_msg, sizeof(error_msg), nullptr);
		snprintf(message, 1028, "SOCKET: %s: (%d) %s", phase, error_code, error_msg);
	}
	else
	{
		snprintf(message, 1028, "SOCKET: %s", phase);
	}
#else
	char* error_msg = strerror(errno);
	snprintf(message, 1028, "SOCKET: %s %s(%d): %s", phase, address_.c_str(), port_num_, error_msg);
#endif
	status_->misc_status(ST_ERROR, message);
	close_server(false);
}

// Set handlers
void socket_server::callback(int (*request)(stringstream &))
{
	do_request = request;
}

// Diagnostic print
void socket_server::dump(string data)
{
	string escaped = "";
	bool newline = false;
	// For every data byte
	for (auto it = data.begin(); it != data.end(); it++)
	{
		unsigned char c = *it;
		if (c < 32 || c > 0x7F)
		{
			// Convert other non-printable or non-ASCII characters to hex escaped form
			if (newline)
			{
				escaped += '\n';
				newline = false;
			}
			char hex[10];
			snprintf(hex, 10, "\\x%02x", c);
			escaped += hex;
		}
		else if (c == '\\')
		{
			if (newline)
			{
				escaped += '\n';
				newline = false;
			}
			// Convert '\' to '\\'
			escaped += "\\\\";
		}
		else
		{
			if (newline)
			{
				escaped += '\n';
				newline = false;
			}
			escaped += *it;
		}
	}
	// printf("%s\n", escaped.c_str());
}

// main thread side handle packet
void socket_server::cb_th_packet(void *v)
{
	socket_server *that = (socket_server *)v;
	// Lock queue while empty it
	that->mu_packet_.lock();
	while (!that->q_packet_.empty())
	{
		string s = that->q_packet_.front();
		stringstream ss;
		ss.clear();
		ss << s;
		that->q_packet_.pop();
		that->mu_packet_.unlock();
		// Process packet having unlocked queue to allow another packet in
		that->do_request(ss);
		that->mu_packet_.lock();
	}
	that->mu_packet_.unlock();
}

// Thread runner
void socket_server::thread_run(socket_server *that)
{
	that->rcv_packet();
}