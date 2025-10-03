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
class stn_call_dlg;
class stn_oper_dlg;
class stn_qth_dlg;
struct qth_info_t;
struct oper_info_t;


 
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

	//! Set the active tab \p t and std::set its default value \p id.
	void set_tab(tab_type t, std::string id);

protected:

	//! Current QSO record.
	record* qso_;

	Fl_Tabs* tabs_;           //!< Tabs: 
	stn_qth_dlg* g_qth_;       //!< Tab: Location editing
	stn_oper_dlg* g_oper_;      //!< Tab: Operator editing.
	stn_call_dlg* g_call_;      //!< Tab: Callsign editing.
};

