#ifndef __SOCKET_SERVER__
#define __SOCKET_SERVER__

#include "utils.h"

#include <string>
#include <cstdint>
#include <WinSock2.h>

using namespace std;

// This class provides an interface to use a generic socket as a server.

namespace zzalib {


	class socket_server
	{

	public:
		enum protocol_t {
			HTTP,
			UDP
		};
		socket_server(protocol_t protocol, int port_num);
		~socket_server();

		// Close the socket
		void close_server();
		// Start listening
		void run_server();
		// Is this listening
		bool has_server();
		// Set handler 
		void callback(int(*do_request)(stringstream&), void(*message)(status_t, const char*));
		// Send response
		int send_response(istream& response);

	protected:

		// Receive a datagram from WSJT-X
		int rcv_packet();
		// Error handler
		void handle_error(const char* phase);
		// Send request - set by call-back
		int (*do_request)(stringstream& request);
		// Error message
		void (*status)(status_t level, const char* message);

		// Open socket and create server
		int create_server();
		// Timer to wait before testing receive
		static void cb_timer_udp(void* v);
		static void cb_timer_http(void* v);
		// Diagnostic data
		void dump(string line);
		// Server socket
		SOCKET server_;
		// HTTP Client
		SOCKET client_;
		// Current client address
		SOCKADDR_IN client_addr_;
		// Host IP address e.g. 127.0.0.1
		string host_id_;
		// port number
		int port_num_;
		// protocol
		protocol_t protocol_;
		// Closure in progress
		bool closing_;

	};

}

#endif

