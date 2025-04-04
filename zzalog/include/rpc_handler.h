#ifndef __RPC_HANDLER__
#define __RPC_HANDLER__

#include "rpc_data_item.h"

#include <string>
#include <istream>
#include <ostream>
#include <regex>
#include <map>

using namespace std;

class xml_element;
class xml_writer;
class socket_server;

	// 


	// This class acts as both a server or a client for the XML-RPC interfcae.
	class rpc_handler
	{
	public:
		// The only codes implemented
		enum http_code {
			OK = 200,
			BAD_REQUEST = 400
		};
		struct method_entry {
			string name; string signature; string help_text;
		};

		// Handler for client (or both)
		rpc_handler(string host_address, int port_name, string resource_name);
		~rpc_handler();
		// Generate XML for the request and decode XML for the response
		bool do_request(string method_name, rpc_data_item::rpc_list* params, rpc_data_item* response);
		// Packet handler
		static int rcv_request(stringstream& ss);
		// Run server
		void run_server();
		// Close server
		void close_server();
		// Has server
		bool has_server();
		// Add server method
		void add_method(method_entry method, int(*callback)(rpc_data_item::rpc_list& params, rpc_data_item& response));

	protected:
		struct method_def {
			string signature;
			string help_text;
			int(*callback)(rpc_data_item::rpc_list& params, rpc_data_item& response) { nullptr };
		};


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
		// Actually handle the request
		int handle_request(stringstream& ss);
		// Rserved methods
		static int list_methods(rpc_data_item::rpc_list& params, rpc_data_item& response);
		static int method_help(rpc_data_item::rpc_list& params, rpc_data_item& response);

		// Generate error resposne
		void generate_error(int code, string message, rpc_data_item& response);

		// Remove header - returns true if successful and payload is valid
		bool strip_header(stringstream& message, stringstream& payload);
		// Add header
		bool add_header(http_code code, stringstream& payload, stringstream& message);

		// The resource name
		string resource_;
		// The host name
		string host_name_;
		// HTML server
		socket_server* server_;
		// Server port
		int server_port_;
		// Pointer back to sel
		static rpc_handler* that_;
		// The method definitions
		map<string, method_def> method_list_;

	};
#endif
