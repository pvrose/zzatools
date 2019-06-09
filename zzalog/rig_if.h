#ifndef __RIG_IF__
#define __RIG_IF__

// zzalog includes

#include "rpc_data_item.h"
#include "rpc_handler.h"
// hamlib icludes
#include "hamlib/rig.h"

// C/C++ includes
#include <string>

#ifdef _WIN32
//!! OmniRIG type library
#import "C:\Program Files (x86)\Afreet\OmniRig\OmniRig.exe"
#include <atlbase.h>
#include <atlcom.h>
#endif

using namespace std;

namespace zzalog {

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
		RIG_OMNIRIG,
		RIG_HAMLIB,
		RIG_FLRIG,
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
		// Return drive level * 100% power
		virtual double power_drive() = 0;
		// Rig is working split TX/RX frequency
		virtual bool is_split() = 0;
		// Get separate frequency
		virtual double rx_frequency() = 0;
		// Return S-meter reading (S9+/-dB)
		virtual int s_meter() = 0;
		// Return the most recent error message
		virtual string error_message() = 0;

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
		// Return open message
		string success_message();


		// Protected methods
	protected:
		// convert frequency to text according to settings
		string display_frequency(double frequency);
		// convert power to text according to settings
		string display_power(double power);
		// convert mode to display form
		string display_mode(rig_mode_t mode);
		// convert s-meter reading to text according to settings
		string display_smeter(int smeter);

		// Protected attributes
	protected:
		// Name of the rig
		string rig_name_;
		// Name of the rig (prepended with manufacturer)
		string mfg_name_;
		// Power (W) when drive level is 100%
		double power_at_100_;
		// Rig opened OK
		bool opened_ok_;
		// Handler name
		string handler_;
		// Text error message
		string error_message_;
		// Text success message
		string success_message_;

	};

	// THis class implements the hamlib specific methods of the base class
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
		virtual double power_drive();
		// Rig is working split TX/RX frequency
		virtual bool is_split();
		// Get separate frequency
		virtual double rx_frequency();
		// Return S-meter reading (S9+/-dB)
		virtual int s_meter();
		// Return the most recent error message
		virtual string error_message();

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

	// Omnirig is a windows-only application
#ifdef _WIN32

// This class implements the Omnirig specific implementation of the base class
	class rig_omnirig :
		public rig_if
		, public ::IDispEventSimpleImpl<1, rig_omnirig, &__uuidof(::OmniRig::IOmniRigXEvents)>

	{
	public:
		rig_omnirig();
		virtual ~rig_omnirig();

		// Opens the COM port associated with the rig
		virtual bool open();
		// Return rig name
		virtual string& rig_name();
		// Read TX Frequency
		virtual double tx_frequency();
		// Read mode from rig
		virtual rig_mode_t mode();
		// Return drive level * 100% power
		virtual double power_drive();
		// Rig is working split TX/RX frequency
		virtual bool is_split();
		// Get separate frequency
		virtual double rx_frequency();
		// Return S-meter reading (S9+/-dB)
		virtual int s_meter();
		// Return the most recent error message
		virtual string error_message();

		// Error Code is not OK.
		virtual bool is_good();
		// close rig - may be null for some 
		virtual void close();

		// Asynchronous callback from Omnirig
		HRESULT __stdcall cb_custom_reply(long rig_number, VARIANT command, VARIANT reply);
		// Add the callback interface from Omnirig to ZZALOG
		BEGIN_SINK_MAP(rig_omnirig)
			SINK_ENTRY_INFO_P(1, &__uuidof(::OmniRig::IOmniRigXEvents), 0x5, &rig_omnirig::cb_custom_reply, new _ATL_FUNC_INFO({ CC_STDCALL, VT_I4, 3,{ VT_I4, VT_VARIANT, VT_VARIANT } }))
		END_SINK_MAP()

	protected:

		// The Omnirig interfaces
		::OmniRig::IOmniRigXPtr omnirig_;
		// Rig
		::OmniRig::IRigXPtr rig_;
		// One of 1 or 2 rigs that Omnirig can simulatneously connect to
		int rig_num_;
		// Waiting for a reply for drive level
		bool waiting_drive_reply_;
		// Waiting for a reply for signal strength
		bool waiting_smeter_reply_;
		// Drive level
		int drive_level_;
		// Signal level
		int signal_level_;
		// Most recent error code received
		HRESULT error_code_;

	};

#endif


	// This class is the flrig specific implementation of the base class
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
		virtual double power_drive();
		// Rig is working split TX/RX frequency
		virtual bool is_split();
		// Get separate frequency
		virtual double rx_frequency();
		// Return S-meter reading (S9+/-dB)
		virtual int s_meter();
		// Return the most recent error message
		virtual string error_message();

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
