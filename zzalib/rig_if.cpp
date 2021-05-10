#include "rig_if.h"
#include "formats.h"
#include "../zzalib/utils.h"
#include "../zzalib/ic7300.h"

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>

using namespace zzalib;

extern Fl_Preferences* settings_;
rig_if* rig_if_;
extern ic7300* ic7300_;

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
	handler_t_ = RIG_NONE;
	error_message_ = "";
	// Default action
	on_timer_ = nullptr;
	freq_to_band_ = nullptr;
	error = default_error_message;
	have_freq_to_band_ = false;
	reported_hi_swr_ = false;
	// Make this a daft value
	last_tx_power_ = 0.0;
	last_tx_swr_ = 1.0;
	sigma_tx_power_ = 0.0;
	num_pwr_samples_ = 0;
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
		error(ST_ERROR, "RIG: Failed to open rig");
		return false;
	}
}

// Common functionality on closing rig interface - to be called before the overrides
void rig_if::close() {
	// Timeout must be removed before rig connection is closed otherwise it could fire while the connection is closing
	Fl::remove_timeout(cb_timer_rig);
}

// Convert s-meter reading into display format
string rig_if::get_smeter() {
	int smeter = s_meter();
	if (get_tx()) {
		// Remember SWR value for display
		smeter = last_rx_smeter_;
	}
	else {
		last_rx_smeter_ = smeter;
	}
	char text[100];
	// SMeter value is relative to S9
	// Smeter value 0 indicates S9,
	// 1 S-point is 6 dB
	if (smeter < -54) {
		// Below S0
		sprintf(text, " S0%ddB", 54 + smeter);
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

// Convert SWR meter into display formet
string rig_if::get_swr_meter() {
	double swr = swr_meter();
	if (!get_tx()) {
		// Remember last SWR for display and clear reported flag to report it next transmission
		swr = last_tx_swr_;
		reported_hi_swr_ = false;
	}
	else {
		last_tx_swr_ = swr;
	}
	char text[100];
	snprintf(text, 100, "1:%.1f", swr);
	return text;
}

// Returns the string displaying the rig information for the rig status
// e.g. Hamlib: TS-2000 14.123456 MHz 10 W USB S9+20
string rig_if::rig_info() {
	string rig_info = handler_ + ": ";
	rig_info += rig_name_;
	// Valid rig - get data from it. TX frequency
	if (is_split()) {
		if (is_good()) rig_info += " TX:" + get_frequency(true) + " MHz";
		if (is_good()) rig_info += " RX:" + get_frequency(false) + " MHz";
	}
	else {
		if (is_good()) rig_info += " " + get_frequency(true) + " MHz";
	}
	if (is_good()) rig_info += " " + get_tx_power() + " W";
	string mode;
	string submode;
	if (is_good()) {
		get_string_mode(mode, submode);
		if (submode.length()) {
			rig_info += " " + mode + "(" + submode + ")";
		}
		else {
			rig_info += " " + mode;
		}
	}
	if (is_good()) rig_info += " " + get_smeter();
	if (is_good()) rig_info += " " + get_swr_meter();
	return rig_info;
}

// Return the transmit or receive frequency
string rig_if::get_frequency(bool tx) {
	// Read the frequency (in MHz)
	double frequency;
	if (tx) {
		frequency = tx_frequency() / 1000000.0;
	}
	else {
		frequency = rx_frequency() / 1000000.0;
	}
	char text[15];
	sprintf(text, "%0.6f", frequency);

	return text;

}

// Return the power
string rig_if::get_tx_power() {
	double tx_power = pwr_meter();
	if (tx_power == 0.0) {
		// If metered power is zero use the last actual value read
		// May need to tweak this to be less than a small number
		tx_power = last_tx_power_;
		if (num_pwr_samples_) {
			check_power();
		}
		sigma_tx_power_ = 0.0;
		num_pwr_samples_ = 0;
	}
	else {
		// Otherwise remember this value as the last value read
		last_tx_power_ = tx_power;
		num_pwr_samples_++;
		sigma_tx_power_ += tx_power;
	}
	char text[100];
	sprintf(text, "%g", tx_power);

	return text;
}

// Rig timer callback
void rig_if::cb_timer_rig(void* v) {
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
			rig_settings.get("Polling Interval", timer_value, FAST_RIG_DEF);
			if (rig_if_->on_timer_) {
				rig_if_->on_timer_();
			}
		}
		if (!rig_if_ || !rig_if_->is_good()) {
			// Rig connected and broken - SLOW
			rig_settings.get("Slow Polling Interval", timer_value, SLOW_RIG_DEF);
		}
	}
	// repeat the timer
	Fl::repeat_timeout(timer_value, cb_timer_rig, v);
}

// Converts rig mode to string
void rig_if::get_string_mode(string& mode, string& submode) {
	rig_mode_t rig_mode = this->mode();
	mode = "";
	submode = "";
	switch (rig_mode) {
	case GM_INVALID:
		error(ST_ERROR, "RIG: Invalid mode got from rig");
		return;
	case GM_DIGL:
		mode = "DATA L";
		return;
	case GM_DIGU:
		mode = "DATA U";
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

// Return any success message
string rig_if::success_message() {
	return success_message_;
}

// Set callbacks - 
// on_timer - callback on rig timer
// freq_to_band - callback to convert frequency to band
// error - callback for outputing error message
void rig_if::callback(void(*function)(), string(*spec_func)(double), void(*mess_func)(status_t, const char*)) {
	on_timer_ = function;
	freq_to_band_ = spec_func;
	error = mess_func;
	if (spec_func) {
		have_freq_to_band_ = true;
	}
	else {
		have_freq_to_band_ = false;
	}
}

void rig_if::update_clock() {
	// Only implemented for IC-7300
	if (rig_name() == "IC-7300" && ic7300_ && handler_t_ == RIG_FLRIG) {
		char message[200];
		// Get current time
		time_t now = time(nullptr);
		// convert to struct in UTC
		tm* figures = gmtime(&now);
		// And repeat until seconds reads 00
		snprintf(message, 200, "RIG: Waiting %d seconds to update clock", 60 - figures->tm_sec);
		error(ST_NOTE, message);
		while (figures->tm_sec) {
			now = time(nullptr);
			figures = gmtime(&now);
			Fl::wait();
		}
		error(ST_OK, "RIG: Updating clock");
		// Convert date and time to integers. 20200317, 1514
		int date = (figures->tm_year + 1900) * 10000 + (figures->tm_mon + 1) * 100 + figures->tm_mday;
		int time = figures->tm_hour * 100 + figures->tm_min;
		// Generate set date command x1A x05 x00 x94 date in BCD
		string data = int_to_bcd(date, 4, false);
		char command = '\x1a';
		bool ok;
		string sub_command = "   ";  
		sub_command[0] = '\x05';
		sub_command[1] = '\x00';
		sub_command[2] = '\x94';
		ic7300_->send_command(command, sub_command, data, ok);
		if (ok) {
			// Set time - x1A x05 x00 x95 time in BCD
			data = int_to_bcd(time, 2, false);
			sub_command[2] = '\x95';
			ic7300_->send_command(command, sub_command, data, ok);
		}
		if (ok) {
			// Set UTC off-set - UTC + 0000 - x1A x05 x00 x96 x00 x00 x00 (TZ=UTC)
			data = int_to_bcd(0, 3, false);
			sub_command[2] = '\x96';
			ic7300_->send_command(command, sub_command, data, ok);
		}
		if (ok) {
			snprintf(message, 200, "RIG: Updated rig clock to %04d %08d", time, date);
			error(ST_OK, message);
		}
		else {
			error(ST_ERROR, "RIG: Failed to update rig clock");
		}
	}
}

// Check SWR value - error if > error level, warning if > warning level (got from settings)
// Only check once per transmission period
bool  rig_if::check_swr() {
	Fl_Preferences rig_settings(settings_, "Rig");
	double warn_level;
	double error_level;
	// SWR Settings
	rig_settings.get("SWR Warning Level", warn_level, 1.5);
	rig_settings.get("SWR Error Level", error_level, 2.0);
	double swr = swr_meter();
	if (swr > error_level && !reported_hi_swr_) {
		char message[200];
		snprintf(message, 200, "RIG: SWR is %.1f", swr);
		error(ST_ERROR, message);
		reported_hi_swr_ = true;
		return false;
	}
	else if (swr > warn_level) {
		char message[200];
		snprintf(message, 200, "RIG: SWR is %.1f", swr);
		error(ST_WARNING, message);
		// Clear reported flag when we go back to receive
		return true;
	}
	return true;
}

// Check power level - only when TX ended
bool rig_if::check_power() {
	Fl_Preferences rig_settings_(settings_, "Rig");
	double power_warn_level;
	rig_settings_.get("Power Warning Level", power_warn_level, 50.0);
	if (num_pwr_samples_ > 0) {
		double power = sigma_tx_power_ / num_pwr_samples_;
		if (power > power_warn_level) {
			char message[200];
			snprintf(message, 200, "RIG: Average power was %.0f", power);
			error(ST_NOTIFY, message);
			return false;

		}
		else {
			return true;
		}
	}
	else {
		return true;
	}
}


///////////////////////////////////////////////////////////////////////////////////////
//    H a M L I B implementation
//
// hamlib is a library API compiled with zzalib and provides standard API connecting
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


	// set handler name
	handler_ = "Hamlib";
	handler_t_ = RIG_HAMLIB;

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

	// Read hamlib configuration - manufacturer,  model, port and baud-rate
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

	// Search through the rig database until we find the required rig.
	model_id_ = -1;
	const rig_caps* capabilities = nullptr;
	for (rig_model_t i = 0; i < 4000 && model_id_ == -1; i++) {
		// Read each rig's capabilities
		capabilities = rig_get_caps(i);
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
	if (rig_ != nullptr && capabilities->port_type == RIG_PORT_SERIAL) {
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
		if (capabilities->port_type == RIG_PORT_SERIAL) {
			success_message_ = "hamlib connection " + mfg_name_ + "/" + rig_name_ + " on port " + port_name_ + " opened OK";
		}
		else {
			success_message_ = "hamlib connection " + mfg_name_ + "/" + rig_name_ + " opened OK";
		}
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
	freq_t frequency_hz;
	// Get ID of transmit VFO and read its value
	if ((error_code_ = rig_get_freq(rig_, RIG_VFO_TX, &frequency_hz)) == RIG_OK) {
		return frequency_hz;
	}
	else {
		error(ST_ERROR, error_message("tx_frequency").c_str());
		return nan("");
	}
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
		if (mode & (RIG_MODE_RTTY | RIG_MODE_PKTLSB)) {
			return GM_DIGL;
		}
		if (mode & (RIG_MODE_RTTYR | RIG_MODE_PKTUSB)) {
			return GM_DIGU;
		}
		else {
			// failed to decvode mode
			return GM_INVALID;
		}
	}
	else {
		// Failed to access rig
		error(ST_ERROR, error_message("mode").c_str());
		return GM_INVALID;
	}
}

// Return drive level * 100% power
double rig_hamlib::drive() {
	value_t drive_level;
	if ((error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, &drive_level)) == RIG_OK) {
		return drive_level.f * 100;
	}
	else {
		error(ST_ERROR, error_message("drive").c_str());
		return nan("");
	}
}

// Rig is working split TX/RX frequency
bool rig_hamlib::is_split() {
	vfo_t TxVFO;
	split_t split;
	if ((error_code_ = rig_get_split_vfo(rig_, RIG_VFO_CURR, &split, &TxVFO)) == RIG_OK) {
		return (split == split_t::RIG_SPLIT_ON);
	}
	else {
		error(ST_ERROR, error_message("is_split").c_str());
		return false;
	}
}

// Get separate receive frequency
double rig_hamlib::rx_frequency() {
	freq_t frequency_hz;
	// VFO A is always receive VFO
	if ((error_code_ = rig_get_freq(rig_, RIG_VFO_CURR, &frequency_hz)) == RIG_OK) {
		return frequency_hz;
	}
	else {
		error(ST_ERROR, error_message("rx_frequency").c_str());
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
		error(ST_ERROR, error_message("s_meter").c_str());
		return -54;
	}
}

// Return power meter reading (Watts)
double rig_hamlib::pwr_meter() {
	value_t meter_value;
	if ((error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER_WATTS, &meter_value)) == RIG_OK) {
		// hamlib returns relative to S9. 
		return meter_value.f;
	}
	else {
		// Default to S0 (S9 is -73 dBm)
		error(ST_ERROR, error_message("pwr_meter").c_str());
		return nan("");
	}
}

// Return SWR value
double rig_hamlib::swr_meter() {
	value_t meter_value;
	if ((error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_SWR, &meter_value)) == RIG_OK) {
		return meter_value.f;
	}
	else {
		error(ST_ERROR, error_message("swr_meter").c_str());
		return nan("");
	}
}

// Return the most recent error message
string rig_hamlib::error_message(string func_name) {
	const char* hamlib_error = error_text((rig_errcode_e)abs(error_code_));
	if (hamlib_error == nullptr) {
		error_message_ = "RIG: Hamlib: " + func_name + " (No error details)";
	}
	else {
		error_message_ = "RIG: Hamlib: " + func_name + " " + string(hamlib_error);
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
		return "Error not defined";
	}
};

// Raw message - not implemented
string rig_hamlib::raw_message(string message) {
	error(ST_ERROR, "RIG: Hamlib does not support sending raw messages");
	return "";
};

// Transmit mode
bool rig_hamlib::get_tx() {
	// Get ID of transmit VFO and read its value
	ptt_t ptt;
	error_code_ = rig_get_ptt(rig_, RIG_VFO_CURR, &ptt);
	return (ptt != RIG_PTT_OFF);
}


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
	handler_t_ = RIG_FLRIG;
	rpc_handler_ = nullptr;
	error_ = true;
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
	// Default indicates server is on same computer as running zzalib
	flrig_settings.get("Host", temp, "127.0.0.1");
	string host_name = temp;
	free(temp);
	int port_num;
	// Default is same as hard-coded in Flrig
	flrig_settings.get("Port", port_num, 12345);
	// Default resource (hard-coded in flrig)
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
		error_ = false;
	}

	if (opened_ok_) {
		success_message_ = "FlRig connection " + host_name + ":" + to_string(port_num) + " opened OK";
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
		error(ST_ERROR, error_message("rig_name").c_str());
		error_ = true;
		rig_name_ = "";
	}
	return rig_name_;
}

// Read TX Frequency
double rig_flrig::rx_frequency() {
	rpc_data_item response;
	if (do_request("rig.get_vfoA", nullptr, &response)) {
		return response.get_double();
	}
	else {
		// Failed return NAN
		error(ST_ERROR, error_message("rx_frequency").c_str());
		error_ = true;
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
		else if (mode_id == "FSK" || mode_id == "LSB-D") return GM_DIGL;
		else if (mode_id == "FSK-R" || mode_id == "USB-D") return GM_DIGU;
		else 
			// Unsupported mode returned
			return GM_INVALID;
	}
	else {
		// Request failed
		error(ST_ERROR, error_message("mode").c_str());
		error_ = true;
		return GM_INVALID;
	}
}

// Return drive level
double rig_flrig::drive() {
	rpc_data_item response;
	if (do_request("rig.get_power", nullptr, &response)) {
		return (double)response.get_int();
	}
	else {
		error(ST_ERROR, error_message("drive").c_str());
		error_ = true;
		return nan("");
	}
}

// Rig is working split TX/RX frequency
bool rig_flrig::is_split() {
	rpc_data_item response;
	if (do_request("rig.get_split", nullptr, &response)) {
		return (bool)response.get_int();
	}
	else {
		error(ST_ERROR, error_message("is_split").c_str());
		error_ = true;
		return false;
	}
}

// Get separate frequency
double rig_flrig::tx_frequency() {
	rpc_data_item response;
	if (is_split()) {
		if (do_request("rig.get_AB", nullptr, &response) && response.get_int() == 0) {
			// return VFO B 
			if (do_request("rig.get_vfoB", nullptr, &response)) {
				return (double)response.get_int();
			}
			else {
				error(ST_ERROR, error_message("tx_frequency").c_str());
				error_ = true;
				return nan("");
			}
		}
	} 
	if (is_good()) {
		// Return VFO A if either not split or VFO B is the TX frequency
		if (do_request("rig.get_vfoA", nullptr, &response)) {
			return (double)response.get_int();
		}
		else {
			error(ST_ERROR, error_message("tx_frequency").c_str());
			error_ = true;
			return nan("");
		}
	}
	return nan("");
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
		error(ST_ERROR, error_message("s_meter").c_str());
		error_ = true;
		return -54;
	}
}

// Return power-meter reading (S9+/-dB)
double rig_flrig::pwr_meter() {
	rpc_data_item response;
	if (do_request("rig.get_pwrmeter", nullptr, &response)) {
		// It looks like 0 = S0 so convert to dB below S9
		return (double)response.get_int();
	}
	else {
		// Default return S0
		error(ST_ERROR, error_message("pwr_meter").c_str());
		error_ = true;
		return nan("");
	}
}

// Return SWR meter reading
double rig_flrig::swr_meter() {
	//if (ic7300_) {
	//	bool ok;
	//	char command = '\x15';
	//	string subcommand = "\x12";
	//	string data = ic7300_->send_command(command, subcommand, ok);
	//	if (ok) {
	//		char mess[256];
	//		if (data.length() >= 4) {
	//			unsigned int value = bcd_to_int(data.substr(2, 2), false);
	//			// reflection coefficient (ρ) - assume this is linear wrt value - supplied values infer it is
	//			// 0: SWR = 1 (ρ = 0), 48: SWR = 1.5 (ρ = 0.2), 80: SWR = 2 (ρ = 0.33), 120: SWR = 3 (ρ = 0.5)
	//			// implies value = 240 / ρ.
	//			if (value > 240) {
	//				value = 240;
	//			}
	//			double rho = value / 240.0;
	//			// SWR = (1 + ρ) / (1 - ρ) 
	//			double swr = (1.0 + rho) / (1 - rho);
	//			//			snprintf(mess, 256, "DEBUG: Value received from rig - %d, SWR = %g", value, swr);
	//			//			error(ST_LOG, mess);
	//			return swr;
	//		}
	//		else {
	//			snprintf(mess, 256, "RIG: Insufficient data received from transceiver - %d bytes", data.length());
	//			error(ST_ERROR, mess);
	//		}
	//	}
	//}
	//// Not IC7300 or not OK - return nan.
	//error(ST_ERROR, error_message("swr_meter").c_str());
	//return nan("");
	rpc_data_item response;
	if (do_request("rig.get_swrmeter", nullptr, &response)) {
		// It matches the SWR meter display widget in flrig
		// 0 = 1:1
		// 12(.5) = 1:1.5
		// 25 = 1:2
		// 50 = 1:3
		// 100 = 1:∞
		unsigned int value = response.get_int();
		double rho;
		// Attempt to unpick flrig's tweaking - see above
		// But keeps the linearity between these points in reflection coefficient
		if (value >= 50) {
			// Between 50 and 100 - treat it as %-age reflected power
			rho = value / 100.0;
		}
		else if (value >= 25) {
			// rho = 1/3 + interpolate between 0.33 and 0.5 (1/6)
			rho = (1.0 / 3.0) + ((1.0 / 6.0) * ((double)value - 25.0) / 25.0);
		}
		else if (value >= 13) {
			// rho = 0.2 + interpolate between 0.2 and 0.33 (2/15)
			rho = (0.2) + ((2.0 / 15.0) * ((double)value - 12.5) / 12.5);
		}
		else {
			// rho = interpolate between 0 and 0.2
			rho = (0.2 * (double)value / 12.5);
		}
		// SWR = (1 + ρ) / (1 - ρ) 
		double swr = (1.0 + rho) / (1.0 - rho);
		//char mess[256];
		//snprintf(mess, 256, "DEBUG: Value received from rig - %d, ρ = %g, SWR = %g", value, rho,  swr);
		//error(ST_DEBUG, mess);
		return swr;
	}
	else {
		// Default return S0
		error(ST_ERROR, error_message("swr_meter").c_str());
		error_ = true;
		return nan("");
	}

}

// Return the most recent error message
string rig_flrig::error_message(string func_name) {
	return "RIG: FlRig: " + func_name + " " + error_message_;
}

// Error Code is not OK.
bool rig_flrig::is_good() {
	return opened_ok_ && !error_;
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
		error_ = true;
		return false;
	}
	else {
		return true;
	}
}

// Do the raw message
string rig_flrig::raw_message(string message) {
	rpc_data_item param;
	rpc_data_item::rpc_list params;
	rpc_data_item response;
	param.set(message, XRT_STRING);
	params.clear();
	params.push_back(&param);
	if (do_request("rig.cat_priority", &params, &response)) {
		if (response.get_string() == "OK") {
			if (do_request("rig.cat_string", &params, &response)) {
				return response.get_string();
			}
		}
		error_message_ = "Raw message failed to execute command";
		return "";
	}
	else {
		error_message_ = "Raw message failed to get priority";
		return "";
	}
}

// Get PTT status
bool rig_flrig::get_tx() {
	rpc_data_item response;
	if (do_request("rig.get_ptt", nullptr, &response)) {
		return response.get_int() != 0;
	}
	else {
		error(ST_ERROR, error_message("get_tx").c_str());
		error_ = true;
		return false;
	}
}
