#include "rpc_handler.h"
#include "url_handler.h"
#include "xml_writer.h"
#include "xml_reader.h"
#include "utils.h"
#include "status.h"

#include <sstream>

#include <FL/fl_ask.H>

using namespace std;
using namespace zzalog;

extern url_handler* url_handler_;
extern status* status_;

// Constructor
rpc_handler::rpc_handler(string host_name, int port_num, string resource_name)
{
	host_name_ = host_name + ':' + to_string(port_num);
	resource_ = resource_name;
}

// Destructor
rpc_handler::~rpc_handler()
{
}

// Do request - response = method_name(params)
bool rpc_handler::do_request(
	string method_name,
	rpc_data_item::rpc_list* params,
	rpc_data_item* response
) {
	// The stream for sending the request
	stringstream put_request;
	// The stream for receiving the response
	stringstream put_response;
	// Generate XML for the request
	generate_request(method_name, params, put_request);
	// Post the request and get the response
	if (url_handler_->post_url(host_name_, resource_, &put_request, &put_response)) {
		// Successful - process response
		put_response.seekg(0, ios::beg);
		bool rpc_fault;
		decode_response(put_response, response, rpc_fault);
		if (rpc_fault) {
			// Display reeceived response
			string error = response->print_item();
			char* message = new char[error.length() + 100];
			sprintf(message, "XML_RPC: Received RPC fault:\n%s", error.c_str());
			status_->misc_status(ST_ERROR, message);
			delete[] message;
			return false;
		}
		else {
			return true;
		}
	}
	else {
		status_->misc_status(ST_ERROR, "XML_RPC: HTTP post failed.");
		return false;
	}
}

// Generate XML for the request onto the selected output stream
bool rpc_handler::generate_request(
	string method_name,
	rpc_data_item::rpc_list* params,
	ostream& request_xml
) {
	// The XML writer to generate XML
	xml_writer* writer = new xml_writer;

	// Start writing XML - 
	bool xml_ok = writer->process_instr("xml", "version = \"1.0\"");
	// <methodCall>
	xml_ok &= writer->start_element("methodCall", nullptr);

	// Method name - <methodName>NAME</methodName>
	xml_ok &= writer->start_element("methodName", nullptr);
	xml_ok &= writer->characters(method_name);
	xml_ok &= writer->end_element("methodName");

	// Parameters - <params>
	xml_ok &= writer->start_element("params", nullptr);

	// Individual parameters
	if (params != nullptr) {
		// For each parameter
		for (auto it = params->begin(); it != params->end() && xml_ok; it++) {
			// <param>ITEM</param>
			xml_ok &= writer->start_element("param", nullptr);
			xml_ok &= write_item(writer, *it);
			xml_ok &= writer->end_element("param");
		}
	}

	// End enclosing elements </params></methodCall>
	xml_ok &= writer->end_element("params");
	xml_ok &= writer->end_element("methodCall");

	// Now write the destination to the output stream.
	xml_ok &= writer->data(request_xml);

	delete writer;

	return xml_ok;
}

// Generate an RPC Respose
bool rpc_handler::generate_response(
	bool rpc_fault,
	rpc_data_item* response,
	ostream& response_xml) {

	xml_writer* writer = new xml_writer;

	// Start writing XML
	bool xml_ok = writer->process_instr("xml", "version = \"1.0\"");
	// <methodResponse>
	xml_ok &= writer->start_element("methodResponse", nullptr);
	// <params> or <fault>
	if (rpc_fault) {
		// <fault>ITEM</fault>
		xml_ok &= writer->start_element("fault", nullptr);
		xml_ok &= write_item(writer, response);
		xml_ok &= writer->end_element("fault");
	}
	else {
		// <params>
		xml_ok &= writer->start_element("params", nullptr);
		// <param>ITEM</param>
		xml_ok &= writer->start_element("param", nullptr);
		xml_ok &= write_item(writer, response);
		xml_ok &= writer->end_element("param");
		// </params>
		xml_ok &= writer->end_element("params");
	}
	// Write the response to the output stream
	xml_ok &= writer->data(response_xml);

	delete writer;

	return xml_ok;
}

// Create XML for an individual item
bool rpc_handler::write_item(xml_writer* writer, rpc_data_item* item) {
	rpc_data_t element_type = item->type();
	string text;

	// <value>
	bool xml_ok = writer->start_element("value", nullptr);

	switch (element_type) {
	case XRT_BOOLEAN:
		text = item->get_int() == 0 ? "1" : "0";
		// <Boolean>0/1</Boolean
		xml_ok &= writer->start_element("Boolean", nullptr);
		xml_ok &= writer->characters(text);
		xml_ok &= writer->end_element("Boolean");
		break;
	case XRT_INT:
		text = to_string(item->get_int());
		// <int>n</int>
		xml_ok &= writer->start_element("int", nullptr);
		xml_ok &= writer->characters(text);
		xml_ok &= writer->end_element("int");
		break;
	case XRT_DOUBLE:
		text = to_string(item->get_double());
		// <double>n.nn</double>
		xml_ok &= writer->start_element("double", nullptr);
		xml_ok &= writer->characters(text);
		xml_ok &= writer->end_element("double");
		break;
	case XRT_STRING:
		text = item->get_string();
		// <string>text</string>
		xml_ok &= writer->start_element("string", nullptr);
		xml_ok &= writer->characters(text);
		xml_ok &= writer->end_element("string");
		break;
	case XRT_DATETIME:
		text = item->get_string();
		// <dateTime.iso8601>text</dateTime.iso8601>
		xml_ok &= writer->start_element("dateTime.iso8601", nullptr);
		xml_ok &= writer->characters(text);
		xml_ok &= writer->end_element("dateTime.iso8601");
		break;
	case XRT_BYTES:
		text = encode_base_64(item->get_string());
		// <base64>n.</base64>
		xml_ok &= writer->start_element("base64", nullptr);
		xml_ok &= writer->characters(text);
		xml_ok &= writer->end_element("base64");
		break;
	case XRT_ARRAY:
		// <array><data>items</data></array
		xml_ok &= writer->start_element("array", nullptr);
		xml_ok &= writer->start_element("data", nullptr);
		for (size_t i = 0; i < item->get_array()->size() && xml_ok; i++) {
			xml_ok &= write_item(writer, item->get_array()->at(i));
		}
		xml_ok &= writer->end_element("data");
		xml_ok &= writer->end_element("array");
		break;
	case XRT_STRUCT:
		// <struct><member>[<name>...</name>item]</member></struct>
		xml_ok &= writer->start_element("struct", nullptr);
		rpc_data_item::rpc_struct* struct_item = item->get_struct();
		for (auto it = struct_item->begin(); it != struct_item->end() && xml_ok; it++) {
			xml_ok &= writer->start_element("member", nullptr);
			xml_ok &= writer->start_element("name", nullptr);
			xml_ok &= writer->characters(it->first);
			xml_ok &= writer->end_element("name");
			xml_ok &= write_item(writer, it->second);
			xml_ok &= writer->end_element("member");
		}
		xml_ok &= writer->end_element("struct");
	}
	return xml_ok;
}

// Decode the response on the input stream
bool rpc_handler::decode_response(istream& response_xml, rpc_data_item* response, bool& rpc_fault) {
	xml_reader* reader = new xml_reader;
	// Send the input stream to the XML parser
	reader->parse(response_xml);
	// Get the outer XML: element - it should be <methodResponse>
	xml_element* top_element = reader->element();
	if (top_element == nullptr || to_upper(top_element->name()) != "METHODRESPONSE") {
		// Bad or incorrect XML received
		status_->misc_status(ST_ERROR, "XML_RPC: XML reader has not decoded XML or top-level != methodResponse");
		delete reader;
		return false;
	}
	else {
		// Decode XML starting at outer XML element and iterate down
		bool xml_ok = decode_xml_element(XRP_METHODRESPONSE, top_element, response, rpc_fault);
		delete reader;
		return xml_ok;
	}
}

// Decode the RPC Request XML on the input stream
bool rpc_handler::decode_request(istream& request_xml, string& method_name, rpc_data_item::rpc_list* params) {
	xml_reader* reader = new xml_reader;
	// Send the input stream to the XML parser
	reader->parse(request_xml);
	// Get the outer XML element - it should be <methodCall>
	xml_element* top_element = reader->element();
	if (top_element == nullptr || to_upper(top_element->name()) != "METHODCALL") {
		// bad or incorrect XML received
		status_->misc_status(ST_ERROR, "XML_RPC: XML reader has not decoded XML or top-level != methodCall");
		delete reader;
		return false;
	}
	else {
		delete reader;
		// Decode XML starting at outer XML element and iterate down
		return decode_xml_element(XRP_METHODCALL, top_element, method_name, params);
	}
}

// Decode the individual XML element for more than element expected
bool rpc_handler::decode_xml_element(rpc_element_t element_type, xml_element* element, string& method_name, rpc_data_item::rpc_list* items) {
	xml_element* child_element;
	string child_name;
	string upper_name;
	bool xml_ok = true;
	string error_message = "";
	bool dummy = false;
	switch (element_type) {
	case XRP_METHODCALL:
		// Expect two elements <methodName> and <params>
		if (element->count() == 2) {
			// For each element
			for (int i = 0; i < 2 && xml_ok; i++) {
				// Get the element
				child_element = element->child(i);
				child_name = child_element->name();
				upper_name = to_upper(child_name);
				// Element is <methodName>
				if (upper_name == "METHODNAME") {
					// Get the element
					rpc_data_item* name_item = new rpc_data_item;
					xml_ok = decode_xml_element(XRP_METHODNAME, child_element, name_item, dummy);
					if (xml_ok) {
						method_name = name_item->get_string();
					}
					else {
						method_name = "";
						error_message = "Failed to decode request method name";
					}
				}
				else if (upper_name == "PARAMS") {
					// Get the element
					string dummy_string;
					xml_ok = decode_xml_element(XRP_PARAMS, child_element, dummy_string, items);
				}
				else {
					xml_ok = false;
					error_message = "Expected <methodName> and <params> - got " + child_name;
				}
			}
		}
		else {
			xml_ok = false;
			error_message = "Expected two elements - got " + to_string(element->count());
		}
		break;
	case XRP_PARAMS:
		// For a response should only get one item, but for request any number - for each element
		for (int i = 0; i < element->count() && xml_ok; i++) {
			// Get element
			child_element = element->child(i);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			rpc_data_item* item = new rpc_data_item;
			if (upper_name == "PARAM") {
				// Decode <param> element
				xml_ok = decode_xml_element(XRP_PARAM, child_element, item, dummy);
				if (xml_ok) {
					items->push_back(item);
				}
			}
			else {
				xml_ok = false;
				error_message = "Expected <param> - got <" + child_name + ">";
			}
		}
		method_name = "";
		break;
	default:
		// Cannot use this methods containing only a single item
		xml_ok = false;
		error_message = "Serious programming error decoding element" + element->child(0)->name();
	}
	if (!xml_ok && error_message.length() > 0) {
		status_->misc_status(ST_ERROR, ("XML_RPC" + error_message).c_str());
	}
	return xml_ok;
}

// Decode the individual XML element - contains a single element 
bool rpc_handler::decode_xml_element(rpc_element_t element_type, xml_element* element, rpc_data_item* item, bool& rpc_fault) {
	xml_element* child_element;
	string child_name;
	string upper_name;
	bool xml_ok = true;
	string error_message = "";
	bool dummy;
	switch (element_type) {
	case XRP_METHODRESPONSE:
		// Expect to get either a single <params> or <fault>
		if (element->count() == 1) {
			child_element = element->child(0);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			if (upper_name == "PARAMS") {
				xml_ok = decode_xml_element(XRP_PARAMS, child_element, item, dummy);
				rpc_fault = false;
			}
			else if (upper_name == "FAULT") {
				xml_ok = decode_xml_element(XRP_FAULT, child_element, item, dummy);
				rpc_fault = true;
			}
			else {
				xml_ok = false;
				error_message = "Expected <params> or <fault> - got <" + child_name + ">";
				rpc_fault = false;
			}
		}
		else {
			xml_ok = false;
			error_message = "Expected a single child - got" + to_string(element->count()) + " children";
		}
		break;
	case XRP_PARAMS:
		// For a response should only get one item, but for request any number - see above for request
		if (element->count() == 1) {
			child_element = element->child(0);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			if (upper_name == "PARAM") {
				// Get the single <param> element
				xml_ok = decode_xml_element(XRP_PARAM, child_element, item, dummy);
			}
			else {
				xml_ok = false;
				error_message = "Expected <param> - got <" + child_name + ">";
			}
		}
		else {
			xml_ok = false;
			error_message = "Expected a single child - got" + to_string(element->count()) + " children";
		}
		break;
	case XRP_FAULT:
		// Only 1 item expected
		if (element->count() == 1) {
			child_element = element->child(0);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			rpc_data_item* item = new rpc_data_item;
			if (upper_name == "VALUE") {
				// get the single <value> element
				xml_ok = decode_xml_element(XRP_PARAM, child_element, item, dummy);
			}
			else {
				xml_ok = false;
				error_message = "Expected <value> - got " + child_name;
			}
		}
		else {
			xml_ok = false;
			error_message = "Expected only 1 element - got" + to_string(element->count());
		}
		break;
	case XRP_PARAM:
		// Expect to get either a single <value> 
		if (element->count() == 1) {
			child_element = element->child(0);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			if (upper_name == "VALUE") {
				// get the single <value> element
				xml_ok = decode_xml_element(XRP_VALUE, child_element, item, dummy);
			}
			else {
				xml_ok = false;
				error_message = "Expected <value> - got " + child_name;
			}
		}
		else {
			xml_ok = false;
			error_message = "Expected a single child - got " + to_string(element->count()) + " children";
		}
		break;
	case XRP_VALUE:
		// Expect one of a single data-type element: no element means it's a STRING
		if (element->count() == 0) {
			// Use special DEFAULT data type to cope with lazy servers that use it 
			// for numerical data-types as well.
			string content = element->content();
			item->set(content, XRT_DEFAULT);
		}
		else if (element->count() == 1) {
			child_element = element->child(0);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			if (upper_name == "BOOLEAN") {
				xml_ok = decode_xml_element(XRP_BOOLEAN, child_element, item, dummy);
			}
			else if (upper_name == "INT" || upper_name == "I4") {
				xml_ok = decode_xml_element(XRP_INT, child_element, item, dummy);
			}
			else if (upper_name == "DOUBLE") {
				xml_ok = decode_xml_element(XRP_DOUBLE, child_element, item, dummy);
			}
			else if (upper_name == "STRING") {
				xml_ok = decode_xml_element(XRP_STRING, child_element, item, dummy);
			}
			else if (upper_name == "DATETIME.ISO8601") {
				xml_ok = decode_xml_element(XRP_DATETIME, child_element, item, dummy);
			}
			else if (upper_name == "BASE64") {
				xml_ok = decode_xml_element(XRP_BASE64, child_element, item, dummy);
			}
			else if (upper_name == "ARRAY") {
				xml_ok = decode_xml_element(XRP_ARRAY, child_element, item, dummy);
			}
			else if (upper_name == "STRUCT") {
				xml_ok = decode_xml_element(XRP_STRUCT, child_element, item, dummy);
			}
			else {
				xml_ok = false;
				error_message = "Expected a data-type element - got " + child_name;
			}
		}
		else {
			xml_ok = false;
			error_message = "Expected a 0 or 1 child - got " + to_string(element->count()) + " children";
		}
		break;
	case XRP_BOOLEAN:
		// expect only character data "0" or "1"
		if (element->count() == 0) {
			string content = element->content();
			bool value;
			if (content == "0") {
				value = false;
			}
			else if (content == "1") {
				value = true;
			}
			else {
				xml_ok = false;
				error_message = "Boolean value expected (0/1) - got " + content;
			}
			if (xml_ok) {
				item->set(value, XRT_BOOLEAN);
			}
		}
		else {
			xml_ok = false;
			error_message = "Children found when a value expected";
		}
		break;
	case XRP_INT:
		// expect only character data
		if (element->count() == 0) {
			string content = element->content();
			int value = 0;
			try {
				value = stoi(content);
			}
			catch (invalid_argument&) {
				xml_ok = false;
				error_message = "Integer value expected - got " + content;
			}
			if (xml_ok) {
				item->set(value, XRT_INT);
			}
		}
		else {
			xml_ok = false;
			error_message = "Children found when a value expected";
		}
		break;
	case XRP_DOUBLE:
		// expect only character data
		if (element->count() == 0) {
			string content = element->content();
			double double_value;
			try {
				double_value = stod(content);
			}
			catch( exception&) {
				xml_ok = false;
				error_message = "Real value expected - got " + content;
			}
			if (xml_ok) {
				item->set(double_value);
			}
		}
		else {
			xml_ok = false;
			error_message = "Children found when a value expected";
		}
		break;
	case XRP_STRING:
		// Expect only character data
		if (element->count() == 0) {
			string content = element->content();
			item->set(content, XRT_STRING);
		}
		else {
			xml_ok = false;
			error_message = "Children found when a value expected";
		}
		break;
	case XRP_DATETIME:
		// Expect only character data
		if (element->count() == 0) {
			string content = element->content();
			// Check it a valid ISO date/time yyyymmddThh:mm:ss
			if (regex_match<char>(content.c_str(), REGEX_RPC_DATETIME)) {
				item->set(content, XRT_DATETIME);
			}
			else {
				xml_ok = false;
				error_message = "Expected data in ISO date format yyyymmddThh:mm:ss - got " + content;
			}
		}
		else {
			xml_ok = false;
			error_message = "Children found when a value expected";
		}
		break;
	case XRP_BASE64:
		// Expect only character data
		if (element->count() == 0) {
			// Convert it to UTF-8
			string content = decode_base_64(element->content());
			item->set(content, XRT_BYTES);
		}
		else {
			xml_ok = false;
			error_message = "Children found when a value expected";
		}
	case XRP_ARRAY:
		// Expect only 1 element
		if (element->count() == 1) {
			child_element = element->child(0);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			if (upper_name == "DATA") {
				xml_ok = decode_xml_element(XRP_DATA, child_element, item, dummy);
			}
			else {
				xml_ok = false;
				error_message = "Expected <data> - got " + child_name;
			}
		}
		else {
			xml_ok = false;
			error_message = "Expected a single child - got " + to_string(element->count()) + "children";
		}
		break;
	case XRP_DATA:
		// Expect several elements - for each
		for (int i = 0; i < element->count() && xml_ok; i++) {
			// Get the element
			child_element = element->child(i);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			if (upper_name == "VALUE") {
				// Decode it
				rpc_data_item* sub_item = new rpc_data_item;
				xml_ok = decode_xml_element(XRP_VALUE, child_element, sub_item, dummy);
				if (xml_ok) {
					// Append the item to the array
					item->get_array()->push_back(sub_item);
				}
			}
			else {
				xml_ok = false;
				error_message = "Expected <value> got " + child_name;
			}
		}
		break;
	case XRP_STRUCT:
		// Expect several Elements - create a new structure to receive them
		item->set(new rpc_data_item::rpc_struct);
		for (int i = 0; i < element->count() && xml_ok; i++) {
			child_element = element->child(i);
			child_name = child_element->name();
			upper_name = to_upper(child_name);
			if (upper_name == "MEMBER") {
				xml_ok = decode_xml_element(XRP_MEMBER, child_element, item, dummy);
			}
			else {
				xml_ok = false;
				error_message = "Expected <member> got " + child_name;
			}
		}
		break;
	case XRP_MEMBER:
		// Expect two elements <name> and <value>
		if (element->count() == 2) {
			rpc_data_item* sub_item = new rpc_data_item;
			rpc_data_item* name_item = new rpc_data_item;
			for (int i = 0; i < 2 && xml_ok; i++) {
				child_element = element->child(i);
				child_name = child_element->name();
				upper_name = to_upper(child_name);
				if (upper_name == "NAME") {
					xml_ok = decode_xml_element(XRP_NAME, child_element, name_item, dummy);
				}
				else if (upper_name == "VALUE") {
					xml_ok = decode_xml_element(XRP_VALUE, child_element, sub_item, dummy);
				}
			}
			if (xml_ok) {
				(*item->get_struct())[name_item->get_string()] = sub_item;
				delete name_item;
			}
		}
	default:
		// Cannot use this methods containing only a single item
		xml_ok = false;
		error_message = "Serious programming error decoding element - " + element->child(0)->name();
		break;

	}
	if (!xml_ok && error_message.length() > 0) {
		status_->misc_status(ST_ERROR, ("XML_RPC: " + error_message).c_str());
	}
	return xml_ok;
}


