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
 
//! This class provides a dialog for manging station locations, operators and callsigns
class stn_dialog :
    public page_dialog
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	stn_dialog(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
    ~stn_dialog();

	//! Inheritedfrom page_dialog allowing keyboard F1 to open userguide.
	virtual int handle(int event);

	// inherited methods
	// Standard methods - need to be written for each
	//! Load values from stn_data.
	virtual void load_values();
	//! Instantiate component widgwts
	virtual void create_form(int X, int Y);
	//! Save values and update other widgets with changes.
	virtual void save_values();
	//! Configure component widgets after data changes,
	virtual void enable_widgets();

	//! Callback from changing tabs.
	static void cb_tab(Fl_Widget* w, void* v);

	//! type for the single_tab 
	enum tab_type : char {
		QTH,           //!< Location data editor.
		OPERATOR,      //!< Operator data editor.
		CALLSIGN       //!< Callsign data editor.
	};

	//! Set the active tab \p t and set its default value \p id.
	void set_tab(tab_type t, string id);

protected:

	//! This class provides the tab used for each of location, operator and callsign.
	class single_tab : public Fl_Group
	{
	public:
		//! Constructor.
		single_tab(int rx, int ry, int rw, int rh, char type, const char* l = nullptr);
		//! Destructor.
		~single_tab();

		//! Instantiate component widgets.
		void create_form();
		//! Configure component widgets after data changes.
		void enable_widgets();
		//! Loads type specific data from stn_data.
		void load_data();
		//! Saves data back to stn_data.
		void save_data();

		//! Callback from identifier choice.
		static void cb_ch_id(Fl_Widget* w, void* v);

		//! Callback from "Call" choice in locator - used to populate DXCC etc.
		static void cb_ch_call(Fl_Widget* w, void* v);
		//! Callback from "Clear" button to clear all values for specific identifier.
		static void cb_clear(Fl_Widget* w, void* v);
		//! Callback from "Update from call" updates locator items that can be decoded from the call.
		static void cb_update(Fl_Widget* w, void* v);

		//! Set the type of the tab.
		virtual void type(char t);
		//! Returns the type of the tab.
		virtual char type();

		//! Returns the selected identifier.
		string id();
		//! Set the identifier to \p s.
		void id(string s);

	protected:
		//! Populate the choice
		void populate_choice(Fl_Input_Choice* ch, tab_type t);
		//! Update items when call changes (for location).
		void update_from_call();
		//! Clear all items.
		void clear_entry();

		//! Number of individual input widgets
		int num_inputs_;
		
		//! Array of labels for the widgets.
		vector<string> labels_;

		//! Location specific data
		const qth_info_t* qth_;
		//! Operator specific data.
		const oper_info_t* oper_;
		//! Callsign specific data.
		string call_descr_;

		//! Current data id
		string current_id_;

		//! Update from call
		bool update_from_call_;

		// Choice: identifier
		Fl_Input_Choice* ch_id_;
		// Check: Update items form call.
		Fl_Check_Button* bn_update_;
		// Choice: Separate callsign choice to be used for priming location data.
		Fl_Input_Choice* ch_call_;
		// Button: Clear all.
		Fl_Button* bn_clear_;
		// Array of inputs: Individual items for specific type.
		Fl_Input** ip_values_;
	};

	//! Current QSO record.
	record* qso_;

	Fl_Tabs* tabs_;           //!< Tabs: 
	single_tab* g_qth_;       //!< Tab: Location editing
	single_tab* g_oper_;      //!< Tab: Operator editing.
	single_tab* g_call_;      //!< Tab: Callsign editing.
};

