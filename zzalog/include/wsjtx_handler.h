#ifndef __WSJTX_HANDLER__
#define __WSJTX_HANDLER__

#include <string>
#include <cstdint>
#include <map>
#include <set>
#include <atomic>

#ifdef _WIN32
#include <WinSock2.h>
#endif



class record;
class socket_server;

	//! This class provides the interface to handle UDP datagrams as a server to the WSJT-X client(s)
	class wsjtx_handler
	{

	public:
		//! Consructor.
		wsjtx_handler();
		//! Destructor.
		~wsjtx_handler();

		//! Returns true if server has been started
		static bool has_server();
		//! Start server
		void run_server();
		//! Close servver
		void close_server();

		//! Search for match for \p qso in ALL.TXT and update \p qso if \p update_qso is true.
		bool match_all_txt(record* qso, bool update_qso);

		//! Called by cb_ticker callback.
		void ticker();
		//! Callback from ticker every 15 seconds
		static void cb_ticker(void* v);
		//! Remove \p call from gridsquare cache
		void delete_qso(std::string call);

		//! Return true if a packet has been received.
		static bool has_data();

		//! Used in static methods to point to the single instance of this class
		static wsjtx_handler* that_;

	protected:

		//! The datagram gets sent for every decode
		struct decode_dg {
			std::string id;          //!< Unique key for WSJT-X instance
			bool new_decode;    //!< If true indicates anew decode
			uint32_t time;      //!< Time (in milliseconds) since midnight UTC
			int32_t snr;        //!< S+N/N ratio
			double d_time;      //!< Delta time of decode
			uint32_t d_freq;    //!< Delta frequency from VFO (in hertz)
			std::string mode;        //!< Mode
			std::string message;     //!< Decoded mesasge
			bool low_confidence;//!< If true decode is of low confidence
			bool off_air;       //!< If true decoded from file

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

		//! The datagram gets sent every change in the status
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
				* TX Message
			*/
			std::string id;                    //!< Unique key
			uint64_t dial_freq{ 0 };      //!< Dial frequency (in hertz)
			std::string mode;                  //!< Mode
			std::string dx_call;               //!< Value in DX Call window
			std::string report;                //!< Value in report window
			std::string tx_mode;               //!< Transmit mode
			bool tx_eanbled{ false };     //!< Transmit enabled
			bool transmitting{ false };   //!< Transmitting
			bool decoding{ false };       //!< Decoding
			uint32_t rx_offset{ 0 };      //!< Receive offset frequency (in hertz)
			uint32_t tx_offset{ 0 };      //!< Transmit offset frequency (in hertz)
			std::string own_call;              //!< Station callsign
			std::string own_grid;              //!< Staton gridsquare
			std::string dx_grid;               //!< Value in DX Gridsquare window
			bool tx_watchdog{ false };    //!< Transmit watchdog triggered
			std::string submode;               //!< Submode
			bool fast_mode{ false };      //!< Fast decode mode std::set
			uint8_t special_op{ 0 };      //!< Special operation 
			uint32_t freq_tolerance{ 0 }; //!< Frequency tolerance
			uint32_t tx_rx_period{ 0 };   //!< Transmit/Receive period
			std::string config_name;           //!< Configuration name
			std::string tx_message;            //!< Message sent by user.
		};

		//! Represents the message format being sent (and received), so represents the state of the QSO
		enum message_t {
			TX1,       //!< CALL1 CALL2 GRID
			TX1A,      //!< CALL1 CALL2 - no excahnge 
			TX2,       //!< CALL1 CALL2 Report
			TX3,       //!< CALL1 CALL2 R-report
			TX4,       //!< CALL1 CALL2 RRR
			TX4A,      //!< CALL1 CALL2 RR73
			TX5,       //!< CALL1 CALL2 73
			TX6,       //!< CQ +  CALL2 GRID
			TX6A,      //!< CQ + CALL2  - no exchange
			MISC,      //!< Miscellaneous transmit (eg TUNE)
			TEXT,      //!< Any other decode
		};

		//! Mapping from message state to text representation.
		std::map<message_t, std::string> mess_char_ = {
			{ TX1, "TX1"},
			{ TX1A, "TX1A"},
			{ TX2, "TX2"},
			{ TX3, "TX3"},
			{ TX4, "TX4"},
			{ TX4A, "TX4A"},
			{ TX5, "TX5"},
			{ TX6, "TX6"},
			{ TX6A, "TX6A"},
			{ MISC, "MISC"},
			{ TEXT, "TEXT"}
		};

		//! The message decoded
		struct decoded_msg {
			message_t type{ TX1 };  //!< Message format
			std::string target;		    //!< The call the message is for
			std::string sender;		    //!< The call the message is from
			std::string exchange;	    //!< The message payload (grid, report)
		};

		//! Callback from server std::thread: Receive a datagram from WSJT-X, returns the type of datagram.
		static int rcv_request(std::stringstream& os);
		//! Receive a datagram from WSJT-X, returns the type of datagram.
		int rcv_dgram(std::stringstream& os);

		//! handle heartbeat datagram from output stream \p os.
		int handle_hbeat(std::stringstream& os);
		//! send heartbeat datagram
		int send_hbeat();
		//! handle "Logged ADIF" datagram from output stream \p os.
		int handle_log(std::stringstream& os);
		//! handle "Close" datagram from output stream \p os.
		int handle_close(std::stringstream& os);
		//! handle default datagrams from output stream \p os.
		int handle_default(std::stringstream& os, uint32_t type);
		//! handle decode from output stream \p os.
		int handle_decode(std::stringstream& os);
		//! handle decode from output stream \p os.
		int handle_reply(std::stringstream& os);
		//! handle status from output stream \p os.
		int handle_status(std::stringstream& os);
		//! Takes the utf8 from the dgram  from output stream \p os.
		std::string get_utf8(std::stringstream& os);
		//! Takes 32-bit unsigned integer from output stream \p os.
		uint32_t get_uint32(std::stringstream& os);
		//! Takes 64-bit unsigned integer from output stream \p os.
		uint64_t get_uint64(std::stringstream& os);
		//! Takes bool from output stream \p os.
		bool get_bool(std::stringstream& os);
		//! Takes 8-bit unsigned int from output stream \p os.
		uint8_t get_uint8(std::stringstream& ss);
		//! Takes double from output stream \p os.
		double get_double(std::stringstream& ss);
		//! Adds the utf8 to the dgram
		void put_utf8(std::stringstream& os, std::string s);
		//! Adds 32-but unsigned integer
		void put_uint32(std::stringstream& os, const unsigned int i);

		//! Decode the message and return message type.
		decoded_msg decode_message(std::string message);
		//! Update QSO with message
		
		//! \param tx If true transmit, otherwise receive.
		//! \param time Time of QSO, Date taken from system clock.
		//! \param audio_freq Transmit offset (in hertz)
		//! \param message Transmit or Received message.
		//! \param match QSO record to update, if nullptr create a new one.
		//! \param dial_frequency Dial frequency.
		//! \param mode Mode.
		record* update_qso(bool tx, std::string time, double audio_freq, std::string message,
		    record* match = nullptr, double dial_frequency = 0.0, std::string mode = "");

		//! Returns the QSO record for this callsign, if necessary create it.
		record* qso_call(std::string call, bool create);
		//! Creates and returns a QSO record for this call
		record* new_qso(std::string call);
		//! Parse the ALL.TXT line against the \p match QSO record: returns true if it matches.
		bool parse_all_txt(record* match, std::string line);

		//! Socket server
		socket_server* server_;

		//! Received magic number
		uint32_t magic_number_;
		//! Received schema number
		uint32_t schema_;
		//! Expected magic number
		const uint32_t expected_magic_ = 0xadbccbda;
		//! Min. schema supported
		uint32_t minimum_schema_ = 2;
		//! Received first datagram
		bool received_datagram_;
		//! Number status received
		unsigned int status_rcvd_;
		//! Station callsign
		std::string my_call_;
		//! Station callsign in angle brackets.
		std::string my_bracketed_call_;
		//! Previous status datagram
		status_dg prev_status_;
		//! Cache of grid locations seen: Maps callsign to gridsquare.
		std::map<std::string, std::string> grid_cache_;
		//! Map of callsigns to QSO records.
		std::map<std::string, record*> qsos_;
		//! Dial frequency (MHz)
		double dial_frequency_;
		//! Current mode
		std::string mode_;
		//! Received partner ID
		std::string id_;

		// Heartbeat management -
		//! Reeceived heartbeats
		std::set<std::string> received_beats_;
		//! Check missing heartbeats
		bool check_beats_;
		//! WSJT-X connected
		bool connected_;
		//! Semaphore to prevent multiple creations of a QSO for the same status 
		std::atomic_flag tx_semaphore_;
		
	};

#endif
