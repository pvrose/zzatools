#ifndef __FLLOG_EMUL__
#define __FLLOG_EMUL__

#include "rpc_data_item.h"
#include "rpc_handler.h"

class record;


//! \brief This class provides an emulation for fllog software, the logging application
//! for the fldigi suite of applications. 

//! It allows the log to be updated 
//! directly from fldigi. Requests and Responses are in XML, Remote Procedure Call (XML-RPC) format.

	class fllog_emul
	{
	public:
		//! Constructor.
		fllog_emul();
		//! Destructoor.
		~fllog_emul();
		//! Start the server.
		void run_server();
		//! Close the server.
		void close_server();
		//! Returns the server state.
		bool has_server();
		//! Returns true if a request has been received.
		bool has_data();

	protected:
		//! Fetch the first record that matches callsign request.
		
		//! \param v Pointer to this instance.
		//! \param params Request.
		//! \param response Returned response.
		//! \return true if request fails, otherwise false.
		static int get_record(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response);
		//! Check duplicate - replies true (exact match), possible (callsign matches), false (not a match.

		//! \param v Pointer to this instance.
		//! \param params Request.
		//! \param response Returned response.
		//! \return true if request fails, otherwise false.
		static int check_dup(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response);
		//! Add new record

		//! \param v Pointer to this instance.
		//! \param params Request.
		//! \param response Returned response.
		//! \return true if request fails, otherwise false.
		static int add_record(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response);
		//! Update fileds in current selection

		//! \param v Pointer to this instance.
		//! \param params Request.
		//! \param response Returned response.
		//! \return true if request fails, otherwise false.
		static int update_record(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response);
		//! List methods - std::string responds with a std::list of methods suppported

		//! \param v Pointer to this instance.
		//! \param params Request.
		//! \param response Returned response.
		//! \return true if request fails, otherwise false.
		static int list_methods(void* v, rpc_data_item::rpc_list& params, rpc_data_item& response);

		//! Generate error response
		
		//! \param code Error code.
		//! \param message Error message
		//! \param response Formatted response for sending to client.
		void generate_error(int code, std::string message, rpc_data_item& response);

		//! Check FlDigi isconnected
		void check_connected();

		//! The currently selected record.
		record* current_qso_;
		//! The record possibly being created.
		record* putative_qso_;
		//! XMK-RPC handler.
		rpc_handler* rpc_handler_;
		//! The std::list of methods supported by the RPC interface
		std::list<rpc_handler::method_entry> method_list_;
		
		//! Connected
		bool connected_;


	};

#endif
