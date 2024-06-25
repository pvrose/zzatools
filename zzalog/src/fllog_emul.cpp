#include "fllog_emul.h"
#include "book.h"
#include "extract_data.h"
#include "status.h"
#include "adi_writer.h"
#include "adi_reader.h"
#include "spec_data.h"
#include "rpc_data_item.h"
#include "qso_manager.h"

#include <sstream>

#include <FL/Fl_Preferences.H>


extern rpc_handler* rpc_handler_;
extern status* status_;
extern fllog_emul* fllog_emul_;
extern extract_data* extract_records_;
extern book* book_;
extern spec_data* spec_data_;
extern qso_manager* qso_manager_;
extern Fl_Preferences* settings_;

// Static - only one instance of this class supported
fllog_emul* fllog_emul::that_ = nullptr;

// Constructor
fllog_emul::fllog_emul() {
	rpc_handler_ = nullptr;
	current_qso_ = nullptr;
	putative_qso_ = nullptr;
	that_ = this;
	connected_ = false;
}

// DEstructor
fllog_emul::~fllog_emul() {
	// Disconnect port
	close_server();
}

// Start and run the RPC Server
void fllog_emul::run_server() {
	status_->misc_status(ST_NOTE, "FLLOG_EMUL: Creating new socket");
	if (!rpc_handler_) {
		Fl_Preferences nw_settings(settings_, "Network");
		Fl_Preferences fllog_settings(nw_settings, "Fllog");
		int rpc_port = 8421;
		fllog_settings.get("Port Number", rpc_port, rpc_port);
		char* addr;
		fllog_settings.get("Address", addr, "127.0.0.1");
		rpc_handler_ = new rpc_handler(string(addr), rpc_port, "/RPC2");
	}
	// Set up callbacks to handle these
	method_list_.push_back({ "log.add_record", "s:s", "adds new ADIF-RECORD" });
	rpc_handler_->add_method(method_list_.back(), add_record);
	method_list_.push_back({ "log.get_record",    "s:s", "returns ADIF-RECORD for CALL" });
	rpc_handler_->add_method(method_list_.back(), get_record);
	method_list_.push_back({ "log.update_record", "s:s", "updates current record with specified ADIF-RECORD" });
	rpc_handler_->add_method(method_list_.back(), update_record);
	method_list_.push_back({ "log.check_dup",     "s:s", "return true/false/possible for ADIF record" });
	rpc_handler_->add_method(method_list_.back(), check_dup);
	method_list_.push_back({ "log.list_methods",  "s:s", "return this list" });
	rpc_handler_->add_method(method_list_.back(), list_methods);

	rpc_handler_->run_server();
}

// Close the RPC server
void fllog_emul::close_server() {
	if (rpc_handler_) {
		status_->misc_status(ST_NOTE, "FLLOG_EMUL: Closing server");
		rpc_handler_->close_server();
		delete rpc_handler_;
		rpc_handler_ = nullptr;
	}
}

// Generate an XML-RPC error response
void fllog_emul::generate_error(int code, string message, rpc_data_item& response) {
	rpc_data_item error_code;
	error_code.set(code, XRT_INT);
	rpc_data_item error_msg;
	error_msg.set(message, XRT_STRING);
	rpc_data_item::rpc_struct fault_resp = { { "code", &error_code }, { "message", &error_msg } };
	response.set(&fault_resp);
}

// Get ADIF string for first record with callsign - also displays all matching records in extract window
int fllog_emul::get_record(rpc_data_item::rpc_list& params, rpc_data_item& response) {
	that_->check_connected();
	if (params.size() == 1) {
		rpc_data_item* item_0 = params.front();
		string callsign = item_0->get_string();
		record* qso = nullptr;
		if (that_->current_qso_ && that_->current_qso_->item("CALL") == callsign) {
			// Use this 
			printf("DEBUG: Using previous get_record\n");
			qso = that_->current_qso_;
		} else {
			if (extract_records_->size() == 0 || extract_records_->get_record(0, true)->item("CALL") != callsign) 
				extract_records_->extract_call(callsign);
			printf("DEBUG: get_record %s %zu records found\n", callsign.c_str(), extract_records_->size());
			if (extract_records_->size()) {
				qso = extract_records_->get_record(0, true);
				extract_records_->selection(0, HT_SELECTED);
				that_->current_qso_ = qso;
			}
		}
		if (qso) {
			adi_writer* writer = new adi_writer;
			stringstream ss;
			writer->to_adif(qso, ss);
			printf("DEBUG: get_record: %s\n", ss.str().c_str());
			response.set(ss.str(), XRT_STRING);
			delete writer;
		} else {
			response.set("NO RECORD", XRT_STRING);
		}
		// Create an entry for this
		qso = qso_manager_->start_modem_qso(callsign, qso_data::QSO_COPY_FLDIGI);
		qso->item("CALL", callsign);
		qso_manager_->update_modem_qso(false);
		that_->putative_qso_ = qso;
		return 0;
	}
	else {
		that_->generate_error(-2, "Wrong number of parameters in call", response);
		return 1;
	}
}

// Check duplicate - replies true (exact match), possible (callsign matches), false (not a match
int fllog_emul::check_dup(rpc_data_item::rpc_list& params, rpc_data_item& response) {
	that_->check_connected();
	if (params.size() == 6) {
		// Get parametrs
		rpc_data_item* i_call = params.front();
		string callsign = i_call->get_string();
		params.pop_front();
		rpc_data_item* i_mode = params.front();
		string mode = i_mode->get_string();
		params.pop_front();
		rpc_data_item* i_span = params.front();
		long span = atol(i_span->get_string().c_str());
		params.pop_front();
		// Frequency received in kHz
		rpc_data_item* i_freq = params.front();
		long freq_kHz = atol(i_freq->get_string().c_str());
		double freq_MHz = (double)freq_kHz / 1000.0;
		params.pop_front();
		rpc_data_item* i_state = params.front();
		string state = i_state->get_string();
		params.pop_front();
		rpc_data_item* i_rst_in = params.front();
		string rst_in = i_rst_in->get_string();
		// Get all possible matches
		if (extract_records_->size() == 0 || extract_records_->get_record(0, true)->item("CALL") != callsign) 
			extract_records_->extract_call(callsign);
		printf("DEBUG: check_dup %s Mode=%s Span=%d Freq=%d State=%s RST=%s", 
		callsign.c_str(), mode.c_str(), span, freq_kHz, state.c_str(), rst_in.c_str());
		time_t timestamp = time(nullptr);
		if (extract_records_->size() && extract_records_->get_record(0, true) != that_->current_qso_) {
			bool found = false;
			item_num_t item_num;
			item_num_t found_item;
			for (item_num = 0; item_num < extract_records_->size() && !found; item_num++) {
				record* qso = extract_records_->get_record(item_num, false);
				found = true;
				found_item = item_num;
				// Now check for exact match 
				if (mode != "0" && qso->item("MODE", true) != to_upper(mode)) {
					// different mode (note this includes submode)
					found = false;
				}
				else if (span > 0 && difftime(timestamp, qso->timestamp(true)) > (span * 60)) {
					// More that span minutes ago
					found = false;
				}
				else if (freq_MHz > 0) {
					// Different frequency - need to check if this is exact frequency
					string band = qso->item("BAND");
					if (spec_data_->band_for_freq(freq_MHz) != band) {
						found = false;
					}
				}
				else if (state != "0" && qso->item("STATE") != to_upper(state)) {
					// Different state
					found = false;
				}
				else if (rst_in != "0" && qso->item("RST_RCVD") != to_upper(rst_in)) {
					// Different RST
					found = false;
				}
			}
			if (found) {
				// Exact match - set selection
				printf(" Exact match\n");
				extract_records_->selection(found_item, HT_SELECTED);
				response.set("true", XRT_STRING);
			}
			else {
				// Callsign matches - select the first one
				printf(" Callsign match\n");
				extract_records_->selection(0, HT_SELECTED);
				response.set("possible", XRT_STRING);
			}
		}
		else {
			// Not a match
			printf(" No match\n");
			response.set("false", XRT_STRING);
		}
		return 0;
	}
	else {
		that_->generate_error(-2, "Wrong number of parameters in call", response);
		return 1;
	}
}

// Add new record
int fllog_emul::add_record(rpc_data_item::rpc_list& params, rpc_data_item& response) {
	that_->check_connected();
	if (params.size() == 1) {
		// Clear down any look up from fldigi
		extract_records_->clear_criteria();
		// Convert the adif data into a new record - use existing code from adi_reader
		rpc_data_item* item = params.front();
		stringstream ss;
		ss.str(item->get_string());
		printf("DEBUG: add_record: %s\n", item->get_string().c_str());
		adi_reader* reader = new adi_reader();
		load_result_t dummy;
		record* qso = that_->putative_qso_;
		reader->load_record(qso, ss, dummy);
		// Frig - fldigi sets MY_STATE incorrectly, and only uses MODE
		qso->item("MY_STATE", string(""));
		qso->item("MODE", qso->item("MODE"), true);
		qso_manager_->update_modem_qso(true);
		status_->misc_status(ST_NOTE, "FLLOG_EMUL: Logged QSO");
		return 0;
	}
	else {
		that_->generate_error(-2, "Wrong number of parameters in call", response);
		return 1;
	}
}

// Update fields in current selection
int fllog_emul::update_record(rpc_data_item::rpc_list& params, rpc_data_item& response) {
	that_->check_connected();
	if (params.size() == 1) {
		// Convert the adif data into a new record - use existing code from adi_reader
		rpc_data_item* item = params.front();
		stringstream ss;
		ss.str(item->get_string());
		printf("DEBUG: update_record: %s\n", item->get_string().c_str());
		adi_reader* reader = new adi_reader();
		load_result_t dummy;
		record* changes = new record();
		reader->load_record(changes, ss, dummy);
		that_->current_qso_->merge_records(changes);
		return 0;
	}
	else {
		that_->generate_error(-2, "Wrong number of parameters in call", response);
		return 1;
	}

}

// List methods - string returns list of methods suppported
int fllog_emul::list_methods(rpc_data_item::rpc_list& params, rpc_data_item& response) {
	that_->check_connected();
	if (params.size() == 0) {
		// Copied nearly verbatim from fllog
		printf("list_methods\n");
		rpc_data_item::rpc_array* array = new rpc_data_item::rpc_array;
		for (auto it = that_->that_->method_list_.begin(); it != that_->method_list_.end(); it++) {
			rpc_data_item::rpc_struct* method_data = new rpc_data_item::rpc_struct;
			method_data->clear();
			rpc_data_item* name = new rpc_data_item;
			name->set(it->name, XRT_STRING);
			(*method_data)["name"] = name;
			rpc_data_item* sig = new rpc_data_item;
			sig->set(it->signature, XRT_STRING);
			(*method_data)["signature"] = sig;
			rpc_data_item* help = new rpc_data_item;
			help->set(it->help_text, XRT_STRING);
			(*method_data)["help"] = help;
			rpc_data_item* method_obj = new rpc_data_item;
			method_obj->set(method_data);
			array->push_back(method_obj);
		}
		response.set(array);
		return 0;
	}
	else {
		that_->generate_error(-2, "Wrong number of parameters in call", response);
		return 1;
	}

}

// Sets connected flag and redraws dashboard
void fllog_emul::check_connected() {
	if (!connected_) {
		connected_ = true;
		qso_manager_->enable_widgets();
	}
}

// Returns connected state
bool fllog_emul::has_data() {
	return connected_;
}

// server state
bool fllog_emul::has_server() {
	if (rpc_handler_) {
		return rpc_handler_->has_server();
	} else {
		return false;
	}
}
