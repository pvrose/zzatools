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

using namespace std;
using namespace std::chrono;

	// Encoding of result from reading the mode from the rig
	enum rig_mode_t {
		GM_INVALID = 0,
		GM_LSB = '1',
		GM_USB = '2',
		GM_CWU = '3',
		GM_FM = '4',
		GM_AM = '5',
		GM_DIGL = '6',
		GM_CWL = '7',
		GM_DIGU = '9',
		GM_DSTAR = 'A'
	};

	// slow rig polling - 1s -> 10min (default 1 min)
	const int SLOW_RIG_TIMER = 60;
	// fast rig polling - 20ms -> 2s (default 1s);
	const int FAST_RIG_TIMER = 1;

	enum power_mode_t : uchar {
		NO_POWER,        // No power returnable
		RF_METER,        // Can read the RF power out meter
		DRIVE_LEVEL,     // else, can read the power drive level
		MAX_POWER        // else supply a default value 
	};

	enum freq_mode_t : uchar {
		NO_FREQ,         // No frequency available
		VFO,             // Can read a VFO
		XTAL             // else supply a specified (crystal) value
	};

	enum accessory_t : uchar {
		BAREBACK,        // No accessory
		AMPLIFIER = 1,   // Amplifer attached
		TRANSVERTER = 2, // Transverter attached
		BOTH = AMPLIFIER | TRANSVERTER
	};

	// Hamlib parameters 
	struct hamlib_data_t {
		// Manufacturer
		string mfr = "";
		// Model
		string model = "";
		// Portname
		string port_name = "";
		// Baud rate
		int baud_rate = 9600;
		// Model ID - as known by hamlib
		rig_model_t model_id = -1;
		// Port type
		rig_port_t port_type = RIG_PORT_NONE;
		// additional features required by rig_if to return data
		// Timeout value (not a hamlib item
		double timeout = 1.0;
		// S-meter reading queue length
		int num_smeters = 5;
		// Power defaults
		power_mode_t power_mode = RF_METER;
		double max_power = 0.0;
		// Frequency defaults
		freq_mode_t freq_mode = VFO;
		double frequency = 0.0;
		// Amplifer and transverter defaults
		accessory_t accessory = BAREBACK;
		int gain = 0;
		double tvtr_power = 0.0;
		double freq_offset = 0.0;
	};

	// This class is the base class for rig handlers. 
	class rig_if
	{
	public:

		rig_if(const char* name, hamlib_data_t* data);
		~rig_if();

		// Values read from rig
		struct rig_values {
			atomic<double> tx_frequency;
			atomic<double> rx_frequency;
			atomic<rig_mode_t> mode;
			atomic<double> drive ;
			atomic<bool> split;
			atomic<int> s_value;
			atomic<int> s_meter;
			atomic<double> pwr_value;
			atomic<double> pwr_meter;
			atomic<bool> ptt;
			atomic<bool> slow;
			atomic<bool> powered_on;
			
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

		// Opens the COM port associated with the rig
		bool open();
		// Return rig name
		string& rig_name();
		// Return the most recent error message
		string error_message(const char* func_name);

		// Error Code is not OK.
		bool is_good();
		// close rig - may be null for some 
		void close();

		// Types of error
		bool is_network_error();
		bool is_rig_error();

		// Port was successfully opened
		bool is_open();
		// Port is being opened
		bool is_opening();
		// Port has no CAT
		bool has_no_cat();

		// return mode/submode  
		void get_string_mode(string& mode, string& submode);
		// return frequency
		string get_frequency(bool tx);
		// Return frequency as double
		double get_dfrequency(bool tx);
		// return power
		string get_tx_power(bool max = true);
		// return max power as double
		double get_dpower(bool max = true);
		// return S-meter reading - max - maximum over receive perion, false = instatntaneous
		string get_smeter(bool max = true);
		// Return PTT value
		bool get_ptt();
		// Get the data from the rig
		static void th_run_rig(rig_if* that);
		// Get slow - rig taking over 1 s to access
		bool get_slow();
		// Power on-off
		bool get_powered();

		// Protected attributes
	protected:
		void th_read_values();
		// Open rig - run in thread
		bool th_open_rig();
		// Rig opened OK
		atomic<bool> opened_ok_;
		// SEmaphore to use around opening
		atomic<bool> opening_;
		// bool
		bool have_freq_to_band_;
		// last band read
		string previous_band_;
		// Stop incessane errors
		bool inhibit_repeated_errors;
		// Full rig name
		string full_rig_name_;

		// Re-implement error message
		const char* error_text(rig_errcode_e code);

		// MY_RIG name
		string my_rig_name_;
		// Hamlib specific attributes
		hamlib_data_t* hamlib_data_;
		// Rig interface
		RIG* rig_;
		// Numeric error code
		int error_code_;
		// Reported error code
		bool unsupported_function_;
		// Rig values
		rig_values rig_data_;
		// Timer count down
		int count_down_;
		// Read rig thread
		thread* thread_;
		// Stop thread
		atomic<bool> run_read_;

		// The time of the last PTT off - to decide if it's a new over
		system_clock::time_point last_ptt_off_;

		// Queue of s-meter readings
		vector<int> smeters_;
		// Cumulated value of smeter readings
		int sum_smeters_;


};
#endif
