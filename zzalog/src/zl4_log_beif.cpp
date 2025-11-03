#include "zl4_log_beif.h"

#include "json_rpc.h"
#include "record.h"
#include "settings.h"
#include "zl4_log_data.h"

extern zl4_log_data* zl4_log_data_;

//! Constructor.
//! Sets up UDP socket server to receive log requests.
zl4_log_beif::zl4_log_beif()
{
	load_values();
	server_ = new json_rpc(address_, port_);
	server_->set_callbacks(
		this,
		rcv_request,
		nullptr,
		nullptr
	);
}

//! Destructor.
//! Saves configuration data.
//! 
zl4_log_beif::~zl4_log_beif()
{
	delete server_;
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
bool zl4_log_beif::rcv_request(void* v, int id, json& jreq)
{
	zl4_log_beif* that = static_cast<zl4_log_beif*>(v);
	if (that) {
		return that->process_request(id, jreq);
	}
	return false;
}

//! handler for processing log requests from frontend.
//! \param id the request identifier.
//! \param jreq the json request data.
//! \return true if request was handled.
//! This method dispatches the request to the appropriate handler based on the "method" field in the JSON request.
bool zl4_log_beif::process_request(int id, json& jreq)
{
	if (!jreq.contains("method")) {
		return false;
	}
	string method = jreq.at("method").get<string>();
	if (method == "get") {
		return handle_get(id, jreq);
	}
	else if (method == "get_next") {
		return handle_get_next(id, jreq);
	}
	else if (method == "put") {
		return handle_put(id, jreq);
	}
	else if (method == "erase") {
		return handle_erase(id, jreq);
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
	record* rec = zl4_log_data_->get(q_id);
	if (rec) {
		// Result is { qso_id, record }
		json jrec;
		jrec["record"] = json(*rec);
		jrec["qso_id"] = q_id;
		server_->send_response(req_id, jrec);
		return true;
	}
	else {
		// Prepare error response.
		server_->send_error(req_id, -32001, "QSO ID %u not found", q_id);
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
	record* rec = zl4_log_data_->get_next(q_id, criteria);
	if (rec) {
		// Prepare response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		server_->send_response(req_id, jres);
		return true;
	}
	else {
		server_->send_error(req_id, -32001, "No matching QSO found after ID %u", q_id);
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
	if (zl4_log_data_->put(q_id, rec)) {
		// Prepare response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		jres["result"]["qso id"] = q_id;
		server_->send_response(req_id, jres);
		return true;
	}
	else {
		server_->send_error(req_id, -32002, "Failed to put QSO ID %u", q_id);
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
	if (zl4_log_data_->erase(q_id)) {
		// Prepare response.
		json jres;
		jres["jsonrpc"] = "2.0";
		jres["id"] = req_id;
		jres["result"]["qso id"] = q_id;
		server_->send_response(req_id, jres);
		return true;
	}
	else {
		server_->send_error(req_id, -32003, "Failed to erase QSO ID %u", q_id);
		return true;
	}
}

