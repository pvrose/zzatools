#ifndef __WSJTX_HANDLER__
#define __WSJTX_HANDLER__

#include "socket_server.h"

#include <string>
#include <cstdint>
#include <map>
#ifdef _WIN32
#include <WinSock2.h>
#endif

using namespace std;

	// This class provides the interface to handle UDP datagrams as a server to the
	// WSJT-X client(s)
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

		// Used in static methods to point to the single instance of this class
		static wsjtx_handler* that_;

	protected:

		struct decode_dg {
			string id;
			bool new_decode;
			uint32_t time;
			int32_t snr;
			double d_time;
			uint32_t d_freq;
			string mode;
			string message;
			bool low_confidence;
			bool off_air;

			decode_dg() {
				id = "";
				new_decode = false;
				time = 0;
				snr = 0;
				d_time = 0.0;
				d_freq = 0;
				mode = "";
				message = "";
				low_confidence = false;
				off_air = false;
			}
		};

		struct status_dg {
			/*Status        Out       1                      quint32
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
				*
			*/
			string id;
			uint64_t dial_freq;
			string mode;
			string dx_call;
			string report;
			string tx_mode;
			bool tx_eanbled;
			bool transmitting;
			bool decoding;
			uint32_t rx_offset;
			uint32_t tx_offset;
			string own_call;
			string own_grid;
			string dx_grid;
			bool tx_watchdog;
			string submode;
			bool fast_mode;
			uint8_t special_op;
			uint32_t freq_tolerance;
			uint32_t tx_rx_period;
			string config_name;
		};

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
		// New heartbeat received
		bool new_heartbeat_;
		// Number status received
		unsigned int status_rcvd_;
		// My call
		string my_call_;
		// My bracketed call
		string my_bracketed_call_;
		// Previous status datagram
		status_dg prev_status_;
		// Grid locations worked
		map<string, string> grid_cache_;
	};

#endif
