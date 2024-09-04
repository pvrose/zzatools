#include "rig_if.h"
#include "utils.h"
#include "status.h"

#include <climits>
#include <chrono>
#include <cmath>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>

extern status* status_;

// Returns if the rig opened OK
bool rig_if::is_open() {
	return opened_ok_;
}

// Convert s-meter reading into display format
string rig_if::get_smeter(bool max) {
	char text[100];
	int value = max ? (int)rig_data_.s_value : (int)rig_data_.s_meter;
	// SMeter value is relative to S9
	// Smeter value 0 indicates S9,
	// 1 S-point is 6 dB
	if (rig_data_.s_value < -54) {
		// Below S0
		snprintf(text, 100, " S0%ddB", 54 + value);
	}
	else if (rig_data_.s_value <= 0) {
		// Below S9 - convert to S points (6dB per S-point
		snprintf(text, 100, " S%1d", (54 + value) / 6);
	}
	else {
		// S9+
		snprintf(text, 100, " S9+%ddB", value);
	}

	return text;
}

// Return the transmit or receive frequency
double rig_if::get_dfrequency(bool tx) {
	// Read the frequency (in MHz)
	double frequency;
	if (tx) {
		frequency = rig_data_.tx_frequency / 1000000.0;
	}
	else {
		frequency = rig_data_.rx_frequency / 1000000.0;
	}
	if (modify_freq_) return frequency + freq_modifier_;
	else return frequency;
}

string rig_if::get_frequency(bool tx) {
	double frequency = get_dfrequency(tx);
	char text[15];
	snprintf(text, 15, "%0.6f", frequency);
	return text;
}

// Return the power
double rig_if::get_dpower(bool max) {
	double value = max ? (double)rig_data_.pwr_value : (double)rig_data_.pwr_meter;
	switch(modify_power_) {
		case UNMODIFIED: 
			break;
		case GAIN:
			value *= power_modifier_;
			break;
		case ABS_POWER:
			value = power_modifier_;
			break;
	}
	return value;
}

// Convert power to string
string rig_if::get_tx_power(bool max) {
	char text[100];
	snprintf(text, 100, "%g", get_dpower(max));
	return text;
}

// Converts rig mode to ADIF mode/submode
void rig_if::get_string_mode(string& mode, string& submode) {
	rig_mode_t rig_mode = rig_data_.mode;
	mode = "";
	submode = "";
	switch (rig_mode) {
	case GM_INVALID:
		mode = "???";
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
	case GM_DSTAR:
		mode = "DIGITALVOICE";
		submode = "DSTAR";
		return;
	default:
		return;
	}
}

// Get current PTT state
bool rig_if::get_ptt() {
	return rig_data_.ptt;
}

// Get rig access is slow
bool rig_if::get_slow() {
	return rig_data_.slow;
}

// Get current power state
bool rig_if::get_powered() {
	return rig_data_.powered_on;
}

// Constructor
rig_if::rig_if(const char* name, hamlib_data_t* data) 
{
	// Initialise
	opening_ = false;
	opened_ok_ = false;
	// Default action
	have_freq_to_band_ = false;
	inhibit_repeated_errors = false;
	rig_ = nullptr;
	error_code_ = 0;
	unsupported_function_ = false;
	my_rig_name_ = name;
	hamlib_data_ = data;
	run_read_ = false;
	// Modifiers
	modify_freq_ = false;
	freq_modifier_ = 0.0;
	modify_power_ = UNMODIFIED;
	power_modifier_ = 0.0;
	// Last PTT off
	last_ptt_off_ = system_clock::now();;
	
	// If the name has been set, there is a rig
	if (my_rig_name_.length()) {
		if (hamlib_data_->port_type != RIG_PORT_NONE) {
			open();
		}
		else {
			char msg[128];
			snprintf(msg, 128, "RIG: Not opening %s - no CAT available", my_rig_name_.c_str());
			status_->misc_status(ST_WARNING, msg);
		}
	}
}

// Destructor
rig_if::~rig_if()
{
	// close the connection
	close();
}

// Clsoe the connection
void rig_if::close() {


	if (rig_ != nullptr) {
		if (opened_ok_) {
			char msg[128];
			snprintf(msg, 128, "RIG: Closing connection %s (%s/%s on port %s)",
				my_rig_name_.c_str(),
				hamlib_data_->mfr.c_str(),
				hamlib_data_->model.c_str(),
				hamlib_data_->port_name.c_str());
			status_->misc_status(ST_NOTE, msg);
			// If we have a connection and it's open, close it and tidy memory used by hamlib
			// Delete the thread that reads the required rig values
			run_read_ = false;
			thread_->join();
			delete thread_;
			rig_close(rig_);
			rig_cleanup(rig_);
		}
	}
	opened_ok_ = false;
}

// Opens the connection associated with the rig
bool rig_if::open() {
	if (hamlib_data_->port_type == RIG_PORT_NONE) {
		char msg[256];
		snprintf(msg, 256, "RIG: Connection %s/%s No port to connect",
			hamlib_data_->mfr.c_str(),
			hamlib_data_->model.c_str()
		);
		status_->misc_status(ST_WARNING, msg);
 	} 
	else if (hamlib_data_->port_name.length() == 0) {
		char msg[256];
		snprintf(msg, 256, "RIG: Connection %s/%s No port supplied - open failed",
			hamlib_data_->mfr.c_str(),
			hamlib_data_->model.c_str()
		);
		status_->misc_status(ST_WARNING, msg);
		return false;

	}
	opening_.store(true, memory_order_seq_cst);
	thread_ = new thread(th_run_rig, this);
	// Lock until the rig has been opened - should wait 
	int timer = 0;
	const int interval = 100;
	const int timeout = 150000;
	while(opening_) {
		// Allow FLTK scheduler in
		Fl::check();
		this_thread::sleep_for(chrono::milliseconds(interval));
		timer += interval;
		if (timer >= timeout) {
			char msg[128];
			snprintf(msg, sizeof(msg), "RIG: Connecting %s abandoned after %d s", 
				hamlib_data_->model.c_str(),
				timeout / 1000);
			status_->misc_status(ST_WARNING, msg);
			opening_.store(false);
		}
	}
	// And immediately unlock it
		
	char msg[256];
	if (opened_ok_) {

		if (hamlib_data_->port_type == RIG_PORT_SERIAL) {
			snprintf(msg, 256, "RIG: Connection %s/%s on port %s opened OK",
				hamlib_data_->mfr.c_str(),
				hamlib_data_->model.c_str(),
				hamlib_data_->port_name.c_str());
		}
		else {
			snprintf(msg, 256, "RIG: Connection %s/%s on port %s (%s) opened OK",
				hamlib_data_->mfr.c_str(),
				hamlib_data_->model.c_str(),
				hamlib_data_->port_name.c_str(),
				rig_get_info(rig_));
		}
		status_->misc_status(ST_OK, msg);
		return true;
	}
	else {
		snprintf(msg, 256, "open(): %s/%s port %s",
			hamlib_data_->mfr.c_str(),
			hamlib_data_->model.c_str(),
			hamlib_data_->port_name.c_str()
			);
		status_->misc_status(ST_WARNING, error_message(msg).c_str());
		return false;
	}
}

// This is within the thread
bool rig_if::th_open_rig() {
	// If changing rig type - delete any existing rig and create new one
	if (rig_ != nullptr) {
		close();
	}
	// Get the rig interface
	rig_ = rig_init(hamlib_data_->model_id);
	if (rig_ != nullptr) {
		switch (hamlib_data_->port_type) {
		case RIG_PORT_SERIAL:
			// Successful - set up the serial port parameters
			strcpy(rig_->state.rigport.pathname, hamlib_data_->port_name.c_str());
			rig_->state.rigport.parm.serial.rate = hamlib_data_->baud_rate;
			break;
	// open r
		case RIG_PORT_NETWORK:
		case RIG_PORT_USB:
			int err = rig_set_conf(rig_, rig_token_lookup(rig_, "rig_pathname"), hamlib_data_->port_name.c_str());
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
	return opened_ok_;
}

// Return rig name
string& rig_if::rig_name() {
	// Read capabilities to get manufacturer and model name
	full_rig_name_ = rig_get_info(rig_);
	return full_rig_name_;
}

// Thraed method
void rig_if::th_run_rig(rig_if* that) {
	// Open the rig
	if (!that->th_open_rig()) {
		that->opening_ = false;
		return;
	}
	// run_read_ will be cleared when the rig closes or errors.
	that->th_read_values();
	if (that->opened_ok_) {
		that->opening_ = false;
		that->run_read_ = true;
		while (that->run_read_) {
			// Read the values from the rig once per second
			that->th_read_values();
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	} else {
		that->opening_ = false;
	}
}

// Read the data from the rig
void rig_if::th_read_values() {
	system_clock::time_point start = system_clock::now();
	if (hamlib_data_->port_type == RIG_PORT_NONE) {
		return;
	}
	// Check powered on
	powerstat_t power_state;
	if (opened_ok_) error_code_ = rig_get_powerstat(rig_, &power_state);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	switch (power_state) {
		case RIG_POWER_ON:
		case RIG_POWER_STANDBY: 
		{
			rig_data_.powered_on = true;
			break;
		}
		case RIG_POWER_OFF:
		{
			rig_data_.powered_on = false;
			return;
		}
	}
	// Read TX frequency
	double d_temp;
	if (opened_ok_) error_code_ = rig_get_freq(rig_, RIG_VFO_TX, &d_temp);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	rig_data_.tx_frequency = d_temp;
	// Read RX frequency
	bool qsy = false;
	if (opened_ok_) error_code_ = rig_get_freq(rig_, RIG_VFO_CURR, &d_temp);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	// Check if qSY since last report - used later to reset S-meter
	if (d_temp != rig_data_.rx_frequency) qsy = true;
	rig_data_.rx_frequency = d_temp;
	// Read mode
	rmode_t mode;
	shortfreq_t bandwidth;
	if (opened_ok_) error_code_ = rig_get_mode(rig_, RIG_VFO_CURR, &mode, &bandwidth);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	// Convert hamlib mode encoding to ZLG encoding
	rig_data_.mode = GM_INVALID;
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
	if (mode & RIG_MODE_DSTAR) {
		rig_data_.mode = GM_DSTAR;
	}
	// Read drive level
	value_t drive_level;
	if (opened_ok_) error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, &drive_level);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	rig_data_.drive = drive_level.f * 100;
	// Split
	vfo_t TxVFO;
	split_t split;
	if (opened_ok_) error_code_ = rig_get_split_vfo(rig_, RIG_VFO_CURR, &split, &TxVFO);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	rig_data_.split = split == split_t::RIG_SPLIT_ON;
	// PTT value
	ptt_t ptt;
	bool current_ptt = rig_data_.ptt;
	if (opened_ok_) error_code_ = rig_get_ptt(rig_, RIG_VFO_CURR, &ptt);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	rig_data_.ptt = ptt == ptt_t::RIG_PTT_ON;
	// If we are releasing PTT record the time
	if (current_ptt & !ptt) {
		last_ptt_off_ = system_clock::now();
	}
	// S-meter - set to max value during RX and last RX value during TX
	value_t meter_value;
	if (opened_ok_) error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_STRENGTH, &meter_value);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	// TX->RX (or changed RX frequency - use read value
	if (current_ptt && !rig_data_.ptt || qsy) {
		rig_data_.s_value = meter_value.i;
	}
	// RX use accumulated maximum value
	else if (!rig_data_.ptt) {
		if (meter_value.i > rig_data_.s_value) {
			rig_data_.s_value = meter_value.i;
		}
	}
	rig_data_.s_meter = meter_value.i;
	// Power meter
	if (opened_ok_) error_code_ = rig_get_level(rig_, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER_WATTS, &meter_value);
	else return;
	if (error_code_ != RIG_OK) {
		opened_ok_ = false;
		return;
	}
	// RX->TX - reset the power level unless...
	if (!current_ptt && rig_data_.ptt) {
		// ...PTT is released for only a few seconds (e.g. CW break-in or SSB VOX)
		seconds gap = duration_cast<seconds>(system_clock::now() - last_ptt_off_);
		if ( gap > seconds(5)) {
			rig_data_.pwr_value = meter_value.f;
		}
	}
	// TX use accumulated maximum value
	else if (rig_data_.ptt) {
		if ((double)meter_value.f > rig_data_.pwr_value) {
			rig_data_.pwr_value = (double)meter_value.f;
		}
	}
	// Also store iommediate value
	rig_data_.pwr_meter = (double)meter_value.f;
	// All successful
	count_down_ = FAST_RIG_TIMER;
	// Check rig response time and inform use if it's slowed down or back to normal
	char msg[128];
	system_clock::time_point finish = system_clock::now();
	milliseconds response = duration_cast<milliseconds>(finish - start);
	if (rig_data_.slow) {
		if (response < milliseconds(100)) {
			snprintf(msg, sizeof(msg), "RIG %s Responding normally %dms", 
				my_rig_name_.c_str(), response);
			status_->misc_status(ST_NOTE, msg);
			rig_data_.slow = false;
		}
	} else if (response > milliseconds((int)(hamlib_data_->timeout * 1000.0))) {
			snprintf(msg, sizeof(msg), "RIG %s Responding slowly %dms", 
				my_rig_name_.c_str(), response);
			status_->misc_status(ST_ERROR, msg);
		rig_data_.slow = true;
	}

	return;
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

// Error code is network error
bool rig_if::is_network_error() {
	return abs(error_code_) == (int)RIG_EIO;
}

// Error code is rig error
bool rig_if::is_rig_error() {
	return abs(error_code_) == (int)RIG_EPROTO;
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

// returns true if the connection is in the process of being opened
bool rig_if::is_opening() {
	return opening_;
}

// Set the frqeuency modifier (for whan a transverter is attached)
void rig_if::set_freq_modifier(double delta_freq) {
	modify_freq_ = true;
	freq_modifier_ = delta_freq;
}

// Clear the frequency modiier
void rig_if::clear_freq_modifier() {
	modify_freq_ = false;
}

// Set power modifier to a gain (in dB)
void rig_if::set_power_modifier(int gain) {
	modify_power_ = GAIN;
	power_modifier_ = pow(10.0, (double)gain/10.0);
}

// Set power modifier to an aboslute power (in W)
void rig_if::set_power_modifier(double power) {
	modify_power_ = ABS_POWER;
	power_modifier_ = power;
}

// Clear power modifier
void rig_if::clear_power_modifier() {
	modify_power_ = UNMODIFIED;
	power_modifier_ = 0;
}

// Returns true if the rig has CAT control
bool rig_if::has_no_cat() {
	return (hamlib_data_->port_type == RIG_PORT_NONE);
}

