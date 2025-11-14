#include "rpc_handler.h"

#include "main.h"
#include "regices.h"
#include "socket_server.h"
#include "status.h"
#include "url_handler.h"

#include "utils.h"

#include "pugixml.hpp"

#include <sstream>
#include <iostream>
#include <map>
#include <string>

#include <FL/fl_ask.H>

// Constructor
rpc_handler::rpc_handler(std::string address, int port_number, std::string resource_name)
{
	host_name_ = address;
	server_port_ = port_number;
	resource_ = resource_name;
	server_ = nullptr;
	method_list_.clear();
	add_method(this, { "system.listMethods", "s:s", "List of methods available" }, list_methods);
	add_method(this, { "system.methodHelp", "s:s", "Help text for method" }, method_help);
}

// Destructor
rpc_handler::~rpc_handler()
{
	close_server();
	for (auto ix = method_list_.begin(); ix != method_list_.end(); ix++) {
	}
	method_list_.clear();
}

// Close the RPC server
void rpc_handler::close_server() {
	if (server_) {
		server_->close_server(true);
		delete server_;
		server_ = nullptr;
	}
}

// Do request - response = method_name(params)
bool rpc_handler::do_request(
	std::string method_name,
	rpc_data_item::rpc_list* params,
	rpc_data_item* response
) {
	// The stream for sending the request
	std::stringstream put_request;
	// The stream for receiving the response
	std::stringstream put_response;
	// Generate XML for the request
	generate_request(method_name, params, put_request);
	// Post the request and get the response
	if (url_handler_->post_url(host_name_, resource_, &put_request, &put_response)) {
		// Successful - process response
		put_response.seekg(0, std::ios::beg);
		bool rpc_fault;
		decode_response(put_response, response, rpc_fault);
		if (rpc_fault) {
			return false;
		}
		else {
			return true;
		}
	}
	else {
		return false;
	}
}

// Generate XML for the request onto the selected output stream
bool rpc_handler::generate_request(
	std::string method_name,
	rpc_data_item::rpc_list* params,
	std::ostream& request_xml
) {
	// Generate the document
	pugi::xml_document doc;
	// Add the declaraion
	auto n_decl = doc.append_child(pugi::node_declaration);
	n_decl.append_attribute("version") = "1.0";

	// Top level node
	pugi::xml_node n_call = doc.append_child("methodCall");

	// Method name
	n_call.append_child("methodName").text().set(method_name);

	// Parameters
	pugi::xml_node n_params = n_call.append_child("params");
	for (auto param : *params) {
		pugi::xml_node n_param = n_params.append_child("param");
		write_item(*param, n_param);
	}

	// WRite XML to output stream
	doc.save(request_xml, " ");

	return true;
}

// Generate an RPC Respose
bool rpc_handler::generate_response(
	bool rpc_fault,
	rpc_data_item* response,
	std::ostream& response_xml) {
	// Generate the document
	pugi::xml_document doc;
	// Add the declaraion
	auto n_decl = doc.append_child(pugi::node_declaration);
	n_decl.append_attribute("version") = "1.0";

	// Add the response
	pugi::xml_node n_resp = doc.append_child("methodResponse");

	// <params> or <fault>
	if (rpc_fault) {
		pugi::xml_node n_fault = n_resp.append_child("fault");
		write_item(*response, n_fault);
	}
	else {
		// Parameters
		pugi::xml_node n_params = n_resp.append_child("params");
		pugi::xml_node n_param = n_params.append_child("param");
		write_item(*response, n_param);
	}
	// Write the response to the output stream
	doc.save(response_xml, " ");

	return true;
}

// Create XML for an individual item
void rpc_handler::write_item(rpc_data_item& item, pugi::xml_node& node) {
	rpc_data_t element_type = item.type();
	std::string text;
	pugi::xml_node n_value = node.append_child("value");

	switch (element_type) {
	case XRT_BOOLEAN:
		n_value.append_child("boolean").text().set(item.get_int() == 1);
		break;
	case XRT_INT:
		n_value.append_child("int").text().set(item.get_int());
		break;
	case XRT_DOUBLE:
		n_value.append_child("double").text().set(item.get_double());
		break;
	case XRT_STRING:
		n_value.append_child("string").text().set(item.get_string());
		break;
	case XRT_DATETIME:
		n_value.append_child("dateTime.iso8601").text().set(item.get_string());
		break;
	case XRT_BYTES:
		n_value.append_child("base64").text().set(encode_base_64(item.get_string()));
		break;
	case XRT_ARRAY: {
		pugi::xml_node n_array = n_value.append_child("array");
		pugi::xml_node n_data = n_array.append_child("data");
		for (auto datum : *item.get_array()) {
			write_item(*datum, n_data);
		}
		break;
	}
	case XRT_STRUCT: {
		pugi::xml_node n_struct = n_value.append_child("struct");
		for (auto member : *item.get_struct()) {
			pugi::xml_node n_member = n_struct.append_child("member");
			n_member.append_child("name").text().set(member.first);
			write_item(*member.second, n_member);
		}
		break;
	}
	default:
		break;
	}
	return;
}

// Decode the response on the input stream
bool rpc_handler::decode_response(std::istream& response_xml, rpc_data_item* response, bool& rpc_fault) {

	pugi::xml_document doc;
	doc.load(response_xml);

	pugi::xml_node n_resp = doc.document_element();

	if (strcmp(n_resp.name(), "methodResponse") != 0) {
		status_->misc_status(ST_ERROR, "RPC: Not a valid response");
		return false;
	}
	pugi::xml_node n_child = n_resp.first_child();
	if (strcmp(n_child.name(), "params")) {
		pugi::xml_node n_param = n_child.child("param");
		read_item(n_param, *response);
		rpc_fault = false;
	}
	else if (strcmp(n_child.name(), "fault")) {
		read_item(n_child, *response);
		rpc_fault = true;
	}
	else {
		status_->misc_status(ST_ERROR, "RPC: Not a valid response");
		return false;
	}
	return true;
}

// Decode the RPC Request XML on the input stream
bool rpc_handler::decode_request(std::istream& request_xml, std::string& method_name, rpc_data_item::rpc_list* params) {
	// parse the request
	pugi::xml_document doc;
	doc.load(request_xml);
	// Check it's a request
	pugi::xml_node n_req = doc.document_element();
	if (strcmp(n_req.name(), "methodCall") != 0) {
		status_->misc_status(ST_ERROR, "RPC: Not a valid request");
		return false;
	}
	// Get method call
	method_name = n_req.child("methodCall").text().as_string();
	// Get params
	pugi::xml_node n_params = n_req.child("params");
	for (auto n_param : n_params) {
		if (strcmp(n_param.name(), "param") != 0) {
			status_->misc_status(ST_ERROR, "RPC: Not a valid request");
			return false;
		}
		rpc_data_item* item = new rpc_data_item;
		read_item(n_param, *item);
		params->push_back(item);
	}
	return true;
}

void rpc_handler::read_item(pugi::xml_node& node, rpc_data_item& item) {
	pugi::xml_node n_value = node.child("value");
	pugi::xml_node n_item = n_value.first_child();
	const char* type = n_item.name();
	if (strcmp(type, "array") == 0) {
		rpc_data_item::rpc_array* array = new rpc_data_item::rpc_array;
		pugi::xml_node n_data = n_item.child("data");
		for (auto n_datum : n_data.children()) {
			rpc_data_item* datum = new rpc_data_item;
			read_item(n_datum, *datum);
			array->push_back(datum);
		}
		item.set(array);
	}
	else if (strcmp(type, "base64") == 0) {
		std::string s = decode_base_64(n_item.text().as_string());
		item.set(s, XRT_STRING);
	}
	else if (strcmp(type, "boolean") == 0) {
		bool b = n_item.text().as_bool();
		item.set(b);
	}
	else if (strcmp(type, "dateTime.iso8601") == 0) {
		std::string s = n_item.text().as_string();
		item.set(s, XRT_DATETIME);
	}
	else if (strcmp(type, "double") == 0) {
		double d = n_item.text().as_double();
		item.set(d);
	}
	else if (strcmp(type, "int") == 0 || strcmp(type, "i4") == 0) {
		uint32_t i = n_item.text().as_int();
		item.set(i);
	}
	else if (strcmp(type, "string") == 0) {
		std::string s = n_item.text().as_string();
		item.set(s, XRT_STRING);
	}
	else if (strcmp(type, "struct") == 0) {
		rpc_data_item::rpc_struct* str = new rpc_data_item::rpc_struct;
		for (auto member : n_item.children()) {
			if (strcmp(member.name(), "member") == 0) {
				std::string name = member.text().as_string();
				rpc_data_item* datum = new rpc_data_item;
				read_item(member, *datum);
				(*str)[name] = datum;
			}
		}
		item.set(str);
	}
}

// Run the HTTP server
void rpc_handler::run_server() {
	if (server_) {
		server_->run_server();
	}
	else {
		server_ = new socket_server(socket_server::HTTP, host_name_, server_port_);
		server_->callback(this, rcv_request);
		server_->run_server();
	}
}

// Static callback - calls the one in this class
int rpc_handler::rcv_request(void* instance, std::stringstream& ss) { 
	return ((rpc_handler*)instance)->handle_request(ss);
}

// Handle request - decode it, action it and send response
int rpc_handler::handle_request(std::stringstream& ss) {
	std::stringstream payload;
	if (strip_header(ss, payload)) {
		// Decode request
		std::string method_name = "";
		rpc_data_item::rpc_list params;
		rpc_data_item response;
		decode_request(payload, method_name, &params);
		int error;
		// Does method exist
		if (method_list_.find(method_name) == method_list_.end()) {
			error = 1;
		}
		else {
			// It does, so do it
			auto meth = method_list_.at(method_name);
			error = meth.callback(meth.v, params, response);
		}
		// Convert to XML
		std::stringstream xml;
		generate_response(error, &response, xml);
		// Add header
		std::stringstream resp;
		add_header(OK, xml, resp);
		// Send to server
		return server_->send_response(resp);
	}
	else {
		// Not a post or not to the RPC server supported - send "bad request" back to client
		std::stringstream error_response;
		std::stringstream dummy;
		dummy.str("");
		add_header(BAD_REQUEST, dummy, error_response);
		return server_->send_response(error_response);
	}
}

// Check and parse HTML header - returns start of payload
bool rpc_handler::strip_header(std::stringstream& message, std::stringstream& payload) {
	message.seekg(0, std::ios::beg);
	std::streampos start = message.tellg();
	std::string line;
	// First line - e.g. POST <resource>....
	getline(message, line);
	std::vector<std::string> words;
	split_line(line, words, ' ');
	if (words[0] != "POST" || words[1] != resource_) {
		return false;
	}
	else {
		// Find the payload length
		while (to_upper(words[0]) != "CONTENT-LENGTH:") {
			getline(message, line);
			split_line(line, words, ' ');
		}
		// Now go on to the payload indicated by an empty line
		while (line != "" && line != "\r") {
			getline(message, line);
		}
		// Now copy payload to output stream
		std::streampos pos_pl = message.tellg();
		std::string s = message.str();
		payload.str(s.substr((size_t)(pos_pl - start)));
		return true;
	}
}

// Add the apropriate header
bool rpc_handler::add_header(http_code code, std::stringstream& payload, std::stringstream& resp) {
	std::string pl = payload.str();
	int len_pl = pl.length();
	switch (code) {
	case OK:
		//  HTTP/1.1 200 OK
		//	Date : Sat, 06 Oct 2001 23:20:04 GMT
		//	Server : Apache.1.3.12 (Unix)
		//	Connection : close
		//	Content-Type : text/xml
		//	Content-Length : 124
		resp << "HTTP/1.1 " << code << " OK\r\n";
		resp << "Date: " << now(false, "%a %d %b %Y %X GMT") << "\r\n";
		resp << "Server: " << PROGRAM_ID << ". " << PROGRAM_VERSION << "\r\n";
		resp << "Content-Type: text/xml\r\n";
		resp << "Content-Length: " << len_pl << "\r\n";
		resp << "\r\n";
		resp << pl << "\r\n";
		break;
	case BAD_REQUEST:
		resp << "HTTP/1.1 " << code << " BAD REQUEST\r\n";
		resp << "Date: " << now(false, "%a %d %b %Y %X GMT") << "\r\n";
		resp << "Server: " << PROGRAM_ID << ". " << PROGRAM_VERSION << "\r\n";
		resp << "Connection: close\r\n";
		resp << "\r\n";
		break;
	}
	return true;
}

// Add server method
void rpc_handler::add_method(void* v, method_entry method, int(*callback)(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response)) {
	method_list_[method.name] = { method.signature, method.help_text, v, callback };
}

// system.listMethods
int rpc_handler::list_methods(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response) {
	rpc_handler* that = (rpc_handler*)v;
	rpc_data_item::rpc_array* array = new rpc_data_item::rpc_array;
	for (auto it = that->method_list_.begin(); it != that->method_list_.end(); it++) {
		rpc_data_item* name = new rpc_data_item;
		name->set(it->first, XRT_STRING);
		array->push_back(name);
	}
	response.set(array);
	return 0;
}

// system.MethodHelp
int rpc_handler::method_help(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response) {
	rpc_handler* that = (rpc_handler*)v;
	if (params.size() == 1) {
		rpc_data_item* item_0 = params.front();
		std::string method_name = item_0->get_string();
		if (that->method_list_.find(method_name) != that->method_list_.end()) {
			std::string help_text = that->method_list_.at(method_name).help_text;
			response.set(help_text, XRT_STRING);
			return 0;
		}
		that->generate_error(-1, "Unknown method name", response);
		return 1;
	}
	that->generate_error(-2, "Invalid number of paarmeters", response);
	return 1;
}

// Generate an error item for RPC response
void rpc_handler::generate_error(int code, std::string message, rpc_data_item & response) {
	rpc_data_item error_code;
	error_code.set(-2, XRT_INT);
	rpc_data_item error_msg;
	error_msg.set("Incorrect number of parameters", XRT_STRING);
	rpc_data_item::rpc_struct fault_resp = { { "code", &error_code }, { "message", &error_msg } };
	response.set(&fault_resp);
}

// Returns the state of the server
bool rpc_handler::has_server() {
	if (server_) {
		return server_->has_server();
	} else {
		return false;
	}
}