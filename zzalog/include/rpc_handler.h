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

	//! This class acts as both a server or a client for the XML-RPC interfcae.
	class rpc_handler
	{
	public:
		//! The only HTTP codes supported
		enum http_code {
			OK = 200,           //<! 200 OK
			BAD_REQUEST = 400   //<! 400 Bad Request.
		};
		//! Structure for an entry for specific method call.
		struct method_entry {
			string name;        //!< Method name
			string signature;   //!< Method signature - ie encoded parameter and response
			string help_text;   //!< Brief help text.
		};

		//! Constructor.
		
		//! \param host_address Network address of host
		//! \param port_name Integer port identifier
		//! \param resource_name A name indicating protocol (in this case "RPC2")
		rpc_handler(string host_address, int port_name, string resource_name);
		//! Destructor
		~rpc_handler();
		//! Generate XML for the request and decode XML for the response
		
		//! \param method_name Name of the method
		//! \param params Parameters for the method
		//! \param response Receives response from the remote procedure call.
		//! \return true if successful.
		bool do_request(string method_name, rpc_data_item::rpc_list* params, rpc_data_item* response);
		//! Receive the request from strean \p ss.
		static int rcv_request(stringstream& ss);
		//! Run server
		void run_server();
		//! Close server
		void close_server();
		//! Returns true if the server is active.
		bool has_server();
		//! Add a method for the server to handle.
		
		//! \param method Method entry structire.
		//! \param callback Local method to handle request.
		void add_method(method_entry method, int(*callback)(rpc_data_item::rpc_list& params, rpc_data_item& response));

	protected:
		//! Method definition structure
		struct method_def {
			string signature;        //!< Method signature (coded form of parameters and response).
			string help_text;        //!< Help text
			int(*callback)(rpc_data_item::rpc_list& params, rpc_data_item& response) { nullptr };
			                         //!< Method call - callback(params, response) 
		};


		//! Generate the RPC request XML
		
		//! \param method_name Name of method.
		//! \param params Parameters for the method.
		//! \param request_xml Output stream to send the request out on.
		//! \return true if successful.
		bool generate_request(string method_name, rpc_data_item::rpc_list* params, ostream& request_xml);
		//! Generate an RPC Response.
		
		//! \param fault true if responding with an error.
		//! \param response The data item as method return.
		//! \param response_xml Output stream to send the response out on.
		bool generate_response(bool fault, rpc_data_item* response, ostream& response_xml);
		//! Decode the RPC Request XML
		
		//! \param request_xml Input stream provodong the request
		//! \param method_name Receives the method name
		//! \param params Receives the parameters for the request
		//! \return true if successful.
		bool decode_request(istream& request_xml, string& method_name, rpc_data_item::rpc_list* params);
		//! Decode the RPC Response XML.
		
		//! \param response_xml Input stream providing the response
		//! \param response Receives the response returned by the remote method.
		//! \param fault Receives true if the response contains an error.
		//! \return true if successful.
		bool decode_response(istream& response_xml, rpc_data_item* response, bool& fault);
		//! Write XML for an individual data \p item using \p pWriter.
		bool write_item(xml_writer* pWriter, rpc_data_item* item);
		//! Decode the individual XML element for a request
		
		//! \param Type The element type.
		//! \param pElement The element or elements to decode.
		//! \param method_name Receives the name of the method
		//! \param items Receives the method parameters or response.
		//! \return true if successful
		bool decode_xml_element(rpc_element_t Type, xml_element* pElement, string& method_name, rpc_data_item::rpc_list* items);
		//! Decode the individual XML element for a response

		//! \param Type The element type.
		//! \param pElement The element or elements to decode.
		//! \param item Response data item
		//! \param fault Response was an error,
		//! \return true if successful
		bool decode_xml_element(rpc_element_t Type, xml_element* pElement, rpc_data_item* item, bool& fault);
		//! Decode the request on the input stream \p ss, perform the action and send response.
		int handle_request(stringstream& ss);
		//! Reserved method: List the available methods.
		static int list_methods(rpc_data_item::rpc_list& params, rpc_data_item& response);
		//! Reseeved method: Send method help message.
		static int method_help(rpc_data_item::rpc_list& params, rpc_data_item& response);

		//! Generate error \p response for \p code with \p message.
		void generate_error(int code, string message, rpc_data_item& response);

		//! Remove header - returns true if successful and \p payload receives the request body.
		bool strip_header(stringstream& message, stringstream& payload);
		//! Add header with result \p code to the \p payload to generate  \p message.
		bool add_header(http_code code, stringstream& payload, stringstream& message);

		//! The resource name
		string resource_;
		//! The host name
		string host_name_;
		//! HTML server
		socket_server* server_;
		//! Server port
		int server_port_;
		//! Pointer back to self
		static rpc_handler* that_;
		//! The method definitions
		map<string, method_def> method_list_;

	};
#endif
