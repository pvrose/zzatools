#ifndef __FLLOG_EMUL__
#define __FLLOG_EMUL__

#include "../zzalib/rpc_data_item.h"
#include "../zzalib/rpc_handler.h"
#include "record.h"

using namespace zzalib;

// This class provides an emulation for fllog software, the logging application
// for the fldigi suite of applications. It allows the log to be updated 
// directly from fldigi

namespace zzalog {


	class fllog_emul
	{
	public:
		fllog_emul();
		~fllog_emul();
		// Start the server
		void run_server();
		// request back from the RPC handler
		static int cb_do_request(string method_name, rpc_data_item::rpc_list& params, rpc_data_item& response);

	protected:
		// Non-static version
		int do_request(string method_name, rpc_data_item::rpc_list& params, rpc_data_item& response);
		// Get ADIF string for first record with callsign
		int get_record(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// Check duplicate - replies true (exact match), possible (callsign matches), false (not a match
		int check_dup(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// Add new record
		int add_record(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// Update fileds in current selection
		int update_record(rpc_data_item::rpc_list& params, rpc_data_item& response);
		// List methods - string returns list of methods suppported
		int list_methods(rpc_data_item::rpc_list& params, rpc_data_item& response);

		// Generate error resposne
		void generate_error(int code, string message, rpc_data_item& response);

		// The currently selected record
		record* current_record_;
		// RPC handler
		rpc_handler* rpc_handler_;

	};

}

#endif
