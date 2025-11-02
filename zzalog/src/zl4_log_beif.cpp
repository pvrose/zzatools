#include "zl4_log_beif.h"

#include "record.h"
#include "settings.h"
#include "socket_server.h"
#include "zl4_logdata.h"

extern zl4_logdata* zl4_logdata_;

//! Constructor.
//! Sets up UDP socket server to receive log requests.
zl4_log_beif::zl4_log_beif()
{
	load_values();
	server_ = new socket_server(socket_server::UDP, address_, static_cast<int>(port_));
	server_->callback(this, zl4_log_beif::rcv_request);
	server_->run_server();
}

//! Destructor.
//! Saves configuration data.
//! 
zl4_log_beif::~zl4_log_beif()
{
	server_->close_server(false);
	save_values();
}

//! Load configuration data from settings.
void zl4_log_beif::load_values()
{
	settings s;
	settings if_s(&s, "Interface");
	if_s.get<std::string>("Log Server Address", address_, "224.0.0.127");
	if_s.get<uint16_t>("Log Server Port", port_, 34567);
}

//! Save configuration data to settings.
void zl4_log_beif::save_values()
{
	settings s;
	settings if_s(&s, "Interface");
	if_s.set<std::string>("Log Server Address", address_);
	if_s.set<uint16_t>("Log Server Port", port_);
}

//! Callback to handle frontend requests to update backend data.
//! \param v pointer to zl4_log_beif instance.
//! \param ss the request stream.
//! \return 0 if request was handled OK, else error code.
int zl4_log_beif::rcv_request(void* v, stringstream& ss)
{
	zl4_log_beif* that = static_cast<zl4_log_beif*>(v);
	if (that) {
		if (that->process_request(ss)) {
			return 0;
		}
	}
	return -1;
}

//! handler for processing log requests from frontend.
//! \param ss the request stream.
//! \return true if request was handled.

bool zl4_log_beif::process_request(stringstream& ss)
{
	try {
		json jreq;
		ss >> jreq;
		// Basic validation of request.
		if (jreq.is_null()) {
			return false;
		} 
		// Check for jsonrpc version 2.0
		if (!jreq.contains("jsonrpc")) {
			return false;
		} else if (jreq["jsonrpc"] != "2.0") {
			return false;
		}
		if (!jreq.contains("id")) {
			return false;
		}
		int id = jreq.at("id").get<int>();
		if (!jreq.contains("params")) {
			return false;
		}
		json jparams = jreq.at("params");
		if (!jparams.is_object()) {
			return false;
		}
		// Determine request type.
		if (!jparams.contains("method")) {
			return false;
		}
		string method = jparams.at("method").get<std::string>();
		if (method == "get") {
			return handle_get(id, jparams);
		} else if (method == "get_next") {
			return handle_get_next(id, jparams);
		} else if (method == "put") {
			return handle_put(id, jparams);
		} else if (method == "erase") {
			return handle_erase(id, jparams);
		} else {
			return false;
		}
	}
	catch (...) {
		return false;
	}
	return false;
}

//! handler for "get" requests.
//! \param id the request identifier.
//! \param jreq the json request data.
//! \return true if request was handled.
bool zl4_log_beif::handle_get(int id, const json& jreq)
{
	int req_id = id;
	if (!jreq.contains("qso id")) {
		return false;
	}
	qso_id q_id = jreq.at("qso id").get<int>();
	record* rec = zl4_logdata_->get(q_id);
	if (rec) {
		// Prepare response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		// Result is { qso_id, record }
		json jrec;
		jrec["record"] = json(*rec);
		jrec["qso_id"] = q_id;
		jres["result"] = jrec;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
	else {
		// Prepare error response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		json jerr;
		jerr["code"] = -32001;
		char emsg[64];
		snprintf(emsg, sizeof(emsg), "QSO ID %u not found", q_id);
		jerr["message"] = emsg;
		jres["error"] = jerr;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
}

//! handler for "get_next" requests.
//! \param id the request identifier.
//! \param jreq the json request data.
//! \return true if request was handled.
bool zl4_log_beif::handle_get_next(int id, const json& jreq)
{
	int req_id = id;
	if (!jreq.contains("qso id")) {
		return false;
	}
	qso_id q_id = jreq.at("qso id").get<int>();
	if (!jreq.contains("criteria")) {
		return false;
	}
	search_criteria_t criteria = jreq.at("criteria").get<search_criteria_t>();
	record* rec = zl4_logdata_->get_next(q_id, criteria);
	if (rec) {
		// Prepare response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		// Result is { qso_id, record }
		json jrec;
		jrec["record"] = json(*rec);
		jrec["qso id"] = q_id;
		jres["result"] = jrec;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
	else {
		// Prepare error response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		json jerr;
		jerr["code"] = -32001;
		char emsg[64];
		snprintf(emsg, sizeof(emsg), "No matching QSO found after ID %u", q_id);
		jerr["message"] = emsg;
		jres["error"] = jerr;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
}

//! handler for "put" requests
bool zl4_log_beif::handle_put(int id, const json& jreq)
{
	int req_id = id;
	if (!jreq.contains("record")) {
		return false;
	}
	record rec = jreq.at("record").get<record>();
	if (!jreq.contains("qso id")) {
		return false;
	}
	qso_id q_id = jreq.at("qso id").get<int>();
	if (zl4_logdata_->put(q_id, rec)) {
		// Prepare response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		jres["result"]["qso id"] = q_id;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
	else {
		// Prepare error response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		json jerr;
		jerr["code"] = -32002;
		char emsg[64];
		snprintf(emsg, sizeof(emsg), "Failed to put QSO ID %u", q_id);
		jerr["message"] = emsg;
		jres["error"] = jerr;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
}

//! handler for "erase" requests
bool zl4_log_beif::handle_erase(int id, const json& jreq)
{
	int req_id = id;
	if (!jreq.contains("qso id")) {
		return false;
	}
	qso_id q_id = jreq.at("qso id").get<int>();
	if (zl4_logdata_->erase(q_id)) {
		// Prepare response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		jres["result"]["qso id"] = q_id;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
	else {
		// Prepare error response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		json jerr;
		jerr["code"] = -32003;
		char emsg[64];
		snprintf(emsg, sizeof(emsg), "Failed to erase QSO ID %u", q_id);
		jerr["message"] = emsg;
		jres["error"] = jerr;
		stringstream ss;
		ss << jres;
		server_->send_response(ss);
		return true;
	}
}