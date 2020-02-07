#ifndef __RIG_DIALOG__
#define __RIG_DIALOG__

#include "page_dialog.h"
#include "../zzalib/rig_if.h"

#include <string>
#include <map>
#include <set>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Int_Input.H>

using namespace std;
using namespace zzalib;

namespace zzalog {

	// Maximum teletype number (for Windows use - TODO: Posix)
	const unsigned int TTY_MAX = 255;

	// This class provides a dialog to configure the rig connection
	class rig_dialog : public page_dialog
	{
	public:
		rig_dialog(int X, int Y, int W, int H, const char* label);
		~rig_dialog();

		// protected methods
	protected:
		// Load data
		virtual void load_values();
		// create the form
		virtual void create_form(int X, int Y);
		// enable/disable widgets
		virtual void enable_widgets();
		// save values
		virtual void save_values();
		// populate hamlib rig choice
		void populate_rig_choice();
		//populate port choice
		void populate_port_choice();
		// Populate model choice
		void populate_model_choice();
		//Populate baud rate choice
		void populate_baud_choice();
		// Save rig values
		void save_hamlib_values();
		// state of the windows comms port
		enum port_state {
			OK,
			NOT_PRESENT,
			NOT_AVAILABLE
		};

	protected:
		// Radio button handler
		static void cb_rad_handler(Fl_Widget* w, void* v);
		// Callback - actual rig select changed
		static void cb_ch_rig(Fl_Widget* w, void* v);
		// Callback - model choice
		static void cb_ch_model(Fl_Widget* w, void* v);
		// Callback override caps
		static void cb_ch_over(Fl_Widget* w, void* v);
		// Callback all ports
		static void cb_bn_all(Fl_Widget* w, void* v);

		// attributes
	protected:
		// settings
		// The rig handler choice
		rig_handler_t selected_rig_if_;
		// Polling interval when connected
		double fast_poll_interval_;
		// Polling interval when disconnected
		double slow_poll_interval_;
		// Hamlib parameters 
		struct hamlib_data {
			// Manufacturer
			string mfr;
			// Model
			string model;
			// Portname
			string port_name;
			// Baud rate
			string baud_rate = "9600";
			// Override caps
			bool override_caps = false;
		};
		// Manufacturer
		string hamlib_mfr_;
		// Model
		string hamlib_model_;
		// Serial port name
		string port_name_;
		// Baud rate
		string baud_rate_;
		// Set of actual supported rigs
		map<string, hamlib_data> actual_rigs_;
		// Name of current rig (as known by user)
		string current_rig_;
		// FLRig parameters
		// 4-byte IPv4 address
		int ip_address_[4];
		// Port number
		int ip_port_;
		// XML-RPC resource identifier
		string ip_resource_;
		// Hamlib rig number
		int model_id_;
		// Allow baud choice to override rig_caps
		bool override_caps_;
		// The available ports
		bool all_ports_;
		string* existing_ports_;

		// The handler radio button parameters
		radio_param_t* handler_radio_params_;
		// widgets
		Fl_Group* hamlib_grp_;
		Fl_Group* flrig_grp_;
		Fl_Group* norig_grp_;
		// Hamlib widgets to revalue when rig selected changes
		Fl_Choice* mfr_choice_;
		Fl_Choice* rig_model_choice_;
		Fl_Choice* port_if_choice_;
		Fl_Choice* baud_rate_choice_;
		Fl_Choice* rig_choice_;
		Fl_Check_Button* override_check_;
		Fl_Check_Button* show_all_ports_;

	};

}
#endif
