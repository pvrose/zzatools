#pragma once

#include "rig_if.h"
#include "hamlib/rig.h"

#include <string>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Spinner.H>

using namespace std;

class qso_connector :
    public Fl_Group
{

	// Hamlib parameters 
	struct hamlib_data {
		// Manufacturer
		string mfr = "";
		// Model
		string model = "";
		// Portname
		string port_name = "";
		// Baud rate
		string baud_rate = "9600";
		// Model ID - as knoen by hamlib
		int model_id = -1;
		// Override caps
		bool override_caps = false;
		// Port type
		rig_port_t port_type = RIG_PORT_NONE;
	};

	// Rig parameters (from handler onwards - rig only)
	struct cat_data {
		// Polling intervals
		double fast_poll_interval = 1.0;
		double slow_poll_interval = 60.0;
		//
		bool all_ports = false;
		// Hamlib data
		hamlib_data hamlib_params;
	};

public:

	qso_connector(int X, int Y, int W, int H, const char* l);
	~qso_connector();

	// get settings 
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable widgets
	void enable_widgets();
	// Save changes
	void save_values();
	// Switch the rig on or off
	void switch_rig();

protected:
	// Callback - model choice
	static void cb_ch_model(Fl_Widget* w, void* v);
	// Callback - hamlib serial ports
	static void cb_ch_port(Fl_Widget* w, void* v);
	// Callback - hamlib baudrate
	static void cb_ch_baud(Fl_Widget* w, void* v);
	// Callback override caps
	static void cb_ch_over(Fl_Widget* w, void* v);
	// Callback all ports
	static void cb_bn_all(Fl_Widget* w, void* v);
	// Callback network port
	static void cb_ip_port(Fl_Widget* w, void* v);
public:
	// Callback - Connect button
	static void cb_bn_connect(Fl_Widget* w, void* v);
protected:
	// Spinner fast polling rate
	static void cb_ctr_pollfast(Fl_Widget* w, void* v);
	// slow polling rate
	static void cb_ctr_pollslow(Fl_Widget* w, void* v);

	//populate port choice
	void populate_port_choice();
	// Populate model choice
	void populate_model_choice();
	//Populate baud rate choice
	void populate_baud_choice();

	Fl_Group* serial_grp_;
	Fl_Group* network_grp_;
	// Hamlib widgets to revalue when rig selected changes
	Fl_Widget* mfr_choice_;
	Fl_Widget* rig_model_choice_;
	Fl_Widget* port_if_choice_;
	Fl_Widget* port_if_input_;
	Fl_Widget* baud_rate_choice_;
	Fl_Widget* rig_choice_;
	Fl_Widget* override_check_;
	Fl_Widget* show_all_ports_;
	Fl_Button* bn_connect_;

	// CAT connection data
	cat_data* cat_data_;
	// Waiting connect
	bool wait_connect_;

	Fl_Spinner* ctr_pollfast_;
	Fl_Spinner* ctr_pollslow_;


};

