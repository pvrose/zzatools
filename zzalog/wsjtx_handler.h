#ifndef __WSJTX_HANDLER__
#define __WSJTX_HANDLER__

#include <string>
#include <cstdint>
#include <WinSock2.h>

using namespace std;

namespace zzalog {

	class wsjtx_handler
	{

	public:
		wsjtx_handler();
		~wsjtx_handler();

		// Close the socket
		void close_socket();
		// Start listening
		void run_server();
		// Is this listening
		bool has_server();

	protected:

		// Receive a datagram from WSJT-X
		int rcv_dgram();
		// handle heartbeat datagram
		int handle_hbeat(char* const dgram, const int len);
		// send heartbeat datagram
		int send_hbeat();
		// handle "Logged ADIF" datagram
		int handle_log(char* const dgram, const int len);
		// handle "Close" datagram
		int handle_close(char* const dgram, const int len);
		// handle default datagrams
		int handle_default(char* const dgram, const int len);
		// handle decode
		int handle_decode(char* const dgram, const int len);
		// handle status
		int handle_status(char* const dgram, const int len);
		// Get the utf8 from the dgram
		char* get_utf8(char* dgram, string* s);
		// Get 32-bit unsigned integer
		char* get_uint(char* dgram, unsigned int* i);
		// Get 64-bit unsigned integer
		char* get_uint64(char* dgram, uint64_t* i);
		// Get bool
		char* get_bool(char* dgram, bool* b);
		// Get 8-bit unsigned int
		char* get_uint8(char* dgram, uint8_t* i);
		// Get double
		char* get_double(char* dgram, double* d);
		// Add the utf8 to the dgram
		char* put_utf8(char* dgram, string s);
		// Add 32-but unsigned integer
		char* put_uint(char* dgram, const unsigned int i);
		// Send a datagaram
		int send_dgram(const char* dgram, const int len);
		// Error handler
		void handle_error(const char* phase);

		// Print heartbeat
		void print_hbeat(char* const dgram);

		// Open socket and create server
		int create_server();
		// Timer to wait a couple of seconds between trying to read a datagram
		static void cb_timer_wait(void* v);

		// Server socket
		SOCKET server_;
		// Current client address
		SOCKADDR_IN client_addr_;
		// Wait flag
		bool wait_flag_;
		// Received magic number
		unsigned int magic_number_;
		// Received schema number
		unsigned int schema_;
		// Expected maginc number
		const unsigned int expected_magic_ = 0xadbccbda;
		// Min. schema supported
		unsigned int minimum_schema_ = 2;
		// Received first datagram
		bool received_datagram_;
	};

}

#endif