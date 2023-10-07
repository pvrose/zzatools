#ifndef __SOCKET_SERVER__
#define __SOCKET_SERVER__

#include "utils.h"

#include <string>
#include <cstdint>
#include <thread>
#include <mutex>
#include <queue>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#define SOCKET int
#define SOCKADDR_IN sockaddr_in
#endif

using namespace std;

// This class provides an interface to use a generic socket as a server.




	class socket_server
	{

	public:
		enum protocol_t {
			HTTP,
			UDP
		};
		socket_server(protocol_t protocol, string address, int port_num);
		~socket_server();

		// Close the socket
		void close_server();
		// Start listening
		void run_server();
		// Is this listening
		bool has_server();
		// Set handler 
		void callback(int(*do_request)(stringstream&));
		// Send response
		int send_response(istream& response);

	protected:

		enum client_status {
			OK = 0,
			NG = 1,
			BLOCK = 2
		};

		// Receive a datagram from WSJT-X
		int rcv_packet();
		// Accept the client
		client_status accept_client();
		// Error handler
		void handle_error(const char* phase);
		// Send request - set by call-back
		int (*do_request)(stringstream& request);

		// Open socket and create server
		int create_server();
		// Timer to wait before testing receive
		static void cb_timer_rcv(void* v);
		// Timer to wait before trying to accept again
		static void cb_timer_acc(void* v);
		// Diagnostic data
		void dump(string line);
		// Call back to handle packets - pass pointer to socket serevr
		static void cb_th_packet(void* v);
		// Thread runner
		static void thread_run(socket_server* that);

		// Server socket
		SOCKET server_;
		// HTTP Client
		SOCKET client_;
		// Current client address
		SOCKADDR_IN client_addr_;
		// Previous client address
		string prev_addr_;
		int prev_port_;
		// Host IP address e.g. 127.0.0.1
		string host_id_;
		// port number
		int port_num_;
		// Server address
		string address_;
		// protocol
		protocol_t protocol_;
		// Closure in progress
		bool closing_;
		// Thread handling
		thread* th_socket_;
		// Packet queue
		queue<string> q_packet_;
		// Packet queue lock
		mutex mu_packet_;

	};

#endif

