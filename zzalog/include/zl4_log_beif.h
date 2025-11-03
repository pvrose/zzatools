#pragma once

#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

class json_rpc;

using json = nlohmann::json;
using string = std::string;
using stringstream = std::stringstream;

//! \brief The class handles the backend of the logging interface.
class zl4_log_beif
{
public:
	//! Constructor.
	zl4_log_beif();
	//! Destructor.
	~zl4_log_beif();

	//! Load configuration data from settings.
	void load_values();
	//! Save configuration data to settings.
	void save_values();

	//! Callback to handle frontend requests to update backend data.
	static bool rcv_request(void* v, int id, json& jreq);

protected:
	//! handler for processing log requests from frontend.
	bool process_request(int id, json& jreq);

	//! handler for "get" requests.
	
	//! \param id the request identifier.
	//! \param jreq the json request data.
	//! \return true if request was handled.
	bool handle_get(int id, const json& jreq);

	//! handler for "get_next" requests.

	//! \param id the request identifier.
	//! \param jreq the json request data.
	//! \return true if request was handled.
	bool handle_get_next(int id, const json& jreq);

	//! handler for "put" requests

	//! \param id the request identifier.
	//! \param jreq the json request data.
	//! \return true if request was handled.
	bool handle_put(int id, const json& jreq);

	//! handler for "erase" requests

	//! \param id the request identifier.
	//! \param jreq the json request data.
	//! \return true if request was handled.
	bool handle_erase(int id, const json& jreq);

	string address_; //!< Address of the logging server.
	uint16_t port_;    //!< Port of the logging server.

	json_rpc* server_; //!< JSON-RPC protocol layer instance.
};

