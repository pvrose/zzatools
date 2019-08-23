#ifndef __RPC_HANDLER__
#define __RPC_HANDLER__

#include "rpc_data_item.h"
#include "xml_element.h"
#include "xml_writer.h"

#include <string>
#include <istream>
#include <ostream>
#include <regex>

namespace zzalog {
	// 
	// RPC date time format
	const basic_regex<char> REGEX_RPC_DATETIME("[0-9]*8T[0123][0-9]:[0-5][0-9]:[0-5][0-9]");


	// This class generates Remote Procedure Call (RPC) requests and analyses RPC responses
	class rpc_handler
	{
	public:
		rpc_handler(string host_name, int port_num_, string resource_name);
		~rpc_handler();
		// Generate XML for the request and decode XML for the response
		bool do_request(string method_name, rpc_data_item::rpc_list* params, rpc_data_item* response);

	protected:
		// Generate the RPC request XML
		bool generate_request(string method_name, rpc_data_item::rpc_list* params, ostream& request_xml);
		// Generate an RPC Respose
		bool generate_response(bool fault, rpc_data_item* response, ostream& response_xml);
		// Decode the RPC Request XML
		bool decode_request(istream& request_xml, string& method_name, rpc_data_item::rpc_list* params);
		// Decode the RPC Response XML
		bool decode_response(istream& response_xml, rpc_data_item* response, bool& fault);
		// Write XML for an individual data item
		bool write_item(xml_writer* pWriter, rpc_data_item* item);
		// Decode the individual XML element 
		bool decode_xml_element(rpc_element_t Type, xml_element* pElement, string& method_name, rpc_data_item::rpc_list* items);
		bool decode_xml_element(rpc_element_t Type, xml_element* pElement, rpc_data_item* item, bool& fault);
		// The resource name
		string resource_;
		// The host name
		string host_name_;

	};

}
#endif
