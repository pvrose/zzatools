#pragma once

#include "page_dialog.h"

#include <string>
#include <vector>

class Fl_Button;
class Fl_Check_Button;
class Fl_Group;
class Fl_Input;
class Fl_Input_Choice;
class Fl_Multiline_Input;
class Fl_Tabs;
class Fl_Widget;

class record;
struct qth_info_t;
struct oper_info_t;

using namespace std;
 
class stn_dialog :
    public page_dialog
{
public:
    stn_dialog(int X, int Y, int W, int H, const char* L = nullptr);
    ~stn_dialog();

	virtual int handle(int event);

	// inherited methods
	// Standard methods - need to be written for each
	// Load values
	virtual void load_values();
	// Used to create the form
	virtual void create_form(int X, int Y);
	// Used to write settings back
	virtual void save_values();
	// Used to enable/disable specific widget - any widgets enabled musr be attributes
	virtual void enable_widgets();

	// Switch tabs
	static void cb_tab(Fl_Widget* w, void* v);

	// type for the single_tab 
	enum tab_type : char {
		QTH,
		OPERATOR,
		CALLSIGN
	};

	void set_tab(tab_type t, string id);

protected:

	class single_tab : public Fl_Group
	{
	public:
		single_tab(int rx, int ry, int rw, int rh, char type, const char* l = nullptr);
		~single_tab();

		void create_form();
		void enable_widgets();
		void load_data();
		void save_data();

		static void cb_ch_id(Fl_Widget* w, void* v);

		static void cb_ch_call(Fl_Widget* w, void* v);
		static void cb_clear(Fl_Widget* w, void* v);
		static void cb_update(Fl_Widget* w, void* v);

		virtual void type(char t);
		virtual char type();

		string id();
		void id(string s);

	protected:
		// Populate the choice
		void populate_choice(Fl_Input_Choice* ch, tab_type t);

		void update_from_call();

		void clear_entry();

		// Number of individual input widgets
		int num_inputs_;

		vector<string> labels_;

		// Type specific data
		const qth_info_t* qth_;
		const oper_info_t* oper_;
		string call_descr_;

		// Current data id
		string current_id_;

		// Update from call
		bool update_from_call_;

		// Widgets
		Fl_Input_Choice* ch_id_;
		// Update from call
		Fl_Check_Button* bn_update_;
		// Separate call choice for QTH
		Fl_Input_Choice* ch_call_;
		// Clear all
		Fl_Button* bn_clear_;
		// Array of input widgets
		Fl_Input** ip_values_;
	};

	record* qso_;

	Fl_Tabs* tabs_;
	single_tab* g_qth_;
	single_tab* g_oper_;
	single_tab* g_call_;


};

