#ifndef __WSJTX_HANDLER__
#define __WSJTX_HANDLER__

#include "../zzalib/socket_server.h"

#include <string>
#include <cstdint>
#include <WinSock2.h>

using namespace std;
using namespace zzalib;

namespace zzalog {

	class wsjtx_handler
	{

	public:
		wsjtx_handler();
		~wsjtx_handler();

		// We have a server
		bool has_server();
		// Start server
		void run_server();
		// Close servver
		void close_server();


		static wsjtx_handler* that_;

	protected:

		// Receive a datagram from WSJT-X
		static int rcv_request(stringstream& os);
		// Receive a datagram from WSJT-X
		int rcv_dgram(stringstream& os);

		// handle heartbeat datagram
		int handle_hbeat(stringstream& os);
		// send heartbeat datagram
		int send_hbeat();
		// handle "Logged ADIF" datagram
		int handle_log(stringstream& os);
		// handle "Close" datagram
		int handle_close(stringstream& os);
		// handle default datagrams
		int handle_default(stringstream& os, uint32_t type);
		// handle decode
		int handle_decode(stringstream& os);
		// handle status
		int handle_status(stringstream& os);
		// Get the utf8 from the dgram
		string get_utf8(stringstream& os);
		// Get 32-bit unsigned integer
		uint32_t get_uint32(stringstream& os);
		// Get 64-bit unsigned integer
		uint64_t get_uint64(stringstream& os);
		// Get bool
		bool get_bool(stringstream& os);
		// Get 8-bit unsigned int
		uint8_t get_uint8(stringstream& ss);
		// Get double
		double get_double(stringstream& ss);
		// Add the utf8 to the dgram
		void put_utf8(stringstream& os, string s);
		// Add 32-but unsigned integer
		void put_uint32(stringstream& os, const unsigned int i);

		// Timer to wait a couple of seconds between trying to read a datagram
		static void cb_timer_wait(void* v);

		// Socket server
		socket_server* server_;

		// Received magic number
		uint32_t magic_number_;
		// Received schema number
		uint32_t schema_;
		// Expected magic number
		const uint32_t expected_magic_ = 0xadbccbda;
		// Min. schema supported
		uint32_t minimum_schema_ = 2;
		// Received first datagram
		bool received_datagram_;
		// Last decode received
		uint32_t last_decode_time_;
		// Number decodes received
		unsigned int decodes_rcvd_;
	};

}

#endif