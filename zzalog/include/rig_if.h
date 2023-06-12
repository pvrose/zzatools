#ifndef __RIG_IF__
#define __RIG_IF__

// zzalib includes

#include "rpc_data_item.h"
#include "rpc_handler.h"
#include "callback.h"
#include "utils.h"

// hamlib icludes
#include "hamlib/rig.h"

// C/C++ includes
#include <string>
#include <thread>
#include <atomic>

using namespace std;

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
		GM_DIGU = '9'
	};

	// slow rig polling - 1s -> 10min (default 1 min)
	const int SLOW_RIG_TIMER = 60;
	// fast rig polling - 20ms -> 2s (default 1s);
	const int FAST_RIG_TIMER = 1;

	// This class is the base class for rig handlers. 
	class rig_if
	{
	public:
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
		};

		rig_if(const char* name, hamlib_data_t data);
		~rig_if();

		// Values read from rig
		struct rig_values {
			atomic<double> tx_frequency;
			atomic<double> rx_frequency;
			atomic<rig_mode_t> mode;
			atomic<double> drive ;
			atomic<bool> split;
			atomic<int> s_value;
			atomic<double> pwr;
			atomic<bool> ptt;
			
			rig_values() {
				tx_frequency = 0.0;
				rx_frequency = 0.0;
				mode =GM_CWU;
				drive = 0.0;
				split = false;
				s_value = -54;
				pwr = 0.0;
				ptt = false;
			}
		};

		// Opens the COM port associated with the rig
		bool open();
		// Return rig name
		string& rig_name();
		// Formatted values
		string rig_info();
		// Return the most recent error message
		string error_message(const char* func_name);

		// Error Code is not OK.
		bool is_good();
		// close rig - may be null for some 
		void close();

		// Port was successfully opened
		bool is_open();
		// Rig timer callback
		void ticker();

		// return mode/submode
		void get_string_mode(string& mode, string& submode);
		// return frequency
		string get_frequency(bool tx);
		// return power
		string get_tx_power();
		// return S-meter reading
		string get_smeter();
		// Return PTT value
		bool get_ptt();
		// Get the data from the rig
		static void th_run_rig(rig_if* that);
		// Protected attributes
	protected:
		void read_values();
		// Open rig - run in thread
		bool open_rig();
		// Rig opened OK
		atomic<bool> opened_ok_;
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
		hamlib_data_t hamlib_data_;
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
};
#endif
