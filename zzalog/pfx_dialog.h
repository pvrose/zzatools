#ifndef  __PFX_DIALOG__
#define  __PFX_DIALOG__

#include "prefix.h"

#include "../zzalib/win_dialog.h"
#include <vector>
#include <string>

#include <FL/Fl_Output.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Table_Row.H>

using namespace std;
using namespace zzalib;

namespace zzalog {

	// Forward declaration
	class pfx_dialog;

	// This class provides the extensions to Fl_Table_Row for use in pfx_dialog
	class pfx_dlg_table : public Fl_Table_Row {
	public:
		// Constructor sets the parent pfx_dialog as well as standard widget construction
		pfx_dlg_table(pfx_dialog* parent, int X, int Y, int W, int H, const char* label = 0);
		virtual ~pfx_dlg_table();

		// inherited
	public:
		// inherited from Fl_Table_Row
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);

		// protected attributes
		pfx_dialog* parent_;
		// last event
		int last_event_;
		// Mouse Button clicked for last event
		int last_button_;
		// Was it multiple clicks?
		int last_clicks_;

	};

	// This class provides a dialog whereby the user selects one of the possible prefixes that callsign parsing
	// has detected for the callsign
	class pfx_dialog : public win_dialog

	{
	public:
		pfx_dialog();
		virtual ~pfx_dialog();

		// callbacks
	public:
		// Prefix table selected
		static void cb_tab_pfx(Fl_Widget* c, void* v);
		// QRZ.com button clicked
		static void cb_bn_qrz(Fl_Widget* w, void* v);
		// OK button clicked
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// Cancel button clicked
		static void cb_bn_cancel(Fl_Widget* w, void* v);

		//public methods
	public:
		// Set the list of possible prefixes and the callsign being parsed
		void set_data(vector<prefix*>* prefixes, string callsign);
		// Get the selected prefix
		prefix* get_prefix();
		// Get numbered prefix
		prefix* get_prefix(size_t prefix_num);

		// Protected attributes
	protected:
		// The call widget
		Fl_Output * call_out_;
		// The prefix table
		pfx_dlg_table* table_;
		// callsign to display
		string callsign_;
		// An ordered set of prefixes
		vector<prefix*>* prefixes_;
		// selected prefix
		unsigned int selection_;
		// OK/Cancel
		button_t button_;
		// pending OK/CANCEL
		bool pending_button_;
		// List of repositionalble widgets
		list<Fl_Widget*> rep_widgets_;
	};

}
#endif // 


