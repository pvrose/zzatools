#include "wsjtx_handler.h"

#include "adi_reader.h"
#include "book.h"
#include "main.h"
#include "menu.h"
#include "qso_apps.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "record.h"
#include "regices.h"
#include "settings.h"
#include "socket_server.h"
#include "spec_data.h"
#include "status.h"
#include "ticker.h"
#include "toolbar.h"

#include "utils.h"

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#ifdef _WIN32
#include <WS2tcpip.h>
#else
//#include <fnctl.h>
#endif

#include <FL/Fl.H>
#include <FL/fl_ask.H>

// Constructor: 
wsjtx_handler::wsjtx_handler()
{
	server_ = nullptr;
	qsos_.clear();
	run_server();
	check_beats_ = false;
	received_beats_.clear();
	status_rcvd_ = 0;
	// Get logged callsign from spec_data
	my_call_ = qso_manager_->get_default(qso_manager::CALLSIGN);
	my_bracketed_call_ = "<" + my_call_ + ">";
	connected_ = false;
	ticker_->add_ticker(this, cb_ticker, 150);
}

// Destructor
wsjtx_handler::~wsjtx_handler() {
	ticker_->remove_ticker(this);
	close_server();
};

// This callback must be static
int wsjtx_handler::rcv_request(void* instance, std::stringstream& ss) {
	return ((wsjtx_handler*)instance)->rcv_dgram(ss);
}

// Receive the datagram. Decide which one an go to the individual decode methods
int wsjtx_handler::rcv_dgram(std::stringstream & ss) {
	// The first few objects are fixed for all datagrams
	magic_number_ = get_uint32(ss);
	schema_ = get_uint32(ss);
	uint32_t dgram_type = get_uint32(ss);
	// Check the magic number and schema are supported
	if (magic_number_ != expected_magic_ || schema_ < minimum_schema_) {
		char message[256];
		snprintf(message, 256, "WSJTX: datagram had wrong magic number (%08x) or unsupported schema (%d)", magic_number_, schema_);
		status_->misc_status(ST_WARNING, message);
		return 1;
	}
	id_ = get_utf8(ss);

	if (!connected_) {
		connected_ = true;
		qso_manager_->enable_widgets();
	}

	// Select method to interpret datagram
	switch (dgram_type) {
	case 0:
		// Heartbeat
		return handle_hbeat(ss);
	case 1:
		// Status
		return handle_status(ss);
	case 2:
		return handle_decode(ss);
	case 4:
		return handle_reply(ss);
	case 6:
		return handle_close(ss);
	case 12:
		return handle_log(ss);
	default:
		return handle_default(ss, dgram_type);
	}
	return 0;
}

// Ignore the datagrams that do not interest us
int wsjtx_handler::handle_default(std::stringstream& ss, uint32_t type) {
	char message[128];
	snprintf(message, 128, "WSJT-X: Ignored type %d datagram", type);
	status_->misc_status(ST_LOG, message);
	return 0;
}

// Handle the heartbeat - send one back
int wsjtx_handler::handle_hbeat(std::stringstream& ss) {
	check_beats_ = true;
	received_beats_.insert(id_);
	return 0;
}

// Send a heartbeat
int wsjtx_handler::send_hbeat() {
	std::stringstream ss;
	// Add the magic number, schema and function number
	put_uint32(ss, magic_number_);
	put_uint32(ss, schema_);
	// Frequency
	put_uint32(ss, 0);
	// Add the ID
	put_utf8(ss, PROGRAM_ID);
	// Add the max schema
	put_uint32(ss, 3);
	// Add the version
	put_utf8(ss, PROGRAM_VERSION);
	// Add the revision ""
	put_uint32(ss, (uint32_t)(~0));
	
	// Now go back to the start of the stream to send it
	ss.seekg(0, std::ios::beg);
	return server_->send_response(ss);
}

// Close datagram: shut the server down 
int wsjtx_handler::handle_close(std::stringstream& ss) {
	status_->misc_status(ST_NOTE, "WSJT-X: Received Closing down");
	connected_ = false;
	qso_manager_->cancel_modem_qso();
	// Clear any WSJT-X related items
	menu_->update_items();
	return 1;
}

// Handle the logged ADIF datagram. Send it to the logger
int wsjtx_handler::handle_log(std::stringstream& ss) {
	status_->misc_status(ST_LOG, "WSJT-X: Received Log ADIF datagram");
	// Get ADIF std::string
	std::string utf8 = get_utf8(ss);
	// Convert it to a record
	std::stringstream adif;
	adif.str(utf8);
	adi_reader* reader = new adi_reader;
	// The stream received from WSJT-X is header and record so create a book from it
	book* rcvd_book = new book(OT_WSJTX);
	reader->load_book(rcvd_book, adif);
	record* log_qso = rcvd_book->get_record(0, false);
	record* qso = qso_call(log_qso->item("CALL"), true);
	qso->merge_records(log_qso);
	qso->item("QSO_COMPLETE", std::string(""));
	qso_manager_->update_modem_qso(true);
	status_->misc_status(ST_NOTE, "WSJT-X: Logged QSO");
	delete rcvd_book;
	return 0;
}

// handle decode - display and beep if it contains the user's callsign
int wsjtx_handler::handle_decode(std::stringstream& ss) {
	decode_dg decode;
	decode.id = id_;
	decode.new_decode = get_bool(ss);
	decode.time = get_uint32(ss);
	decode.snr = get_uint32(ss);
	decode.d_time = get_double(ss);
	decode.d_freq = get_uint32(ss);
	decode.mode = get_utf8(ss);
	decode.message = get_utf8(ss);
	decode.low_confidence = get_bool(ss);
	decode.off_air = get_bool(ss);
	// display ID, time and message
	double seconds = decode.time / 1000.0;
	unsigned int minutes = (unsigned int)seconds / 60;
	seconds = seconds - (minutes * 60.0);
	unsigned int hours = minutes / 60;
	minutes = minutes - (hours * 60);
	char t[20];
	snprintf(t, sizeof(t), "%02d%02d%02.0f", hours, minutes, seconds);
	record* qso = update_qso(false, std::string(t), (double)decode.d_freq, decode.message);
	if (qso) qso_manager_->update_modem_qso(false);
	return 0;
}

// handle decode - display and beep if it contains the user's callsign
int wsjtx_handler::handle_reply(std::stringstream& ss) {
	decode_dg decode;
	decode.id = id_;
	decode.new_decode = get_bool(ss);
	decode.time = get_uint32(ss);
	decode.snr = get_uint32(ss);
	decode.d_time = get_double(ss);
	decode.d_freq = get_uint32(ss);
	decode.mode = get_utf8(ss);
	decode.message = get_utf8(ss);
	decode.low_confidence = get_bool(ss);
//	decode.off_air = get_bool(ss);
	// display ID, time and message
	double seconds = decode.time / 1000.0;
	unsigned int minutes = (unsigned int)seconds / 60;
	seconds = seconds - (minutes * 60.0);
	unsigned int hours = minutes / 60;
	minutes = minutes - (hours * 60);
	char t[20];
	snprintf(t, sizeof(t), "%02d%02d%02.0f", hours, minutes, seconds);
	record* qso = update_qso(false, std::string(t), (double)decode.d_freq, decode.message);
	if (qso) qso_manager_->update_modem_qso(false);
	return 0;
}

// handle status - display it if the DX Call has changed - indicates that user
// has called a new station
int wsjtx_handler::handle_status(std::stringstream& ss) {
	// Debug code
	std::string datagram = ss.str();

	status_dg status;
	// ID
	status.id = id_;
	// Frequency
	status.dial_freq = get_uint64(ss);
	// Mode
	status.mode = get_utf8(ss);
	// DX Call
	status.dx_call = get_utf8(ss);
	// Report 
	status.report = get_utf8(ss);
	// Transmit mode
	status.tx_mode = get_utf8(ss);
	// Transmit enabled
	status.tx_eanbled = get_bool(ss);
	// Transmit
	status.transmitting = get_bool(ss);
	// Decoding
	status.decoding = get_bool(ss);
	// Received frequency - delta from VFO
	status.rx_offset = get_uint32(ss);
	// Transmit frequency
	status.tx_offset = get_uint32(ss);
	// My call
	status.own_call = get_utf8(ss);
	// My grid
	status.own_grid = get_utf8(ss);
	// DX Grid
	status.dx_grid = get_utf8(ss);
	// Transmit ?
	status.tx_watchdog = get_bool(ss);
	// Submode
	status.submode = get_utf8(ss);
	// FAst decode
	status.fast_mode = get_bool(ss); 
	// Special operation mode
	status.special_op = get_uint8(ss); 
	// Frequency tolerance
	status.freq_tolerance = get_uint32(ss); 
	// ????
	status.tx_rx_period = get_uint32(ss);
	// Configuration name
	status.config_name = get_utf8(ss); 
	// TX Message
	status.tx_message = get_utf8(ss);
	// Save frequency and mode
	dial_frequency_ = (double)status.dial_freq / 1000000.0;
	mode_ = status.mode;
	// Create qso
	if (status.transmitting) {
		// Onl;y update QSO if not already doing so - this is called by multiple threads
		if (!tx_semaphore_.test_and_set()) {
			record* qso = update_qso(true, now(false, "%H%M%S"), (double)status.tx_offset, status.tx_message);
			if (qso && qso->item("CALL").substr(0,2) != "CQ") qso_manager_->update_modem_qso(false);
			tx_semaphore_.clear();
		}
	}

	prev_status_ = status;
	return 0;
}

// Get an bool from the next byte of datagram 
bool wsjtx_handler::get_bool(std::stringstream& ss) {
	unsigned char c;
	ss.get((char&)c);
	bool b = c ? true : false;
	return b;
}

// Get an unsigned integer from the next byte of datagram 
uint8_t wsjtx_handler::get_uint8(std::stringstream& ss) {
	unsigned char c;
	ss.get((char&)c);
	uint8_t i = c;
	return i;
}

// Get an unsigned integer from the next 4 bytes of dgram 
uint32_t wsjtx_handler::get_uint32(std::stringstream& ss) {
	unsigned char c = 0;
	uint32_t i = 0;
	for (int ix = 0; ix < 4; ix++) {
		ss.get((char&)c);
		i = (i << 8) + c;
	}
	return i;
}

// Get 64-bit unsigned integer from the next 8 bytes of datagram 
uint64_t wsjtx_handler::get_uint64(std::stringstream& ss) {
	unsigned char c = 0;
	uint64_t i = 0LL;
	for (int ix = 0; ix < 8; ix++) {
		ss.get((char&)c);
		i = (i << 8) + c;
	}
	return i;
}

// Get double from the next 8-bytes of datagram
double wsjtx_handler::get_double(std::stringstream& ss) {
	// I'm making the assumption that the double has been directly serialised in its bit pattern
	uint64_t uv = get_uint64(ss);
	double* d = reinterpret_cast<double*>(&uv);
	return *d;
}

// Get a std::string from the QByteArray (4 byte length + that number of bytes)
std::string wsjtx_handler::get_utf8(std::stringstream& ss) {
	// Get 4-byte length
	uint32_t len = get_uint32(ss);
	if (len == ~(0)) {
		// Length is all 1's - represents a null std::string
		return "";
	}
	else {
		// Create a std::string long enough to receive the data
		std::string s = "";
		s.reserve(len + 1);
		for (uint32_t i = 0; i < len; i++) {
			// Copy the std::string 1 byte at a timme
			char c;
			ss.get(c);
			s += c;
		}
		return s;
	}
}

// Put an integer as 4 bytes in the next 4 bytes of dgram
void wsjtx_handler::put_uint32(std::stringstream& ss, uint32_t i) {
	for (int ix = 24; ix >= 0; ix-=8) {
		unsigned char c = (i >> ix) & 0xFF;
		ss.put(c);
	}
}

// Put a std::string as QByteArray (see above) into the dgram
void wsjtx_handler::put_utf8(std::stringstream& ss, std::string s) {
	put_uint32(ss, s.length());
	for (unsigned int ix = 0; ix < s.length(); ix++) {
		ss.put(s[ix]);
	}
}

// Return that the server is there
bool wsjtx_handler::has_server() {
	return server_ != nullptr && server_->has_server();
}

// Start the server
void wsjtx_handler::run_server() {
	if (!server_) {
		std::string address = qso_manager_->apps()->network_address(WSJTX);
		int udp_port = qso_manager_->apps()->network_port(WSJTX);
		if (address.length()) {
			server_ = new socket_server(socket_server::UDP, address, udp_port);
			server_->callback(this, rcv_request);
		}
		else return;
	}
	if (!server_->has_server()) {
		status_->misc_status(ST_NOTE, "WSJT-X: Starting socket");
		server_->run_server();
	}
	menu_->update_items();
}

// Close the server
void wsjtx_handler::close_server() {
	if (server_) {
		status_->misc_status(ST_NOTE, "WSJT-X: Closing server");
		server_->close_server(true);
		delete server_;
		server_ = nullptr;
	}
}

wsjtx_handler::decoded_msg wsjtx_handler::decode_message(std::string message) {
	// TODO: Currently assuming non F/H exchange
	decoded_msg decode;
	std::vector<std::string> words;
	// Prune the trailing spaces from message
	size_t ix = message.length();
	while(message[--ix] == ' ');
	std::string pruned = message.substr(0, ix+1);
	split_line(pruned, words, ' ');

	std::string test_exch = words.back();
	const char* test = test_exch.c_str();
	if (strcmp(test, "RR73") == 0) {
		// TX4A
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX4A;
	}
	else if (regex_match(test, REGEX_GRIDSQUARE4)) {
		// TX1 or TX6
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		if (regex_match(decode.target.c_str(), REGEX_CQ)) {
			decode.type = TX6;
		}
		else {
			decode.type = TX1;
		}
	}
	else if (regex_match(test, REGEX_REPORT)) {
		// TX2
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX2;
	}
	else if (regex_match(test, REGEX_ROGER)) {
		// TX3
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX3;
	}
	else if (strcmp(test, "RRR") == 0) {
		// TX4
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX4;
	}
	else if (strcmp(test, "73") == 0) {
		// TX5
		decode.exchange = test_exch;
		words.pop_back();
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		decode.type = TX5;
	}
	else {
		// No exchange - unusual call either TX1A or TX6A
		decode.exchange = "";
		decode.sender = words.back();
		words.pop_back();
		decode.target = join_line(words, ' ');
		if (regex_match(decode.target.c_str(), REGEX_CQ)) {
			decode.type = TX6A;
		}
		else {
			decode.type = TX1A;
		}
	}
	return decode;
}

// Update QSO - returns true if updated and let qso_manager know
record* wsjtx_handler::update_qso(bool tx, std::string time, double audio_freq, std::string message, record* match, double dial, std::string mode) {
	decoded_msg decode = decode_message(message);
	std::string sender = decode.sender[0] == '<' ? decode.sender.substr(1, decode.sender.length() - 2) : decode.sender;
	std::string target = decode.target[0] == '<' ? decode.target.substr(1, decode.target.length() - 2) : decode.target;
	std::string today = now(false, "%Y%m%d");
	double df = dial == 0.0 ? dial_frequency_ : dial;
	std::string m = mode == "" ? mode_ : mode;
	char msg[100];
	if (tx) {
		if (sender == "TUNE") {
			// Quietly ignore
			return nullptr;
		}
		else if (sender != my_call_) {
			snprintf(msg, sizeof(msg), "WSJTX: TX decode not for user %s: %s", my_call_.c_str(), message.c_str());
			status_->misc_status(ST_WARNING, msg);
			return nullptr;
		}
		else {
			record* qso;
			switch(decode.type) {
				case TX6:
				case TX6A: {
					// Do not create a record if CQ
					qso = nullptr;
					break;
				}
				default: {
					qso = match != nullptr ? match : qso_call(target, true);
					qso->item("QSO_COMPLETE", std::string("N"));
					break;
				}
			}
			switch (decode.type) {
			case TX1:
				// <THEM> <ME> <MY_GRID>
			case TX1A:
				// <THEM> <ME> 
			{
				// I am starting a new call - std::set date/ time/freq/mode
				if (qso->item("QSO_DATE") == "") qso->item("QSO_DATE", today);
				qso->item("TIME_ON", time);
				char f[20];
				double freq = df + (audio_freq / 1000000.0);
				snprintf(f, sizeof(f), "%0.6f", freq);
				qso->item("FREQ", std::string(f));
				qso->item("MODE", m);
				if (grid_cache_.find(target) != grid_cache_.end()) {
					qso->item("GRIDSQUARE", grid_cache_.at(target));
				}
				return qso;
			}
			case TX2:
			{
				// <THEM> <ME> <Report>
				// Starting a new record
				if (qso->item("QSO_DATE") == "") qso->item("QSO_DATE", today);
				if (qso->item("TIME_ON") == "") qso->item("TIME_ON", time);
				char f[20];
				double freq = df + (audio_freq / 1000000.0);
				snprintf(f, sizeof(f), "%0.6f", freq);
				qso->item("FREQ", std::string(f));
				qso->item("MODE", m);
				// Set grid square: from cache or no value
				if (grid_cache_.find(target) != grid_cache_.end()) {
					qso->item("GRIDSQUARE", grid_cache_.at(target));
				}
				else {
					qso->item("GRIDSQUARE", std::string(""));
				}
				// Add report to existing QSO
				qso->item("RST_SENT", decode.exchange);
				return qso;
			}
			case TX3:
			{
				// <THEM> <ME> R<report>
				qso->item("RST_SENT", decode.exchange.substr(1));
				char f[20];
				double freq = df + (audio_freq / 1000000.0);
				snprintf(f, sizeof(f), "%0.6f", freq);
				qso->item("FREQ", std::string(f));
				return qso;
			}
			case TX4:
			{
				// <THEM> <ME> RRR
				qso->item("QSO_COMPLETE", std::string("?"));
				char f[20];
				double freq = df + (audio_freq / 1000000.0);
				snprintf(f, sizeof(f), "%0.6f", freq);
				qso->item("FREQ", std::string(f));
				return qso;
			}
			case TX4A:
				// <THEM> <ME> RR73
			case TX5:
			{
				// <THEM><ME> 73
				char f[20];
				double freq = df + (audio_freq / 1000000.0);
				snprintf(f, sizeof(f), "%0.6f", freq);
				qso->item("FREQ", std::string(f));
				if(qso->item("QSO_COMPLETE") == "N" || qso->item("QSO_COMPLETE") == "?")
					qso->item("QSO_COMPLETE", std::string("Y"));
				qso->item("TIME_OFF", time);
				return qso;
			}
			case TX6:
				// CQ <ME> <GRID>
			case TX6A:
				// CQ <ME> 
			default:
				return nullptr;
			}
		}
	}
	else {
		// RX
		if (target != my_call_) {
			// Not for me
			switch (decode.type) {
			case TX1:
				// Call - capture in grid
				grid_cache_[sender] = decode.exchange;
				return nullptr;
			case TX6:
				grid_cache_[sender] = decode.exchange;
				// TODO send tqso_callo new object wsjtx_runner
				return nullptr;
			case TX4:
			case TX4A:
			case TX5:
				// TODO send to new object wsjtx_runner
				return nullptr;
			default:
				return nullptr;
			}
		}
		else {
			record* qso = match != nullptr ? match : qso_call(sender, false);
			if (qso) {
				qso->item("QSO_COMPLETE", std::string("N"));
				switch (decode.type) {
				case TX1:
					// <ME> <CALL> <GRID>
					grid_cache_[sender] = decode.exchange;
					return nullptr;
				case TX1A:
					// <ME> <CALL>
					// Wait until I send the call before starting log entry as may get >1 per decode period
					return nullptr;
				case TX2:
					// <ME> <THEM> <report>
					if (qso->item("GRIDSQUARE").length() == 0) {
						if (grid_cache_.find(target) != grid_cache_.end()) {
							qso->item("GRIDSQUARE", grid_cache_.at(target));
						}
						else {
							qso->item("GRIDSQUARE", std::string(""));
						}
					}
					qso->item("RST_RCVD", decode.exchange);
					return qso;
				case TX3:
					// <ME> <THEM> R<report>
					qso->item("RST_RCVD", decode.exchange.substr(1));
					return qso;
				case TX4:
					// <ME> <THEM> RRR
					qso->item("QSO_COMPLETE", std::string("?"));
					return qso;
				case TX4A:
					// <ME> <THEM> RR73
					qso->item("QSO_COMPLETE", std::string("?"));
					// And drop through
				case TX5:
					// <ME> <THEM> 73
					if (qso->item("QSO_COMPLETE") == "N" || qso->item("QSO_COMPLETE") == "?") {
						qso->item("QSO_COMPLETE", std::string("Y"));
					}
					qso->item("TIME_OFF", time);
					return qso;
				default:
					return nullptr;
				}
			} else {
				if (decode.type == TX1) {
					grid_cache_[sender] = decode.exchange;
				}
				return qso;
			}
		}
	}
}

// Create a new record to hold data received from WSJT-X
record* wsjtx_handler::new_qso(std::string call) {
	qsos_[call] = nullptr;
	record * qso = qso_manager_->start_modem_qso(call, qso_data::QSO_COPY_WSJTX);
	qso->item("CALL", call);
	qso->item("BAND", spec_data_->band_for_freq(dial_frequency_));
	qso->item("MODE", mode_);
	qso->item("QSO_COMPLETE", std::string("N"));
	qsos_[call] = qso;
	qso_manager_->update_modem_qso(false);
	return qso;
}

// Get a QSO record for this callsign
record* wsjtx_handler::qso_call(std::string call, bool create) {
	record* qso = nullptr;
	if (qsos_.find(call) == qsos_.end()) {
		if (create) {
			qso = new_qso(call);
		}
	}
	else {
		qso = qsos_.at(call);
		std::string band = spec_data_->band_for_freq(dial_frequency_);
		if (qso->item("MODE") == mode_ && qso->item("BAND") == band) {
			if (qso->item("TIME_OFF") != "") {
				time_t now = time(nullptr);
				time_t qso_ts = qso->timestamp(true);
				if (difftime(now, qso_ts) > (5 * 60)) {
					switch (
						fl_choice("Received update to QSO %s %s %s, is this...", "the same QSO??", "a new QSO?", nullptr,
							call.c_str(), band.c_str(), mode_.c_str())) {
					case 0:
						break;
					case 1:
						qso = new_qso(call);
						break;
					}
				}
			}
		}
		else {
			if (create) {
				qso = new_qso(call);
			}
		}
	}
	return qso;
}

// Parse a record in the "ALL.TXT" file
bool wsjtx_handler::parse_all_txt(record* qso, std::string line) {
	bool tx_record;
	double dial_frequency = 0.0;
	double audio_frequency = 0.0;
	std::string time_on = "";
	// After this initial processing pos will point to the start og the QSO decode std::string - look for old-style transmit record
	size_t pos = line.find("Transmitting");
	if (pos != std::string::npos) {
		pos = line.find(qso->item("MODE"));
		pos += qso->item("MODE").length() + 3;
		tx_record = true;
		time_on = line.substr(7,6);
		// Nothing else to get from this record
	}
	else {
		// Now see if it's a new-style Tx record
		pos = line.find("Tx");
		if (pos != std::string::npos) {
			// Get frequency of transmission - including audio offset
			std::string freq = line.substr(14, 9);
			// Replace leading spaces with zeroes
			for (size_t i = 0; freq[i] == ' '; i++) {
				freq[i] = '0';
			}
			dial_frequency = std::stod(freq);
			std::string freq_offset = line.substr(43, 4);
			// Replace leading spaces with zeroes
			for (size_t i = 0; freq_offset[i] == ' '; i++) {
				freq_offset[i] = '0';
			}
			audio_frequency = std::stod(freq_offset);
			pos = 48;
			tx_record = true;
			time_on = line.substr(7,6);
		}
		else {
			// Look for a new-style Rx record
			pos = line.find("Rx");
			if (pos != std::string::npos) {
				pos = 48;
				tx_record = false;
				time_on = line.substr(7,6);
			}
			else {
				// Default to old-style Rx record
				pos = 24;
				tx_record = false;
				time_on = line.substr(0,6);
			}
		}
	}
	if (update_qso(tx_record, time_on, audio_frequency, line.substr(pos), qso, dial_frequency, qso->item("MODE")) ) {
		return true;
	} else {
		return false;
	}
}

// Look for details of the supplied QSO in the "ALL.TXT" file
bool wsjtx_handler::match_all_txt(record* qso, bool update_qso) {
	settings top_settings;
	settings behav_settings(&top_settings, "Behaviour");
	std::string temp;
	behav_settings.get<std::string>("WSJT-X", temp, "");
	std::string filename = temp + "/ALL.TXT";
	ifstream* all_file = new ifstream(filename.c_str());
	if (!all_file->good()) {
		char msg[100];
		snprintf(msg, sizeof(msg), "WSJTX: Fail to open %s", filename.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	// This will take a while so display the timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	// calculate the file size and initialise the progress bar
	std::streampos startpos = all_file->tellg();
	all_file->seekg(0, std::ios::end);
	std::streampos endpos = all_file->tellg();
	long file_size = (long)(endpos - startpos);
	status_->misc_status(ST_NOTE, "WSJTX: Starting to parse ALL.TXT");
	status_->progress(file_size, OT_RECORD, "Looking for queried call in ALL.TXT", "bytes");
	// reposition back to beginning
	all_file->seekg(0, std::ios::beg);
	enum {SEARCHING, FOUND, COPYING, COPIED} copy_status = SEARCHING;
	std::string s_status[] = {"SEARCHING", "FOUND    ", "COPYING  ", "COPIED   "};
	// Get user callsign from settings
	// Get search items from record
	std::string their_call = qso->item("CALL");
	std::string datestamp = qso->item("QSO_DATE");
	std::string timestamp = qso->item("TIME_ON");
	std::string mode = qso->item("MODE");
	if (their_call.length() == 0 || datestamp.length() != 8 || 
		timestamp.length() < 4 || mode.length() == 0) {
		status_->misc_status(ST_ERROR, "WSJTX: Searching ALL.TXT requires, call, date/time and mode");
		return false;
	}
	datestamp = datestamp.substr(2);
	char msg[256];
	snprintf(msg, sizeof(msg), "WSJTX: Searching ALL.TXT for %s %s %s %s",
		their_call.c_str(), datestamp.c_str(), timestamp.c_str(), mode.c_str());
	status_->misc_status(ST_NOTE, msg);
	int count = 0;
	// Now read the file - search for the QSO start time
	while (all_file->good() && copy_status != COPIED) {
		std::string line;
		getline(*all_file, line);
		count += line.length() + 1;
		if (count <= file_size) status_->progress(count, OT_RECORD);

		// Does the line contain sought date, time, both calls and "Tx" or "Transmitting"
		if (line.substr(0,6) == datestamp) {
			if (line.substr(7,4) == timestamp.substr(0,4) || 
				copy_status != SEARCHING) {
				if (line.find(my_call_) != std::string::npos &&
					line.find(their_call) != std::string::npos &&
					line.find(mode) != std::string::npos) {
					if (copy_status == SEARCHING) {
						snprintf(msg, 256, "WSJTX: %s %s %s %s %s Found possible match",
							datestamp.c_str(),
							timestamp.c_str(),
							mode.c_str(),
							my_call_.c_str(),
							their_call.c_str());
						status_->misc_status(ST_WARNING, msg);
						copy_status = FOUND;;
					}
					snprintf(msg, sizeof(msg),"WSJTX: %s: %s", s_status[(int)copy_status].c_str(), line.c_str());
					status_->misc_status(ST_NOTE, msg);
				}
			}
		}
		if (copy_status != SEARCHING) {
			if (line.find(my_call_) != std::string::npos &&
				line.find(their_call) != std::string::npos &&
				line.find(mode) != std::string::npos) {
				if (update_qso) parse_all_txt(qso, line);
				if (qso->item("QSO_COMPLETE") == "N" || !update_qso) copy_status = COPYING;
				else if (copy_status == COPYING && 
					(qso->item("QSO_COMPLETE") == "Y" ||
					 qso->item("QSO_COMPLETE") == ""))
				    copy_status = COPIED;
			}
		}
	}
	if (copy_status == COPIED || (copy_status == COPYING && !update_qso)) {
		status_->progress("Found record!", OT_RECORD);
					snprintf(msg, 256, "WSJTX: %s %s %s %s %s Found match",
						datestamp.c_str(),
						timestamp.c_str(),
						mode.c_str(),
						my_call_.c_str(),
						their_call.c_str());
					status_->misc_status(ST_WARNING, msg);
		// If we are complete then say so
		if (qso->item("QSO_COMPLETE") != "N" && qso->item("QSO_COMPLETE") != "?") {
			all_file->close();
		}
		fl_cursor(FL_CURSOR_DEFAULT);
		return true;
	}
	else {
		all_file->close();
		snprintf(msg, 100, "WSJTX: Cannot find contact with %s in ALL.TXT", their_call.c_str());
		status_->misc_status(ST_WARNING, msg);
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}

}

// Called every 15s - check whether a heartbeat has been received from another app
void wsjtx_handler::ticker() {
	if (has_server() && connected_) {
		if (check_beats_ && received_beats_.empty()) {
			status_->misc_status(ST_WARNING, "WSJT-X: No heartbeats received.");
			check_beats_ = false;
		}
		if (!received_beats_.empty()) {
			send_hbeat();
		}
		received_beats_.clear();
	}
}

// Sattic interface
void wsjtx_handler::cb_ticker(void* v) {
	((wsjtx_handler*)v)->ticker();
}

// Erase cache entry for callsign
void wsjtx_handler::delete_qso(std::string call) {
	if (qsos_.find(call) != qsos_.end()) {
		qsos_.erase(call);
	} else {
		char msg[128];
		snprintf(msg, sizeof(msg),"WSJTX: Request to delete unknown record %s from cache", call.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

// Received data
bool wsjtx_handler::has_data() {
	return connected_;
}
