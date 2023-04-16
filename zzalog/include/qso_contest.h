#pragma once

#include <string>
#include "field_choice.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Window.H>

using namespace std;

class qso_contest :
    public Fl_Group
{
public:

	// Adding an exchange
	enum contest_mode_t {
		NO_CONTEST = 0,  // Normal non-contest logging behaviour
		CONTEST,         // Normal contest logging behaviour
		PAUSED,          // Log non-contest within contest logging
		NEW,             // A new format id is selected
		DEFINE,          // Define new contest exchange definition
		EDIT             // Edit contest exchange definition
	};

	qso_contest(int X, int Y, int W, int H, const char* l = nullptr);
	~qso_contest();

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();
	// Mode
	contest_mode_t mode();
	// Contest fields
	string contest_fields();
	// Get serial number
	int serial_number();
	// conditionally Increment serial number
	void increment_serial();
	// Create/display instructions window
	void instructions_window(bool show);

protected:
	// Initialise setial number
	static void cb_init_serno(Fl_Widget* w, void* v);
	// Increment serial number
	static void cb_inc_serno(Fl_Widget* w, void* v);
	// Decrement serial number
	static void cb_dec_serno(Fl_Widget* w, void* v);
	// Define contest exchange format
	static void cb_def_format(Fl_Widget* w, void* v);
	// Start/Stop contest mode
	static void cb_ena_contest(Fl_Widget* w, void* v);
	// Pause contest mode
	static void cb_pause_contest(Fl_Widget* w, void* v);
	// Exchange format choice
	static void cb_format(Fl_Widget* w, void* v);
	// Add exchange button
	static void cb_add_exch(Fl_Widget* w, void* v);

	// Add contest exchanges
	void populate_exch_fmt();
	// Add new format - return format index
	int add_format_id(string id);
	// Add new format definition 
	void add_format_def(int ix, bool tx);
	// Initialise fields through parent
	void initialise_fields();

	// Mode
	contest_mode_t contest_mode_;
	// Contest ID
	string contest_id_;
	// Contest exchange format index
	int exch_fmt_ix_;
	// And its id
	string exch_fmt_id_;
	// Exchanges: format ID, TX and RX fields
	static const int MAX_CONTEST_TYPES = 100;
	string ef_ids_[MAX_CONTEST_TYPES];
	string ef_txs_[MAX_CONTEST_TYPES];
	string ef_rxs_[MAX_CONTEST_TYPES];
	int max_ef_index_;

	// Serial number
	int serial_num_;
	// RX fields edited
	bool rx_set_;
	bool tx_set_;

	// Contest ID
	field_choice* ch_contest_id_;
	// Contest exchange
	Fl_Input_Choice* ch_format_;
	// Add exchange
	Fl_Button* bn_add_exch_;
	// Define exchanges
	Fl_Button* bn_define_tx_;
	Fl_Button* bn_define_rx_;
	// TX Serial number
	Fl_Output* op_serno_;
	// Initialise serial number
	Fl_Button* bn_init_serno_;
	// Increment serial number
	Fl_Button* bn_inc_serno_;
	// Decrement serial number
	Fl_Button* bn_dec_serno_;
	// Pause contest
	Fl_Light_Button* bn_pause_;
	// Enable contest
	Fl_Light_Button* bn_enable_;
	// Instructions
	Fl_Window* wn_instructions_;

};

