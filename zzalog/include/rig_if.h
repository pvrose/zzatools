#ifndef __RIG_IF__
#define __RIG_IF__

#include "callback.h"
#include "utils.h"

// hamlib icludes
#include "hamlib/rig.h"

// C/C++ includes
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>


	//! Internal representation of the mode read from the rig.
	enum rig_mode_t {
		GM_INVALID = 0,   //!< Invalid or unsupported mode
		GM_LSB = '1',     //!< LSB
		GM_USB = '2',     //!< USB
		GM_CWU = '3',     //!< CW - VFO frequency below signal frequency
		GM_FM = '4',      //!< FM
		GM_AM = '5',      //!< AM
		GM_DIGL = '6',    //!< Digital mode: Modulated as LSB
		GM_CWL = '7',     //!< CW - VFO frequency above signal frequency
		GM_DIGU = '9',    //!< Digital mode: Modulated as USB
		GM_DSTAR = 'A'    //!< Digital voice: Icom DSTAR.
	};

	//! slow rig polling - 1s -> 10min (default 1 min)
	const int SLOW_RIG_TIMER = 60;
	//! fast rig polling - 20ms -> 2s (default 1s);
	const int FAST_RIG_TIMER = 1;

	//! Method by which power level read from rig is converted to a power reading.
	enum power_mode_t : uchar {
		NO_POWER,        //!< No power returnable
		RF_METER,        //!< Read the RF power out meter directly.
		DRIVE_LEVEL,     //!< Read the drive level meter and multiply by maximum poer.
		MAX_POWER        //!< Use the specified maximum power.
	};

	//! Method by which frequency is provided.
	enum freq_mode_t : uchar {
		NO_FREQ,         //!< No frequency available
		VFO,             //!< Read a VFO
		XTAL             //!< Fixed freqency
	};

	//! Used to add accessories.
	enum accessory_t : uchar {
		BAREBACK,        //!< No accessory
		AMPLIFIER = 1,   //!< Amplifer attached
		TRANSVERTER = 2, //!< Transverter attached
		BOTH = AMPLIFIER | TRANSVERTER  //!< Both amplifier and transverter attached.
	};

	//! Interface configuration data:
	struct hamlib_data_t {
		//! Manufacturer as known by hamlib.
		std::string mfr = "";
		//! Model as known by hamlib.
		std::string model = "";
		//! Port name used by hamlib.
		std::string port_name = "";
		//! Baud rate used by hamlib.
		int baud_rate = 9600;
		//! Model ID - index into hamlib capabilities table.
		rig_model_t model_id = -1;
		//! Port type used by hamlib.
		rig_port_t port_type = RIG_PORT_NONE;
		//! Maximum number of timeouts allowed before ignore meter
		int max_to_count = 5;
		// additional features required by rig_if to return data
		//! Timeout value (not a hamlib item
		double timeout = 1.0;
		//! S-meter reading std::queue length
		int num_smeters = 5;
		//! Default power mode.
		power_mode_t power_mode = RF_METER;
		//! Maximum power level (in watts)
		double max_power = 0.0;
		//! Default frequency mode
		freq_mode_t freq_mode = VFO;
		//! Fixed frequency (in megahertz)
		double frequency = 0.0;
		//! Amplifer and transverter defaults
		accessory_t accessory = BAREBACK;
		//! Spcified amplifier gain (in decibels)
		int gain = 0;
		//! Specified transverter power (in watts)
		double tvtr_power = 0.0;
		//! Specified transverter local oscillator frequency (in megawatts) 
		double freq_offset = 0.0;
	};

	//! This class is the base class for rig handlers. 
	class rig_if
	{
	public:

		//! Constructor.
		
		//! \param name Name as known to the user.
		//! \param data interface configuration data.
		rig_if(const char* name, hamlib_data_t* data);
		//! Destructor.
		~rig_if();

		//! Values read from rig
		struct rig_values {
			std::atomic<double> tx_frequency;  //!< Transmit Frequency (in megahertz)
			std::atomic<double> rx_frequency;  //!< Receive Frequency (in megahertz)
			std::atomic<rig_mode_t> mode;      //!< Transmit mode
			std::atomic<double> drive ;        //!< Drive level (fraction)
			std::atomic<bool> split;           //!< Split mode.
			std::atomic<int> s_value;          //!< Smoothed S-meter reading (in decibels)
			std::atomic<int> s_meter;          //!< Immediate S-meter reading (in decibels)
			std::atomic<double> pwr_value;     //!< Smoothed RF power meter reading (in watts)
			std::atomic<double> pwr_meter;     //!< Immediate power meter reading (in watts)
			std::atomic<bool> ptt;             //!< If true indicates transmitting otherwise receiving.
			std::atomic<bool> slow;            //!< Rig is not responding 
			std::atomic<bool> powered_on;      //!< Rig appears powered on.
			
			//! Constrctor.
			rig_values() {
				tx_frequency = 0.0;
				rx_frequency = 0.0;
				mode =GM_CWU;
				drive = 0.0;
				split = false;
				s_value = -54;
				s_meter = -54;
				pwr_value = 0.0;
				pwr_meter = 0.0;
				ptt = false;
				slow = false;
				powered_on = false;
			}
		};

		//! Opens the connection to the rig
		bool open();
		//! Returns rig name
		std::string& rig_name();
		//! Returns the most recent error message and adds \p func_name.
		std::string error_message(const char* func_name);

		//! Returns that the rig connection is not in an error state.
		bool is_good();
		//! close rig - may be null for some 
		void close();

		// Types of error
		//! Returns true if the last error was a network error.
		bool is_network_error();
		//! Returns true if the last error was reported by the rig.
		bool is_rig_error();

		//! Returns true if the port was successfully opened
		bool is_open();
		//! Returns true if the port is being opened.
		bool is_opening();
		//! Returns true if the port has no CAT
		bool has_no_cat();

		//! Receives \p mode and \p submode.  
		void get_string_mode(std::string& mode, std::string& submode);
		//! Returns frequency as std::string (in megahertz to 1 hertz resolution)
		std::string get_frequency(bool tx);
		//! Returns frequency as double (in megahertz)
		double get_dfrequency(bool tx);
		//! Returns power (in watts): \p max maximum value over the transmit period.
		std::string get_tx_power(bool max = true);
		//! Returns power (in watts): \p max maximum value over the transmit period.
		double get_dpower(bool max = true);
		//! Returns S-meter reading - \p max - maximum over receive perion, false = instatntaneous
		std::string get_smeter(bool max = true);
		//! Returns PTT value: true indicates transmit.
		bool get_ptt();
		//! Returns Split value
		bool get_split();
		//! Run in std::thread to get the data from the rig
		static void th_run_rig(rig_if* that);
		//! Returns true if the rig is taking over 1 second to access
		bool get_slow();
		//! Returns true if thr rig appears powered.
		bool get_powered();
		//! Open rig in rig access std::thread.
		static void th_sopen_rig(rig_if* that);
		//! Callback from rig std::thread if an error is detected.
		static void cb_rig_error(void* v);
		//! Callback from rig std::thread if a warning is detected.
		static void cb_rig_warning(void* v);
		//! std::set frequency (in megahertz)
		bool set_frequency(double f);


		// Protected attributes
	protected:
		//! Runs in rig std::thread to poll values every 1 second.
		bool th_read_values();
		//! Open rig - run in std::thread
		void th_open_rig(rig_if* that);
		//! Handle errors.
		
		//! \param code Error code
		//! \param meter Name of meter value being accessed.
		//! \param flag Pointer to a Boolean value that if std::set inhibits further attempts to acccess.
		//! \param to_count Number of accesses allowed befor further ones are inhibited.
		//! \return true indicates error prevents further access.
		bool error_handler(int code, const char* meter, bool* flag, int* to_count);
		//! Rig opened OK
		std::atomic<bool> opened_ok_;
		//! Semaphore to use around opening
		std::atomic<bool> opening_;
		//! Full rig name
		std::string full_rig_name_;

		//! Returns error message for error \p code.
		const char* error_text(rig_errcode_e code);

		//! Name to log as MY_RIG
		std::string my_rig_name_;
		//! Interface specific attributes
		hamlib_data_t* hamlib_data_;
		//! Hamlib rig interface
		RIG* rig_;
		//! Numeric error code
		int error_code_;
		//! Values polled from rig.
		rig_values rig_data_;
		//! Timer count down
		int count_down_;
		//! Thread in whcih to run rig access.
		std::thread* thread_;
		//! Keep rig std::thread running.
		std::atomic<bool> run_read_;

		//! The time of the last PTT off - to decide if it's a new transmission.
		std::chrono::system_clock::time_point last_ptt_off_;

		//! The most recent S-meter readings: used for smoothing the value read.
		std::vector<int> smeters_;
		//! Cumulated value of smeter readings
		int sum_smeters_;
		// Flags to avoid unsupported meters
		bool has_smeter_;     //!< S-meter appears to be supported.
		bool has_drive_;      //!< Drive meter appears to be supported.
		bool has_rf_meter_;   //!< RF Power meter appears to be supported.

		//! Timeout counts
		int toc_split_;
		//! Function being performed - for error debug mostly
		std::string read_item_;


};
#endif
