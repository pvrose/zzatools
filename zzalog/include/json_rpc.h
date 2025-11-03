#pragma once

#include <nlohmann/json.hpp>
#include <stdarg.h>
#include <sstream>
#include <string>

class socket_server;

using json = nlohmann::json;
using string = std::string;
using stringstream = std::stringstream;


//! \brief The class handles JSON-RPC requests.
//! It acts as a thin layer between the socket server and the main logic.
//! It handles the JSON-RPC protocol details and delegates request and response processing
class json_rpc
{
	public:
	//! Constructor.
	json_rpc(string address, int port_num);
	//! Destructor.
	~json_rpc();
	//! Callback to handle reques
	static int rcv_request(void* v, stringstream& ss);

	//! Send response
	bool send_response(int id, const json& jres);

	//! Send request
	bool send_request(int id, string method, const json& jparams);

	//! Send error response
	bool send_error(int id, int code, const char* message, ...);

	//! Set callbacks for request and response processing
	void set_callbacks(
		void* instance,
		bool(*do_request)(void* instance, int req_id, json& jreq),
		bool(*do_response)(void* instance, int resp_id, json& jres),
		bool(*do_error)(void* instance, int err_id, json& jerr)
	);

protected:
	//! handler for processing JSON-RPC requests.
	bool process_request(json& jreq);

	//! handler for processing JSON-RPC responses.
	bool process_response(json& jres);

	//! Callback instance pointer
	void* instance_;  //!< Instance pointer for callbacks
	//! Callback to handle requests
	bool (*do_request_)(void* instance, int req_id, json& jreq);  //!< Request handler callback
	//! Callback to handle responses
	bool (*do_response_)(void* instance, int resp_id, json& jres);  //!< Response handler callback
	//! Callback to handle errors
	bool (*do_error_)(void* instance, int err_id, json& jerr);  //!< Error handler callback

	socket_server* server_;  //!< Socket server instance
	
};

