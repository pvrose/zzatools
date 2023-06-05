#include "rig_if.h"
#include "formats.h"
#include "utils.h"
#include "status.h"

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>



extern Fl_Preferences* settings_;
extern status* status_;

// Returns if the rig opened OK
bool rig_if::is_open() {
	return opened_ok_;
}

// Convert s-meter reading into display format
string rig_if::get_smeter() {
	char text[100];
	// SMeter value is relative to S9
	// Smeter value 0 indicates S9,
	// 1 S-point is 6 dB
	if (rig_data_.s_value < -54) {
		// Below S0
		snprintf(text, 100, " S0%ddB", 54 + rig_data_.s_value);
	}
	else if (rig_data_.s_value <= 0) {
		// Below S9 - convert to S points (6dB per S-point
		snprintf(text, 100, " S%1d", (54 + rig_data_.s_value) / 6);
	}
	else {
		// S9+
		snprintf(text, 100, " S9+%ddB", rig_data_.s_value);
	}

	return text;
}

// Convert SWR meter into display formet
string rig_if::get_swr_meter() {
	char text[100];
	snprintf(text, 100, "1:%.1f", rig_data_.swr);
	return text;
}

// Returns the string displaying the rig information for the rig status
// e.g. Hamlib: TS-2000 14.123456 MHz 10 W USB S9+20
string rig_if::rig_info() {
	string rig_info = hamlib_data_.model;
	// Valid rig - get data from it. TX frequency
	if (rig_data_.split) {
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
		frequency = rig_data_.tx_frequency / 1000000.0;
	}
	else {
		frequency = rig_data_.rx_frequency / 1000000.0;
	}
	char text[15];
	snprintf(text, 15, "%0.6f", frequency);

	return text;

}

// Return the power
string rig_if::get_tx_power() {
	char text[100];
	snprintf(text, 100, "%g", rig_data_.pwr);
	return text;
}

// 1 second time from clock 
void rig_if::ticker() {
	count_down_--;
	if (count_down_ == 0) {
		read_values();
	} 
	if (opened_ok_) {
		count_down_ = FAST_RIG_TIMER;
	}
	else {
		count_down_ = SLOW_RIG_TIMER;
	}
}

// Converts rig mode to string
void rig_if::get_string_mode(string& mode, string& submode) {
	rig_mode_t rig_mode = rig_data_.mode;
	mode = "";
	submode = "";
	switch (rig_mode) {
	case GM_INVALID:
		status_->misc_status(ST_ERROR, "RIG: Invalid mode got from rig");
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
rig_if::rig_if(const char* name, hamlib_data_t data) 
{
	// Initialise
	opened_ok_ = false;
	// Default action
	have_freq_to_band_ = false;
	inhibit_repeated_errors = false;
	rig_ = nullptr;
	error_code_ = 0;
	unsupported_function_ = false;
	reported_no_vdd_ = false;
	reported_no_swr_ = false;

	my_rig_name_ = name;
	hamlib_data_ = data;
	
	// If the name has been set, there is a rig
	if (my_rig_name_.length()) {
		if (hamlib_data_.port_type != RIG_PORT_NONE) {
			open();
		}
		else {
			char msg[128];
			snprintf(msg, 128, "RIG: Not opening %s - no hamlib available", my_rig_name_.c_str());
			status_->misc_status(ST_WARNING, msg);
		}
	}
}

// Destructor
rig_if::~rig_if()
{
	// close the serial port
	close();
}

// Clsoe the serial port
void rig_if::close() {


	if (rig_ != nullptr) {
		if (opened_ok_) {
			char msg[128];
			snprintf(msg, 128, "RIG: Closing connection %s (%s on port %s)",
				my_rig_name_.c_str(),
				hamlib_data_.model.c_str(),
				hamlib_data_.port_name.c_str());
			status_->misc_status(ST_NOTE, msg);
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
	if (hamlib_data_.port_name.length() == 0) {
		char msg[256];
		snprintf(msg, 256, "RIG: Connection %s/%s No port supplied - open failed",
			hamlib_data_.mfr.c_str(),
			hamlib_data_.model.c_str()
		);
		status_->misc_status(ST_WARNING, msg);
		return false;

	}
	// Get the rig interface
	rig_ = rig_init(hamlib_data_.model_id);
	if (rig_ != nullptr) {
		switch (hamlib_data_.port_type) {
		case RIG_PORT_SERIAL:
			// Successful - set up the serial port parameters
			strcpy(rig_->state.rigport.pathname, hamlib_data_.port_name.c_str());
			rig_->state.rigport.parm.serial.rate = hamlib_data_.baud_rate;
			break;
		case RIG_PORT_NETWORK:
		case RIG_PORT_USB:
			int err = rig_set_conf(rig_, rig_token_lookup(rig_, "rig_pathname"), hamlib_data_.port_name.c_str());
			//strcpy(rig_->state.rigport.pathname, port_name_.c_str());
			break;
		}
	}
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
	char msg[256];
	if (opened_ok_) {

		if (hamlib_data_.port_type == RIG_PORT_SERIAL) {
			snprintf(msg, 256, "RIG: Connection %s/%s on port %s opened OK",
				hamlib_data_.mfr.c_str(),
				hamlib_data_.model.c_str(),
				hamlib_data_.port_name.c_str());
		}
		else {
			snprintf(msg, 256, "RIG: Connection %s/%s on port %s (%s) opened OK",
				hamlib_data_.mfr.c_str(),
				hamlib_data_.model.c_str(),
				hamlib_data_.port_name.c_str(),
				rig_get_info(rig_));
		}
		status_->misc_status(ST_NOTE, msg);
		read_values();
		return true;
	}
	else {
		snprintf(msg, 256, "open(): %s/%s port %s",
			hamlib_data_.mfr.c_str(),
			hamlib_data_.model.c_str(),
			hamlib_data_.port_name.c_str()
			);
		status_->misc_status(ST_ERROR, error_message(msg).c_str());
		return false;
	}
}

// Return rig name
string& rig_if::rig_name() {
	// Read capabilities to get manufacturer and model name
	full_rig_name_ = rig_get_info(rig_);
	return full_rig_name_;
}

bool rig_if::read_values() {
	opened_ok_ = false;
	rig_values current_values = rig_data_;
	// Read TX frequency
	error_code_ = rig_get_freq(rig_, RIG_VFO_TX, &rig_data_.tx_frequency);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("TX Frequency").c_str());
		return false;
	}
	// Read RX frequency
	error_code_ = rig_get_freq(rig_, RIG_VFO_CURR, &rig_data_.rx_frequency);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("RX Frequency").c_str());
		return false;
	}
	// Read mode
	rmode_t mode;
	shortfreq_t bandwidth;
	error_code_ = rig_get_mode(rig_, RIG_VFO_CURR, &mode, &bandwidth);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("Mode").c_str());
		return false;
	}
	// Convert hamlib mode encoding to ZLG encoding
	if (mode & RIG_MODE_AM) {
		rig_data_.mode = GM_AM;
	}
	if (mode & RIG_MODE_CW) {
		rig_data_.mode = GM_CWU;
	}
	if (mode & RIG_MODE_CWR) {
		rig_data_.mode = GM_CWL;
	}
	if (mode & RIG_MODE_LSB) {
		rig_data_.mode = GM_LSB;
	}
	if (mode & RIG_MODE_USB) {
		rig_data_.mode = GM_USB;
	}
	if (mode & RIG_MODE_FM) {
		rig_data_.mode = GM_FM;
	}
	if (mode & (RIG_MODE_RTTY | RIG_MODE_PKTLSB)) {
		rig_data_.mode = GM_DIGL;
	}
	if (mode & (RIG_MODE_RTTYR | RIG_MODE_PKTUSB)) {
		rig_data_.mode = GM_DIGU;
	}
	// Read drive level
	value_t drive_level;
	error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, &drive_level);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("Drive").c_str());
		return false;
	}
	rig_data_.drive = drive_level.f * 100;
	// Split
	vfo_t TxVFO;
	split_t split;
	error_code_ = rig_get_split_vfo(rig_, RIG_VFO_CURR, &split, &TxVFO);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("Split").c_str());
		return false;
	}
	rig_data_.split = split == split_t::RIG_SPLIT_ON;
	// PTT value
	ptt_t ptt;
	error_code_ = rig_get_ptt(rig_, RIG_VFO_CURR, &ptt);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("PTT").c_str());
		return false;
	}
	rig_data_.ptt = ptt == ptt_t::RIG_PTT_ON;
	// S-meter - set to max value during RX and last RX value during TX
	value_t meter_value;
	error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_STRENGTH, &meter_value);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("S meter").c_str());
		return false;
	}
	// TX->RX - use read value
	if (current_values.ptt && !rig_data_.ptt) {
		rig_data_.s_value = meter_value.i;
	}
	// RX use accumulated maximum value
	else if (!rig_data_.ptt) {
		rig_data_.s_value = max(rig_data_.s_value, meter_value.i);
	}
	// Power meter
	error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER_WATTS, &meter_value);
	if (error_code_ != RIG_OK) {
		status_->misc_status(ST_ERROR, error_message("Power meter").c_str());
		return false;
	}
	// RX->TX - use read value
	if (!current_values.ptt && rig_data_.ptt) {
		rig_data_.pwr = meter_value.f;
	}
	// TX use accumulated maximum value
	else if (rig_data_.ptt) {
		rig_data_.pwr = max(rig_data_.pwr, (double)meter_value.f);
	}
	// VDD meter
	if (!reported_no_vdd_) {
		error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_VD_METER, &meter_value);
		if (error_code_ != RIG_OK) {
			if (abs(error_code_) == RIG_ENAVAIL) {
				// If the rig has no VDD meter
				status_->misc_status(ST_WARNING, error_message("VDD meter").c_str());
				reported_no_vdd_ = true;
				error_code_ = RIG_OK;
			}
			else {
				status_->misc_status(ST_ERROR, error_message("VDD meter").c_str());
				return false;
			}
		}
	}
	rig_data_.vdd = meter_value.f;
	// SWR meter
	if (!reported_no_swr_) {
		error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_SWR, &meter_value);
		if (error_code_ != RIG_OK) {
			if (abs(error_code_) == RIG_ENAVAIL) {
				// If the rig has no VDD meter
				status_->misc_status(ST_WARNING, error_message("SWR meter").c_str());
				reported_no_swr_ = true;
				error_code_ = RIG_OK;
			}
			else {
				status_->misc_status(ST_ERROR, error_message("SWR meter").c_str());
				return false;
			}
		}
	}
	rig_data_.swr = meter_value.f;

	// All successful
	opened_ok_ = true;
	count_down_ = FAST_RIG_TIMER;
	return true;
}

// Return the most recent error message
string rig_if::error_message(const char* func_name) {
	char msg[256];
	snprintf(msg, 256, "RIG: Hamlib error \"%s\" processing %s",
		error_text((rig_errcode_e)abs(error_code_)),
		func_name);
	return string(msg);
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

rig_if::rig_values rig_if::get_data() {
	return rig_data_;
}

