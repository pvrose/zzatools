#ifndef __FLLOG_EMUL__
#define __FLLOG_EMUL__

#include "rpc_data_item.h"
#include "rpc_handler.h"
#include "record.h"



// This class provides an emulation for fllog software, the logging application
// for the fldigi suite of applications. It allows the log to be updated 
// directly from fldigi

	class fllog_emul
	{
	public:
		fllog_emul();
		~fllog_emul();
		// Start the server
		void run_server();
		// Clsoe servefr
		void close_server();
		// Return server state
		static bool has_server();
		// Received a packet
		static bool has_data();

	protected:
		// Get ADIF string for first record with callsign
		static int get_record(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// Check duplicate - replies true (exact match), possible (callsign matches), false (not a match
		static int check_dup(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// Add new record
		static int add_record(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// Update fileds in current selection
		static int update_record(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// List methods - string returns list of methods suppported
		static int list_methods(rpc_data_item::rpc_list& params, rpc_data_item& response);

		// Generate error resposne
		void generate_error(int code, string message, rpc_data_item& response);

		// Check connected
		void check_connected();

		// The currently selected record
		record* current_qso_;
		// The record possibly being created
		record* putative_qso_;
		// RPC handler
		rpc_handler* rpc_handler_;
		// The list of methods supported by the RPC interface
		list<rpc_handler::method_entry> method_list_;
		
		// The only instance
		static fllog_emul* that_;

		// Connected
		bool connected_;


	};

#endif
