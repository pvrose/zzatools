#pragma once

#include "rig_if.h"
#include "hamlib/rig.h"

#include <string>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Spinner.H>

using namespace std;

class qso_rig :
    public Fl_Group
{


public:

	qso_rig(int X, int Y, int W, int H, const char* l);
	~qso_rig();

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
	// 1s clock interface
	void ticker();
	// New rig
	void new_rig();
	// Get the rig
	rig_if* rig();

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
	// Callback - Connect button
	static void cb_bn_connect(Fl_Widget* w, void* v);

	// Get hamlib data
	void find_hamlib_data();

	//populate port choice
	void populate_port_choice();
	// Populate model choice
	void populate_model_choice();
	//Populate baud rate choice
	void populate_baud_choice();

	Fl_Group* serial_grp_;
	Fl_Group* network_grp_;
	// Hamlib widgets to revalue when rig selected changes
	Fl_Choice* ch_rig_model_;
	Fl_Choice* ch_port_name_;
	Fl_Input* ip_port_;
	Fl_Choice* ch_baud_rate_;
	Fl_Check_Button* bn_all_rates_;
	Fl_Check_Button* bn_all_ports_;
	Fl_Button* bn_connect_;

	// Add all ports to port choice
	bool use_all_ports_;
	// Use all baud rates
	bool use_all_rates_;

	rig_if* rig_;
	// hamlib data
	rig_if::hamlib_data_t hamlib_data_;
	// Current mode
	rig_port_e mode_;

};

