#include "rig_if.h"
#include "formats.h"
#include "utils.h"

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>



extern Fl_Preferences* settings_;
rig_if* rig_if_ = nullptr;

// Returns if the rig opened OK
bool rig_if::is_open() {
	return opened_ok_;
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
		snprintf(text, 100, " S0%ddB", 54 + smeter);
	}
	else if (smeter <= 0) {
		// Below S9 - convert to S points (6dB per S-point
		snprintf(text, 100, " S%1d", (54 + smeter) / 6);
	}
	else {
		// S9+
		snprintf(text, 100, " S9+%ddB", smeter);
	}

	return text;
}

// Convert SWR meter into display formet
string rig_if::get_swr_meter() {
	double swr = swr_meter();
	char text[100];
	snprintf(text, 100, "1:%.1f", swr);
	return text;
}

// Returns the string displaying the rig information for the rig status
// e.g. Hamlib: TS-2000 14.123456 MHz 10 W USB S9+20
string rig_if::rig_info() {
	string rig_info = rig_info += rig_name_;
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
	//if (is_good()) rig_info += " " + get_swr_meter();
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
	snprintf(text, 15, "%0.6f", frequency);

	return text;

}

// Return the power
string rig_if::get_tx_power() {
	double tx_power = pwr_meter();
	char text[100];
	snprintf(text, 100, "%g", tx_power);
	return text;
}

// Rig timer callback
void rig_if::cb_timer_rig(void* v) {
	// Get polling intervals from settings
	double timer_value = 0.0;
	Fl_Preferences cat_settings(settings_, "CAT");
	// Set the status and get the polling interval for the current state of the rig
	if (rig_if_ == nullptr) {
		// Rig not set up - DLOW
		cat_settings.get("Slow Polling Interval", timer_value, SLOW_RIG_DEF);
	}
	else {
		if (rig_if_->is_good()) {
			// Rig connected and is working - FAST
			cat_settings.get("Polling Interval", timer_value, FAST_RIG_DEF);
			if (rig_if_->on_timer_) {
				rig_if_->on_timer_();
			}
		}
		if (!rig_if_ || !rig_if_->is_good()) {
			// Rig connected and broken - SLOW
			cat_settings.get("Slow Polling Interval", timer_value, SLOW_RIG_DEF);
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
rig_if::rig_if() 
{
	// Initialise
	rig_name_ = "";
	mfg_name_ = "";
	opened_ok_ = false;
	error_message_ = "";
	// Default action
	on_timer_ = nullptr;
	freq_to_band_ = nullptr;
	error = default_error_message;
	have_freq_to_band_ = false;
	reported_hi_swr_ = false;
	inhibit_repeated_errors = false;
	rig_ = nullptr;
	model_id_ = 0;
	port_name_ = "";
	model_id_ = 0;
	error_code_ = 0;
	error_message_ = "";
	baud_rate_ = 0;
	unsupported_function_ = false;
}

// Destructor
rig_if::~rig_if()
{
	// close the serial port
	close();
}

// Clsoe the serial port
void rig_if::close() {

	// Timeout must be removed before rig connection is closed otherwise it could fire while the connection is closing
	Fl::remove_timeout(cb_timer_rig);

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
bool rig_if::open() {
	// If changing rig type - delete any existing rig and create new one
	if (rig_ != nullptr) {
		close();
	}

	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	Fl_Preferences cat_settings(settings_, "CAT");
	Fl_Preferences hamlib_settings(cat_settings, "Hamlib");
	char* temp;
	hamlib_settings.get("Rig Model", temp, "dummy");
	rig_name_ = temp;
	free(temp);
	hamlib_settings.get("Manufacturer", temp, "Hamlib");
	mfg_name_ = temp;
	free(temp);
	hamlib_settings.get("Port", temp, "COM6");
	port_name_ = temp;
	free(temp);
	hamlib_settings.get("Baud Rate", baud_rate_, 9600);

	// Search through the rig database until we find the required rig.
	model_id_ = -1;
	const rig_caps* capabilities = nullptr;
	rig_model_t max_rig_num = 40 * MAX_MODELS_PER_BACKEND;
	for (rig_model_t i = 0; i < max_rig_num && model_id_ == -1; i++) {
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
	if (model_id_ == -1) {
		// Rig not found in hamlib
		error_message_ = "RIG: " + rig_name_ + " not found in capabilities";
		rig_ = nullptr;
	}
	else {
		// Get the rig interface
		rig_ = rig_init(model_id_);
		if (rig_ != nullptr) {
			switch (capabilities->port_type) {
			case RIG_PORT_SERIAL:
				// Successful - set up the serial port parameters
				strcpy(rig_->state.rigport.pathname, port_name_.c_str());
				rig_->state.rigport.parm.serial.rate = baud_rate_;
				break;
			case RIG_PORT_NETWORK:
			case RIG_PORT_USB:
				printf("RIG: Setting port path=%s\n", port_name_.c_str());
				int err = rig_set_conf(rig_, rig_token_lookup(rig_, "rig_pathname"), port_name_.c_str());
				//strcpy(rig_->state.rigport.pathname, port_name_.c_str());
				break;
			}
		}
	}

	if (rig_ != nullptr) {
		// open rig connection over serial port
		printf("RIG: Opening rig\n");
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
			success_message_ = "Connection " + mfg_name_ + "/" + rig_name_ + " on port " + port_name_ + " opened OK";
		}
		else {
			success_message_ = "Connection " + mfg_name_ + "/" + rig_name_ + " on port " + port_name_ + 
				" (" + rig_get_info(rig_) + ") opened OK";
		}
	}

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

// Return rig name
string& rig_if::rig_name() {
	// Read capabilities to get manufacturer and model name
	full_rig_name_ = rig_get_info(rig_);
	rig_caps capabilities = *(rig_get_caps(model_id_));
	rig_name_ = capabilities.model_name;
	mfg_name_ = capabilities.mfg_name;
	return full_rig_name_;
}

// Read TX frequency in Hz
double rig_if::tx_frequency() {
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
rig_mode_t rig_if::mode() {
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
double rig_if::drive() {
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
bool rig_if::is_split() {
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
double rig_if::rx_frequency() {
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
int rig_if::s_meter() {
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
double rig_if::pwr_meter() {
	value_t meter_value;
	if ((error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER_WATTS, &meter_value)) == RIG_OK) {
		// hamlib returns relative to S9. 
		return meter_value.f;
	}
	else {
		// Default to nan
		error(ST_ERROR, error_message("pwr_meter").c_str());
		return nan("");
	}
}

// Return Vdd meter reading (Volts)
double rig_if::vdd_meter() {
	value_t meter_value;
	if ((error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_VD_METER, &meter_value)) == RIG_OK) {
		return meter_value.f;
	}
	else {
		if (abs(error_code_) != RIG_ENAVAIL || !unsupported_function_) {
			error(ST_ERROR, error_message("vdd_meter").c_str());
			if (abs(error_code_) == RIG_ENAVAIL) {
				unsupported_function_ = true;
			}
		}
		return nan("");
	}
}

// Return SWR value
double rig_if::swr_meter() {
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
string rig_if::error_message(string func_name) {
	const char* hamlib_error = error_text((rig_errcode_e)abs(error_code_));
	if (hamlib_error == nullptr) {
		error_message_ = "RIG: " + func_name + " (No error details)";
	}
	else {
		error_message_ = "RIG: " + func_name + " " + string(hamlib_error);
	}
	return error_message_;
}

// Error Code is OK.
bool rig_if::is_good() {
	return opened_ok_ && error_code_ == RIG_OK;
}

// Return the text for the error code.
const char* rig_if::error_text(rig_errcode_e code) {
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

// Transmit mode
bool rig_if::get_tx() {
	// Get ID of transmit VFO and read its value
	ptt_t ptt;
	error_code_ = rig_get_ptt(rig_, RIG_VFO_CURR, &ptt);
	return (ptt != RIG_PTT_OFF);
}
