#include "rig_if.h"
#include "status.h"
#include "formats.h"
#include "menu.h"
#include "../zzalib/utils.h"
#include "band_view.h"
#include "scratchpad.h"
#include "spec_data.h"

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>

using namespace zzalog;

extern Fl_Preferences* settings_;
extern status* status_;
extern rig_if* rig_if_;
extern band_view* band_view_;
extern menu* menu_;
extern scratchpad* scratchpad_;
extern spec_data* spec_data_;

extern void remove_sub_window(Fl_Window* w);


///////////////////////////////////////////////////////////////////////////////////////
//    B A S E   C L A S S
//
///////////////////////////////////////////////////////////////////////////////////////
// Base class constructor
rig_if::rig_if()
{
	// Initialise
	rig_name_ = "";
	mfg_name_ = "";
	opened_ok_ = false;
	handler_ = "";
	error_message_ = "";
	// Power lookup table
	power_lookup_ = new power_matrix;
}

// Base class destructor
rig_if::~rig_if()
{
}

// Returns if the rig opened OK
bool rig_if::is_open() {
	return opened_ok_;
}

// Open rig - common aspects - to be called at the end of the implementation
bool rig_if::open() {
	// Add rig timer and timeout immediately to use the callback to update
	// the status
	if (opened_ok_) {
		Fl::add_timeout(0.0, cb_timer_rig);
		return true;
	}
	else {
		return false;
	}
}

// Common functionality on closing rig interface
void rig_if::close() {
	// Timeout must be removed before rig connection is closed otherwise it could fire while the connection is closing
	Fl::remove_timeout(cb_timer_rig);
}

// Convert s-meter reading into display format
string rig_if::get_smeter() {
	int smeter = s_meter();
	char text[100];
	// SMeter value is relative to S9
	// Smeter value 0 indicates S9,
	// 1 S-point is 6 dB
	if (smeter < -54) {
		// Below S0
		sprintf(text, " S0%dsB", 54 + smeter);
	}
	else if (smeter <= 0) {
		// Below S9 - convert to S points (6dB per S-point
		sprintf(text, " S%1d", (54 + smeter) / 6);
	}
	else {
		// S9+
		sprintf(text, " S9+%ddB", smeter);
	}

	return text;
}

// Returns the string displaying the rig information for the rig status
// e.g. Hamlib: TS-2000 14.123456 MHz 10 W USB S9+20
string rig_if::rig_info() {
	string rig_info = handler_ + ": ";
	rig_info += rig_name_;
	// Valid rig - get data from it. TX frequency
	rig_info += " " + get_tx_frequency() + " Mhz";
	rig_info += " " + get_tx_power() + " W";
	string mode;
	string submode;
	get_string_mode(mode, submode);
	if (submode.length()) {
		rig_info += " " + mode + "(" + submode + ")";
	}
	else {
		rig_info += " " + mode;
	}
	rig_info += " " + get_smeter();
	return rig_info;
}

// Return the transmit frequency
string rig_if::get_tx_frequency() {
	// get settings 
	Fl_Preferences log_settings(settings_, "Log");

	// Set the frquency display unit and precision
	frequency_t log_format;
	string format;
	log_settings.get("Frequency Precision", (int&)log_format, (int)FREQ_Hz);
	double frequency = tx_frequency() / 1000000.0;

	switch (log_format) {
	case FREQ_Hz:
		format = "%0.6f";
		break;
	case FREQ_Hz10:
		format = "%0.5f";
		break;
	case FREQ_Hz100:
		format = "%0.4f";
		break;
	case FREQ_kHz:
		format = "%0.3f";
		break;
	case FREQ_kHz10:
		format = "%0.2f";
		break;
	case FREQ_kHz100:
		format = "%0.1f";
		break;
	case FREQ_MHz:
		format = "%0.0f";
		break;
	default:
		format = "%0d";
	}
	// Format the text for the display
	char text[100];
	sprintf(text, format.c_str(), frequency);

	return text;

}

// Return the power
string rig_if::get_tx_power() {
	double tx_power = power();
	char text[100];
	sprintf(text, "%g", tx_power);

	return text;
}

// Convert the drive-level to a power
double rig_if::power() {
	double my_drive = drive();
	double my_freq = tx_frequency();
	// Get the band
	string band = spec_data_->band_for_freq(my_freq / 1000000);
	return power_lookup_->power(band, (int)(my_drive * 100));
}

// Rig timer callback
void rig_if::cb_timer_rig(void* v) {
	Fl::lock();
	// Update the label in the rig button 
	// Get polling intervals from settings
	Fl_Preferences rig_settings(settings_, "Rig");
	double timer_value = 0.0;
	// Set the status and get the polling interval for the current state of the rig
	if (rig_if_ == nullptr) {
		// Rig not set up - DLOW
		rig_settings.get("Slow Polling Interval", timer_value, SLOW_RIG_DEF);
	}
	else {
		if (rig_if_->is_good()) {
			// Rig connected and is working - FAST
			status_->rig_status(ST_OK, rig_if_->rig_info().c_str());
			rig_settings.get("Polling Interval", timer_value, FAST_RIG_DEF);
			// Band view may not have been created yet
			if (band_view_) {
				band_view_->update(rig_if_->tx_frequency() / 1000.0);
			}
			// Update scratchpad
			if (scratchpad_) {
				string mode;
				string submode;
				rig_if_->get_string_mode(mode, submode);
				string freq = rig_if_->get_tx_frequency();
				string power = rig_if_->get_tx_power();
				if (submode.length()) {
					scratchpad_->rig_update(freq, submode, power);
				}
				else {
					scratchpad_->rig_update(freq, mode, power);
				}
			}
		}
		if (!rig_if_->is_good()) {
			// Rig connected and broken - SLOW
			status_->rig_status(ST_ERROR, rig_if_->error_message().c_str());
			char message[100];
			sprintf(message, "RIG: Interface broken - %s", rig_if_->error_message().c_str());
			status_->misc_status(ST_ERROR, message);
			rig_settings.get("Slow Polling Interval", timer_value, SLOW_RIG_DEF);
			if (band_view_) {
				remove_sub_window(band_view_);
				delete band_view_;
				band_view_ = nullptr;
			}
		}
	}
	// repeat the timer
	Fl::repeat_timeout(timer_value, cb_timer_rig, v);
	Fl::unlock();
}

// Converts rig mode to string
void rig_if::get_string_mode(string& mode, string& submode) {
	rig_mode_t rig_mode = this->mode();
	mode = "";
	submode = "";
	switch (rig_mode) {
	case GM_INVALID:
	case GM_DIGL:
	case GM_DIGU:
		return;
	case GM_LSB:
		mode = "SSB";
		submode = "LSB";
		return;
	case GM_USB:
		mode = "SSB";
		submode = "USB";
		return;
	case GM_CWU:
	case GM_CWL:
		mode = "CW";
		return;
	case GM_FM:
		mode = "FM";
		return;
	case GM_AM:
		mode = "AM";
		return;
	default:
		return;
	}
}

string rig_if::success_message() {
	return success_message_;
}

///////////////////////////////////////////////////////////////////////////////////////
//    H a M L I B implementation
//
// hamlib is a library API compiled with ZZALOG and provides standard API connecting
// many different rig models using a serial port.(PowerSDR emulates TS2000)
// It has an issue that only a single app (logger or digital modem) may access the 
// rig at the same time
// This is gettable round using omnirig or flrig as the rig slecetd by hamlib 
// to muliplex accesses
///////////////////////////////////////////////////////////////////////////////////////
// Constructor
rig_hamlib::rig_hamlib() 
: rig_if()
{
	rig_ = nullptr;
	model_id_ = 0;
	port_name_ = "";
	model_id_ = 0;
	error_code_ = 0;
	error_message_ = "";
	baud_rate_ = 0;

	// Initialise the rig database in hamlib
	rig_load_all_backends();

	// set handler name
	handler_ = "Hamlib";

}

// Destructor
rig_hamlib::~rig_hamlib()
{
	// close the serial port
	close();
}

// Clsoe the serial port
void rig_hamlib::close() {
	// Call base class  for common behaviour
	rig_if::close();

	if (rig_ != nullptr) {
		if (opened_ok_) {
			// If we have a connection and it's open, close it and tidy memory used by hamlib
			rig_close(rig_);
			rig_cleanup(rig_);
		}
	}
	opened_ok_ = false;


}

// Opens the connection associated with the rig
bool rig_hamlib::open() {
	// If changing rig type - delete any existing rig and create new one
	if (rig_ != nullptr) {
		close();
	}

	// Read hamlib configuration
	Fl_Preferences rig_settings(settings_, "Rig");
	Fl_Preferences hamlib_settings(rig_settings, "Hamlib");
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");

	char* temp;
	rigs_settings.get("Current", temp, "");
	Fl_Preferences current_settings(hamlib_settings, temp);
	free(temp);
	current_settings.get("Rig Model", temp, "dummy");
	rig_name_ = temp;
	free(temp);
	current_settings.get("Manufacturer", temp, "Hamlib");
	mfg_name_ = temp;
	free(temp);
	current_settings.get("Port", temp, "COM6");
	port_name_ = temp;
	free(temp);
	current_settings.get("Baud Rate", baud_rate_, 9600);

	model_id_ = -1;
	for (rig_model_t i = 0; i < 4000 && model_id_ == -1; i++) {
		// Search through the rig database until we find the required rig.
		// Read each rig's capabilities
		const rig_caps* capabilities = rig_get_caps(i);
		try {
			if (capabilities != nullptr) {
				// Not all numbers represent a rig as the mapping is sparse
				// Check the model name
				if (capabilities->model_name == rig_name_ &&
					capabilities->mfg_name == mfg_name_) model_id_ = i;
			}
		}
		catch (exception*) {}
	}
	// Get the rig interface
	rig_ = rig_init(model_id_);
	if (rig_ != nullptr) {
		// Successful - set up the serial port parameters
		strcpy(rig_->state.rigport.pathname, port_name_.c_str());
		rig_->state.rigport.parm.serial.rate = baud_rate_;
	}

	if (rig_ != nullptr) {
		// open rig connection over serial port
		error_code_ = rig_open(rig_);

		if (error_code_ != RIG_OK) {
			// Not opened, tidy hamlib memory usage and mark it so.
			rig_cleanup(rig_);
			rig_ = nullptr;
			opened_ok_ = false;
		}
		else {
			// Opened OK
			opened_ok_ = true;
		}
	}
	else {
		// Rig did not connect
		opened_ok_ = false;
	}

	if (opened_ok_) {
		success_message_ = "RIG: hamlib connection " + mfg_name_ + "/" + rig_name_ + " on port " + port_name_ + " opened OK";
	}

	// Set timer
	return rig_if::open();
}

// Return rig name
string& rig_hamlib::rig_name() {
	// Read capabilities to get manufacturer and model name
	rig_caps capabilities = *(rig_get_caps(model_id_));
	rig_name_ = capabilities.model_name;
	mfg_name_ = capabilities.mfg_name;
	return rig_name_;
}

// Read TX frequency in Hz
double rig_hamlib::tx_frequency() {
	vfo_t vfo_id;
	freq_t frequency_hz;
	// Get ID of transmit VFO and read its value
	if ((error_code_ = rig_get_vfo(rig_, &vfo_id)) == RIG_OK) {
		error_code_ = rig_get_freq(rig_, vfo_id, &frequency_hz);
	}
	else {
		// Return not-a-number as wasn't read
		frequency_hz = nan("");

	}
	return frequency_hz;
}

// Read mode from rig
rig_mode_t rig_hamlib::mode() {
	rmode_t mode;
	shortfreq_t bandwidth;
	if ((error_code_ = rig_get_mode(rig_, RIG_VFO_CURR, &mode, &bandwidth)) == RIG_OK) {
		// Convert hamlib mode encoding to ZLG encoding
		if (mode & RIG_MODE_AM) {
			return GM_AM;
		}
		if (mode & RIG_MODE_CW) {
			return GM_CWU;
		}
		if (mode & RIG_MODE_CWR) {
			return GM_CWL;
		}
		if (mode & RIG_MODE_LSB) {
			return GM_LSB;
		}
		if (mode & RIG_MODE_USB) {
			return GM_USB;
		}
		if (mode & RIG_MODE_FM) {
			return GM_FM;
		}
		if (mode & RIG_MODE_RTTY) {
			return GM_DIGL;
		}
		if (mode & RIG_MODE_RTTYR) {
			return GM_DIGU;
		}
		else {
			// failed to decvode mode
			return GM_INVALID;
		}
	}
	else {
		// Failed to access rig
		return GM_INVALID;
	}
}

// Return drive level * 100% power
double rig_hamlib::drive() {
	value_t drive_level;
	if ((error_code_ = rig_get_level(rig_, RIG_VFO_CURR, rig_level_e::RIG_LEVEL_RFPOWER, &drive_level)) == RIG_OK) {
		return drive_level.f;
	}
	else {
		return nan("");
	}
}

// Rig is working split TX/RX frequency
bool rig_hamlib::is_split() {
	vfo_t TxVFO;
	split_t split;
	error_code_ = rig_get_split_vfo(rig_, RIG_VFO_CURR, &split, &TxVFO);
	return (split == split_t::RIG_SPLIT_ON);
}

// Get separate receive frequency
double rig_hamlib::rx_frequency() {
	freq_t frequency_hz;
	// VFO A is always receive VFO
	if ((error_code_ = rig_get_freq(rig_, RIG_VFO_A, &frequency_hz)) == RIG_OK) {
		return frequency_hz;
	}
	else {
		return nan("");
	}
}

// Return S-meter reading (relative to S9)
int rig_hamlib::s_meter() {
	value_t meter_value;
	if ((error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_STRENGTH, &meter_value)) == RIG_OK) {
		// hamlib returns relative to S9. 
		return meter_value.i;
	}
	else {
		// Default to S0 (S9 is -73 dBm)
		return -54;
	}
}

// Return the most recent error message
string rig_hamlib::error_message() {
	const char* hamlib_error = error_text((rig_errcode_e)abs(error_code_));
	if (hamlib_error == nullptr) {
		error_message_ = "Hamlib: (No error details)";
	}
	else {
		error_message_ = "Hamlib: " + string(hamlib_error);
	}
	return error_message_;
}

// Error Code is OK.
bool rig_hamlib::is_good() {
	return opened_ok_ && error_code_ == RIG_OK;
}

// Return the text for the error code.
const char* rig_hamlib::error_text(rig_errcode_e code) {
	switch (code) {
	case RIG_OK:
		return "No error, operation completed successfully";
	case RIG_EINVAL:
		return "invalid parameter";
	case RIG_ECONF:
		return "invalid configuration (serial,..)";
	case RIG_ENOMEM:
		return "memory shortage";
	case RIG_ENIMPL:
		return "function not implemented, but will be";
	case RIG_ETIMEOUT:
		return "communication timed out";
	case RIG_EIO:
		return "IO error, including open failed";
	case RIG_EINTERNAL:
		return "Internal Hamlib error, huh!";
	case RIG_EPROTO:
		return "Protocol error";
	case RIG_ERJCTED:
		return "Command rejected by the rig";
	case RIG_ETRUNC:
		return "Command performed, but arg truncated";
	case RIG_ENAVAIL:
		return "function not available";
	case RIG_ENTARGET:
		return "VFO not targetable";
	case RIG_BUSERROR:
		return "Error talking on the bus";
	case RIG_BUSBUSY:
		return "Collision on the bus";
	case RIG_EARG:
		return "NULL RIG handle or any invalid pointer parameter in get arg";
	case RIG_EVFO:
		return "Invalid VFO";
	case RIG_EDOM:
		return "Argument out of domain of func";
	default:
		return nullptr;
	}
};

#ifdef _WIN32
///////////////////////////////////////////////////////////////////////////////////////
//    O m n i R i g   implementation
// 
// Omnirig provides an API running in a separate process that provides sharable
// access to 1 of two rigs.
// Not all apps support Omnirig as it is a Windows-only application
///////////////////////////////////////////////////////////////////////////////////////

// Constructor
rig_omnirig::rig_omnirig()
	: rig_if()
	, omnirig_(nullptr)
	, rig_num_(0)
	, rig_(nullptr)
	, error_code_(OmniRig::RigStatusX::ST_NOTCONFIGURED)
	, waiting_drive_reply_(false)
	, waiting_smeter_reply_(false)
	, drive_level_(0)
	, signal_level_(0)
{
	// Get Omnirig rig number from configuration
	Fl_Preferences rig_settings(settings_, "Rig");
	Fl_Preferences omnirig_settings(rig_settings, "Omnirig");
	omnirig_settings.get("Rig Number", rig_num_, 1);
	handler_ = "Omnirig";
}


// Destructor
rig_omnirig::~rig_omnirig()
{
	// Tell the interface we can no longer support the callback
	IDispEventSimpleImpl<1, rig_omnirig, &__uuidof(OmniRig::IOmniRigXEvents)>::DispEventUnadvise(omnirig_);
}


// Open the rig
bool rig_omnirig::open()
{
	// Connect to openrig - start a new one if it's not
	try
	{
		// Connect to OmniRif if it is running
		HRESULT result = omnirig_.GetActiveObject("OmniRig.OmniRigX");
		if (FAILED(result))
			_com_issue_error(result);
	}
	catch (_com_error& /*e*/)
	{
		// Failed to connect to an existing copy of Omnirig so start a new instance
		HRESULT result = omnirig_.CreateInstance("OmniRig.OmniRigX", nullptr, CLSCTX_LOCAL_SERVER);
		if (FAILED(result))
		{
			// Connection failed
			// And if that fails, say oops 
			status_->misc_status(ST_SEVERE, "Unable to open Omnirig!");
			opened_ok_ = false;
		}
		else {
			opened_ok_ = true;
		}
	}
	if (opened_ok_) {
		// Tell the connection we can handle callbacks
		IDispEventSimpleImpl<1, rig_omnirig, &__uuidof(OmniRig::IOmniRigXEvents)>::DispEventAdvise(omnirig_);

		// Select on rig number
		switch (rig_num_) {
		case 1:
			rig_ = omnirig_->GetRig1();
			break;
		case 2:
			rig_ = omnirig_->GetRig2();
			break;
		}
		if (rig_ == nullptr) {
			// No rig connected to that port
			opened_ok_ = false;
		}
		else {
			// Check status of rig connection
			OmniRig::RigStatusX status;
			error_code_ = rig_->get_Status(&status);
			if (FAILED(error_code_) || status != OmniRig::RigStatusX::ST_ONLINE) {
				// failed to open 
				opened_ok_ = false;
			}
			else {
				// Get rig name - failed if empty string
				rig_name_ = rig_->GetRigType();
				if (rig_name_ == "") {
					opened_ok_ = false;
				}
			}
		}
	}

	if (opened_ok_) {
		success_message_ = "RIG: Omnirig connection to " + rig_name_ + " opened OK";
	}
	// base class open() starts timer, timer reports success or otherwise of each access
	return rig_if::open();
}

// Return the rig name
string& rig_omnirig::rig_name()
{
	return rig_name_;
}

/*  Get TX Frquency
Get TX VFO, then get read the VFO. Convert the read string to a double
Return 0.0 if either read fails
*/
double rig_omnirig::tx_frequency()
{
	long frequency_hz = 0;
	OmniRig::RigParamX params;
	// Get the VFO configuration
	error_code_ = rig_->get_Vfo(&params);
	if (is_good()) {
		// Get the approprate VFO - check TX is VFO A
		if ((params & OmniRig::RigParamX::PM_VFOA) ||
			(params & OmniRig::RigParamX::PM_VFOAA)) {
			error_code_ = rig_->get_FreqA(&frequency_hz);
		}
		else {
			error_code_ = rig_->get_FreqB(&frequency_hz);
		}
	}
	if (frequency_hz == 0) {
		// Failed to read frequency
		return nan("");
	}
	else {
		return (double)frequency_hz;
	}
}

/* Return mode from the rig
*/
rig_mode_t rig_omnirig::mode()
{
	OmniRig::RigParamX mode;
	// Get the mode
	error_code_ = rig_->get_Mode(&mode);
	if (is_good()) {
		// Convert from Omnirig to zlg enum type
		if (mode & OmniRig::RigParamX::PM_AM) {
			return GM_AM;
		}
		if (mode & OmniRig::RigParamX::PM_CW_U) {
			return GM_CWU;
		}
		if (mode & OmniRig::RigParamX::PM_CW_L) {
			return GM_CWL;
		}
		if (mode & OmniRig::RigParamX::PM_SSB_L) {
			return GM_LSB;
		}
		if (mode & OmniRig::RigParamX::PM_SSB_U) {
			return GM_USB;
		}
		if (mode & OmniRig::RigParamX::PM_FM) {
			return GM_FM;
		}
		if (mode & OmniRig::RigParamX::PM_DIG_L) {
			return GM_DIGL;
		}
		if (mode & OmniRig::RigParamX::PM_DIG_U) {
			return GM_DIGU;
		}
	}
	// Default is not a mode
	return GM_INVALID;
}

/* Return rig drive level scaled by multiplier
*/
double rig_omnirig::drive()
{
	// Not directly supported by OmniRig 
	// TODO - convert to file driven - but for now just assume PowerSDR
	if (rig_name_ == "PowerSDR") {
		// Send a custom command to Omnirig, the return data will come through a callback
		_variant_t command;
		_variant_t ReplyEnd;
		command = "ZZPC;";
		ReplyEnd = ";";
		waiting_drive_reply_ = false;
		// The callback does not work correctly - the VARIANT parameters are correct
		waiting_drive_reply_ = true;
		error_code_ = rig_->raw_SendCustomCommand(command, 3, ReplyEnd);
		// wait for omnirig to replay - let FL scheduler still get in
		OmniRig::RigStatusX status;
		while (waiting_drive_reply_ && rig_->get_Status(&status) == S_OK && status == OmniRig::ST_ONLINE) Fl::wait();
		// Return 
		return (double)drive_level_;
	}
	return 1.0;

}

// Return true if running split frequency
bool rig_omnirig::is_split()
{
	OmniRig::RigParamX RigParam;
	error_code_ = rig_->get_Split(&RigParam);
	if (is_good()) {
		return ((RigParam & OmniRig::RigParamX::PM_SPLITON) == OmniRig::RigParamX::PM_SPLITON);
	}
	else {
		// Default if the access failed
		return false;
	}
}

// Return the RX frequency_hz
double rig_omnirig::rx_frequency()
{
	long frequency_hz = 0;
	// RX is always VFO A
	error_code_ = rig_->get_FreqA(&frequency_hz);
	if (is_good()) {
		return (double)frequency_hz;
	}
	else {
		return nan("");
	}
}

// Return S-meter reading (S9+/-dB)
int rig_omnirig::s_meter()
{
	// Not directly supported by OmniRig 
	// TODO - convert to file driven - but for now just assume PowerSDR
	if (rig_name_ == "PowerSDR") {
		// Send a custom command, data will be return with a callback
		_variant_t command;
		_variant_t ReplyEnd;
		command = "ZZSM0;";
		ReplyEnd = ";";
		waiting_smeter_reply_ = true;
		error_code_ = rig_->raw_SendCustomCommand(command, 3, ReplyEnd);
		// wait for omnirig to replay - let FL scheduler still get in
		OmniRig::RigStatusX status;
		while (waiting_smeter_reply_ && rig_->get_Status(&status) == S_OK && status == OmniRig::ST_ONLINE) Fl::wait();
		// signal level: 0 = -140 dBm, 260 = -10 dBm, S9 = -73 dBm
		return signal_level_ / 2 - 140 + 73;
	}
	else {
		// Default to S0
		return -54;
	}
}

// Return the most recent error message
string rig_omnirig::error_message()
{
	BSTR status;
	// Test for rig connection first
	if (rig_ != nullptr) {
		// Read the status string
		error_code_ = rig_->get_StatusStr(&status);
		// Convert BSTR to C++ string
		char* temp = _com_util::ConvertBSTRToString(status);
		error_message_ = string(temp);
	}
	else {
		error_message_ = "Not connected";
	}
	return error_message_;
}

// Error Code is OK.
bool rig_omnirig::is_good()
{
	OmniRig::RigStatusX status;
	if (rig_->get_Status(&status) == S_OK && status == OmniRig::ST_ONLINE) {
		return (!FAILED(error_code_));
	}
	else {
		return false;
	}
}

// Call back for replies to custom commands
HRESULT rig_omnirig::cb_custom_reply(long rig_num, VARIANT command, VARIANT reply)
{
	string string_command = "";
	string string_reply = "";

	if (command.vt == (VT_ARRAY | VT_UI1)) {
		// Get original command that triggered the callback
		char * array;
		HRESULT result = SafeArrayAccessData(command.parray, (void**)&array);
		if (result == S_OK) {
			long bound;
			result = SafeArrayGetUBound(command.parray, 1, &bound);
			// Copy the command byte by byte.
			for (int pos = 0; result == S_OK && pos <= bound; pos++) {
				string_command += *(array + pos);
			}
		}
	}
	if (reply.vt == (VT_ARRAY | VT_UI1)) {
		// Get the reply
		char * array;
		HRESULT result = SafeArrayAccessData(reply.parray, (void**)&array);
		if (result == S_OK) {
			long bound;
			result = SafeArrayGetUBound(reply.parray, 1, &bound);
			// Copy the reply byte by byte
			for (int pos = 0; result == S_OK && pos <= bound; pos++) {
				string_reply += *(array + pos);
			}
		}
		// PowerSDR specific commands
		if (string_command == "ZZPC;") {
			// Reading power drive level - ZZPCnnnnnn;
			drive_level_ = stoi(string_reply.substr(4, 6), nullptr, 10);
			waiting_drive_reply_ = false;
		}
		else if (string_command == "ZZSM0;") {
			// Reading signal strength meter - ZZSM0nnnnnnn
			signal_level_ = stoi(string_reply.substr(5, 7), nullptr, 10);
			waiting_smeter_reply_ = false;
		}
		else {
			status_->misc_status(ST_ERROR, "OMNIRIG: Unexpected command reply received from OmniRig");
		}
	}
	return 0;
}

// Close rig
void rig_omnirig::close() {
	// Call base class  for common behaviour
	rig_if::close();
}

#endif //_WIN32

///////////////////////////////////////////////////////////////////////////////////////
//    F l R i g   implementation
// 
//  FLRig provides a shareable access to the rig and is supported by a number of 
// digital modems (FLDigi and WSJT-X). It uses XML-RPC to access the API.
///////////////////////////////////////////////////////////////////////////////////////
// Constructor
rig_flrig::rig_flrig()
{
	rig_name_ = "";
	error_message_ = "";
	handler_ = "Flrig";
	rpc_handler_ = nullptr;
}

// Destructor
rig_flrig::~rig_flrig()
{
	if (opened_ok_) {
		close();
		delete rpc_handler_;
	}
}

// Opens the RPC server associated with the rig
bool rig_flrig::open() {
	// Default host name and port number
	Fl_Preferences rig_settings(settings_, "Rig");
	Fl_Preferences flrig_settings(rig_settings, "Flrig");
	char *temp;
	// Default indicates server is on same computer as running ZZALOG
	flrig_settings.get("Host", temp, "127.0.0.1");
	string host_name = temp;
	free(temp);
	int port_num;
	// Default is same as hard-coded in Flrig
	flrig_settings.get("Port", port_num, 12345);
	// Default resource (hard-coded in flrig
	flrig_settings.get("Resource", temp, "/RPC2");
	string resource = temp;
	free(temp);
	rpc_handler_ = new rpc_handler(host_name, port_num, resource);
	// If we have no rig name then assume it didn't open successfully
	if (rpc_handler_ == nullptr || rig_name() == "") {
		// Close it and release RPC handler
		opened_ok_ = false;
		delete rpc_handler_;
		rpc_handler_ = nullptr;
	}
	else {
		opened_ok_ = true;
	}

	if (opened_ok_) {
		success_message_ = "RIG: FlRig connection " + host_name + ":" + to_string(port_num) + " opened OK";
	}
	// Base implementation sets timer
	return rig_if::open();
}

// Return rig name
string& rig_flrig::rig_name() {
	rpc_data_item response;
	if (do_request("rig.get_xcvr", nullptr, &response)) {
		rig_name_ = response.get_string();
	}
	else {
		// Failed: return empty string
		rig_name_ = "";
	}
	return rig_name_;
}

// Read TX Frequency
double rig_flrig::tx_frequency() {
	rpc_data_item response;
	if (do_request("rig.get_vfo", nullptr, &response)) {
		return response.get_double();
	}
	else {
		// Failed return NAN
		return nan("");
	}
}

// Read mode from rig
rig_mode_t rig_flrig::mode() {
	rpc_data_item response;
	if (do_request("rig.get_mode", nullptr, &response)) {
		// Convert to zlg data type
		string mode_id = response.get_string();
		if (mode_id == "AM") return GM_AM;
		else if (mode_id == "USB") return GM_USB;
		else if (mode_id == "LSB") return GM_LSB;
		else if (mode_id == "CW") return GM_CWU;
		else if (mode_id == "CW-R") return GM_CWL;
		else if (mode_id == "FM") return GM_FM;
		else if (mode_id == "FSK") return GM_DIGL;
		else if (mode_id == "FSK-R") return GM_DIGU;
		else 
			// Unsupported mode returned
			return GM_INVALID;
	}
	else {
		// Request failed
		return GM_INVALID;
	}
}

// Return drive level * 100% power
double rig_flrig::drive() {
	rpc_data_item response;
	if (do_request("rig.get_pwrmeter", nullptr, &response)) {
		return (double)response.get_int();
	}
	else {
		return nan("");
	}
}

// Rig is working split TX/RX frequency
bool rig_flrig::is_split() {
	// There is no flrig rpc to get at split.
	return false;
}

// Get separate frequency
double rig_flrig::rx_frequency() {
	// It is not possible to access the alternate VFO
	return tx_frequency();
}

// Return S-meter reading (S9+/-dB)
int rig_flrig::s_meter() {
	rpc_data_item response;
	if (do_request("rig.get_smeter", nullptr, &response)) {
		// It looks like 0 = S0 so convert to dB below S9
		return response.get_int() - 54;
	}
	else {
		// Default return S0
		return -54;
	}
}

// Return the most recent error message
string rig_flrig::error_message() {
	return error_message_;
}

// Error Code is not OK.
bool rig_flrig::is_good() {
	return opened_ok_;
}

// close the connection
void rig_flrig::close() {
	// Call base class  for common behaviour
	rig_if::close();
	opened_ok_ = false;
}

// Do the specified request using RPC handler
bool rig_flrig::do_request(string method_name, rpc_data_item::rpc_list* params, rpc_data_item* response) {
	if (!rpc_handler_->do_request(method_name, params, response)) {
		// Failed to make the request
		error_message_ = "Request failed";
		opened_ok_ = false;
		return false;
	}
	else {
		return true;
	}
}
