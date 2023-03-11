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
	const double SLOW_RIG_MAX = 600.0;
	const double SLOW_RIG_MIN = 1.0;
	const double SLOW_RIG_DEF = 60.0;
	// fast rig polling - 20ms -> 2s (default 1s);
	const double FAST_RIG_MAX = 2.0;
	const double FAST_RIG_MIN = 0.02;
	const double FAST_RIG_DEF = 1.0;

	// This class is the base class for rig handlers. 
	class rig_if
	{
	public:
		rig_if();
		~rig_if();

		// Opens the COM port associated with the rig
		bool open();
		// Return rig name
		string& rig_name();
		// Read TX Frequency
		double tx_frequency();
		// Read mode from rig
		rig_mode_t mode();
		// Return drive level * 100% power
		double drive();
		// Rig is working split TX/RX frequency
		bool is_split();
		// Get separate frequency
		double rx_frequency();
		// Return S-meter reading (S9+/-dB)
		int s_meter();
		// Return power meter reading
		double pwr_meter();
		// Return the most recent error message
		string error_message(string func_name);
		// Get TX mode
		bool get_tx();
		// Get SWR meter
		double swr_meter();
		// Get Voltage meter
		double vdd_meter();

		// Error Code is not OK.
		bool is_good();
		// close rig - may be null for some 
		void close();

		// Port was successfully opened
		bool is_open();
		// Get the rig info to display
		string rig_info();
		// Rig timer callback
		static void cb_timer_rig(void* v);

		// return mode/submode
		void get_string_mode(string& mode, string& submode);
		// return frequency
		string get_frequency(bool tx);
		// return power
		string get_tx_power();
		// return S-meter reading
		string get_smeter();
		// Return SWR meter
		string get_swr_meter();
		// Return open message
		string success_message();

		// Callback to set certain functions (timer callback, freq to band conversion, error message
		void callback(void (*function)(), string(*spec_func)(double), void(*mess_func)(status_t, const char*));


		// Protected methods
	protected:

		// Protected attributes
	protected:
		// Name of the rig
		string rig_name_;
		// Name of the rig (prepended with manufacturer)
		string mfg_name_;
		// Rig opened OK
		bool opened_ok_;
		// Text error message
		string error_message_;
		// Text success message
		string success_message_;
		// On timer callback to update
		void(*on_timer_)();
		// Access spec for 
		string(*freq_to_band_)(double frequency);
		// Display message
		void(*error)(status_t ok, const char* message);
		// bool
		bool have_freq_to_band_;
		// have reported high SWR
		bool reported_hi_swr_;
		// last band read
		string previous_band_;
		// Stop incessane errors
		bool inhibit_repeated_errors;
		// Full rig name
		string full_rig_name_;

		// Re-implement error message
		const char* error_text(rig_errcode_e code);

		// Hamlib specific attributes
		// Current rig
		string current_rig_;
		// COM port name
		string port_name_;
		// COM baud rate
		int baud_rate_;
		// Rig interface
		RIG* rig_;
		// Numeric ID of rig
		rig_model_t model_id_;
		// Numeric error code
		int error_code_;
		// Reported error code
		bool unsupported_function_;
	};
#endif
