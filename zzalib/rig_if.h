#ifndef __RIG_IF__
#define __RIG_IF__

// zzalib includes

#include "rpc_data_item.h"
#include "rpc_handler.h"
#include "power_matrix.h"
#include "callback.h"

// hamlib icludes
#include "hamlib/rig.h"

// C/C++ includes
#include <string>

using namespace std;

namespace zzalib {

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

	// Rig handler type
	enum rig_handler_t : int {
		RIG_HAMLIB,
		RIG_FLRIG,
		RIG_DIRECT,
		RIG_NONE
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
		virtual ~rig_if();

		// Opens the COM port associated with the rig
		virtual bool open();
		// Return rig name
		virtual string& rig_name() = 0;
		// Read TX Frequency
		virtual double tx_frequency() = 0;
		// Read mode from rig
		virtual rig_mode_t mode() = 0;
		// Return drive level
		virtual double drive() = 0;
		// Rig is working split TX/RX frequency
		virtual bool is_split() = 0;
		// Get separate frequency
		virtual double rx_frequency() = 0;
		// Return S-meter reading (S9+/-dB)
		virtual int s_meter() = 0;
		// Return the most recent error message
		virtual string error_message() = 0;
		// Send a raw message
		virtual string raw_message(string message) = 0;

		// Error Code is not OK.
		virtual bool is_good() = 0;
		// close rig - may be null for some 
		virtual void close();
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
		// Return open message
		string success_message();
		// Convert drive-level to power
		double power();
		// Change power_loopup
		void change_lookup();
		// Update rig clock
		void update_clock();

		// Callback to update app on timer interrupt
		void callback(void (*function)(), string(*spec_func)(double), void(*mess_func)(bool, const char*));


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
		// Handler name
		string handler_;
		// Text error message
		string error_message_;
		// Text success message
		string success_message_;
		// The power lookup matrix
		power_matrix* power_lookup_;
		// On timer callback to update
		void(*on_timer_)();
		// Access spec for 
		string(*freq_to_band_)(double frequency);
		// Display message
		void(*error)(bool ok, const char* message);
		static void default_error_message(bool ok, const char* message);
		// bool
		bool have_freq_to_band_;


	};

	// THis class implements the hamlib specific methods of the base class
	// Hamlib is an standardised API on top of the divers implementations of rig CATs
	// It only allows one client application.
	class rig_hamlib : public rig_if
	{
	public:
		rig_hamlib();
		virtual ~rig_hamlib();

		// Opens the COM port associated with the rig
		virtual bool open();
		// Return rig name
		virtual string& rig_name();
		// Read TX Frequency
		virtual double tx_frequency();
		// Read mode from rig
		virtual rig_mode_t mode();
		// Return drive level * 100% power
		virtual double drive();
		// Rig is working split TX/RX frequency
		virtual bool is_split();
		// Get separate frequency
		virtual double rx_frequency();
		// Return S-meter reading (S9+/-dB)
		virtual int s_meter();
		// Return the most recent error message
		virtual string error_message();
		// Return raw message
		virtual string raw_message(string message);

		// Error Code is not OK.
		virtual bool is_good();
		// close rig - may be null for some 
		virtual void close();

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
	};

	// This class is the flrig specific implementation of the base class.
	// flrig is an applicatiion that provides an XML-RPI server interface over HTTP.
	// It allows multiple client accesses.
	class rig_flrig :
		public rig_if

	{
	public:
		rig_flrig();
		virtual ~rig_flrig();

		// Opens the COM port associated with the rig
		virtual bool open();
		// Return rig name
		virtual string& rig_name();
		// Read TX Frequency
		virtual double tx_frequency();
		// Read mode from rig
		virtual rig_mode_t mode();
		// Return drive level * 100% power
		virtual double drive();
		// Rig is working split TX/RX frequency
		virtual bool is_split();
		// Get separate frequency
		virtual double rx_frequency();
		// Return S-meter reading (S9+/-dB)
		virtual int s_meter();
		// Return the most recent error message
		virtual string error_message();
		// Return raw message
		virtual string raw_message(string message);

		// Error Code is not OK.
		virtual bool is_good();
		// close rig - may be null for some 
		virtual void close();



	protected:
		// Perform an XML-RPC request 
		bool do_request(string sMethodName, rpc_data_item::rpc_list* params, rpc_data_item* response);

		// The RPC reader/writer
		rpc_handler* rpc_handler_;
		// Error code
		unsigned long error_code_;

	};

}
#endif
