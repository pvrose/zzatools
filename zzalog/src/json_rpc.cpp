#include "json_rpc.h"

#include "socket_server.h"
#include <nlohmann/json.hpp>
#include <sstream>
using string = std::string;
using stringstream = std::stringstream;
using json = nlohmann::json;
//! Constructor.
//! Sets up socket server to receive JSON-RPC requests.
json_rpc::json_rpc(string address, int port_num)
{
	server_ = new socket_server(socket_server::UDP, address, port_num);
	server_->callback(this,	rcv_request);
	instance_ = nullptr;
	do_request_ = nullptr;
	do_response_ = nullptr;
}

//! Destructor.
//! Cleans up socket server.
json_rpc::~json_rpc()
{
	server_->close_server(false);
	delete server_;
	server_ = nullptr;
}

//! Callback to handle requests.
//! \param v pointer to json_rpc instance.
//! \param ss the request stream.
//! \return 0 if request was handled OK, else error code.
//! This is a static method called by the socket server when a request is received.
//! It delegates to the process_request method of the json_rpc instance.
//! The request is expected to be in JSON-RPC format.
//! If the request is successfully processed, it returns 0, otherwise -1.
//! The method extracts the json_rpc instance from the void pointer and calls its process_request method.
//! If the instance pointer is null, it returns -1.
//! The request stream is passed as a stringstream reference.
//! If the process_request method returns true, it indicates successful handling of the request.
//! Otherwise, it indicates failure.
int json_rpc::rcv_request(void* v, stringstream& ss)
{
	json jreq;
	ss >> jreq;
	try {
		json_rpc* that = static_cast<json_rpc*>(v);
		if (that) {
			if (jreq.contains("method")) {
				if (that->process_request(jreq)) {
					return 0;
				}
			}
			else if (jreq.contains("result") || jreq.contains("error")) {
				if (that->process_response(jreq)) {
					return 0;
				}
			}
		}
	}
	catch (...) {
		return -1;
	}
	return -1;
}

//! Set callbacks for request and response processing	
//! \param instance Pointer to instance for callbacks.
//! \param do_request Callback to handle requests.
//! \param do_response Callback to handle responses.
//! This method sets the callback functions for processing requests and responses.
//! It also sets the instance pointer that will be passed to the callbacks.
//! The do_request callback is called to process incoming requests,
//! and the do_response callback is called to process outgoing responses.
//! The instance pointer is used to provide context to the callbacks.
void json_rpc::set_callbacks(
	void* instance,
	bool(*do_request)(void* instance, int id, json& jreq),
	bool(*do_response)(void* instance, int id, json& jres),
	bool(*do_error)(void* instance, int id, json& jerr)
)
{
	instance_ = instance;
	do_request_ = do_request;
	do_response_ = do_response;
	do_error_ = do_error;
	server_->run_server();
}

//! handler for processing JSON-RPC requests.
//! \param jreq the json request data.
//! \return true if request was handled.
//! This method processes incoming JSON-RPC requests.
//! It validates the request format and extracts the method and parameters.
//! If a valid method is found, it calls the do_request_ callback to handle the request.
//! If the request is successfully processed, it returns true.
//! If the request is invalid or cannot be processed, it returns false.
//! The method checks for the presence of required fields such as "jsonrpc", "id", and "params".
//! It also ensures that the "jsonrpc" version is "2.0".
//! If the request is valid, it delegates to the do_request_ callback for further processing.
//! If the callback is not set, it returns false.
//! The method uses exception handling to catch any errors during processing and returns false in such cases.
bool json_rpc::process_request(json& jreq)
{
	try {
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
		int req_id = jreq.at("id").get<int>();
		if (!jreq.contains("params")) {
			return false;
		}
		json jparams = jreq.at("params");
		if (!jparams.is_object()) {
			return false;
		}
		// Delegate to request handler callback
		if (do_request_) {
			return do_request_(instance_, req_id, jreq);
		} else {
			return false;
		}
	}
	catch (...) {
		return false;
	}
	return false;
}

//! handler for processing JSON-RPC responses.
//! \param jres the json response data.
//! \return true if response was handled.
//! This method processes incoming JSON-RPC responses.
//! It validates the response format and extracts the result or error.
//! If a valid response is found, it calls the do_response_ callback to handle the response.
//! If the response is successfully processed, it returns true.
//! If the response is invalid or cannot be processed, it returns false.
//! The method checks for the presence of required fields such as "jsonrpc" and "id".
//! It also ensures that the "jsonrpc" version is "2.0".
bool json_rpc::process_response(json& jres)
{
	try {
		// Basic validation of response.
		if (jres.is_null()) {
			return false;
		}
		// Check for jsonrpc version 2.0
		if (!jres.contains("jsonrpc")) {
			return false;
		}
		else if (jres["jsonrpc"] != "2.0") {
			return false;
		}
		if (!jres.contains("id")) {
			return false;
		}
		int resp_id = jres.at("id").get<int>();
		if (!jres.contains("result") && !jres.contains("error")) {
			return false;
		}
		if (jres.contains("error")) {
			json jerr = jres.at("error");
			// Handle error response
			if (do_error_) {
				return do_error_(instance_, resp_id, jerr);
			}
			else {
				return false;
			}
		}
		if (jres.contains("result")) {
			json jresult = jres.at("result");
			// Delegate to response handler callback
			if (do_response_) {
				return do_response_(instance_, resp_id, jres);
			}
			else {
				return false;
			}
		}
	}
	catch (...) {
		return false;
	}
	return false;
};

//! Send response
//! \param id the request identifier.
//! \param jres the json response data.
//! \return true if response was sent.
//! This method sends a JSON-RPC response.
bool json_rpc::send_response(int id, const json& jres)
{
	try {
		json j;
		j["jsonrpc"] = "2.0";
		j["id"] = id;
		j["result"] = jres; 
		stringstream ss;
		ss << j;
		server_->send_response(ss);
		return true;
	}
	catch (...) {
		return false;
	}
	return false;
}

//! Send request
//! \param id the request identifier.
//! \param jreq the json request data.
//! \return true if request was sent.
//! This method sends a JSON-RPC request.
//! It constructs the request in JSON-RPC format and sends it via the socket server.
bool json_rpc::send_request(int id, string method, const json& jparams)
{
	try {
		json j;
		j["jsonrpc"] = "2.0";
		j["id"] = id;
		j["method"] = method;
		j["params"] = jparams;	
		stringstream ss;
		ss << j;
		server_->send_response(ss);
		return true;
	}
	catch (...) {
		return false;
	}
	return false;
}

//! Send error response
//! \param id the request identifier.
//! \param code the error code.
//! \param message the error message format string.
//! \return true if error response was sent.
bool json_rpc::send_error(int id, int code, const char* message, ...)
{
	try {
		char emsg[1024];
		va_list args;
		va_start(args, message);
		vsnprintf(emsg, sizeof(emsg), message, args);
		va_end(args);
		json jerr;
		jerr["code"] = code;
		jerr["message"] = emsg;
		json j;
		j["jsonrpc"] = "2.0";
		j["id"] = id;
		j["error"] = jerr;
		stringstream ss;
		ss << j;
		server_->send_response(ss);
		return true;
	}
	catch (...) {
		return false;
	}
	return false;
}