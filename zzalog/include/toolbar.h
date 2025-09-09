#ifndef __TOOLBAR__
#define __TOOLBAR__

#include "drawing.h"

#include <string>

#include <FL/Fl_Group.H>

using namespace std;

class Fl_Widget;

typedef size_t qso_num_t;

	//! This class provides a tool-bar providing buttons providing shortcuts to certain menu items.
	class toolbar : public Fl_Group
	{
	public:
		//! Constructor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		toolbar(int X, int Y, int W, int H, const char* L = 0);
		//! Destructor.
		~toolbar();

		//! Inherited from Fl_Group to allow keyboard F1 to open userguide.
		virtual int handle(int event);

		//! Callback from buttons that are associated with a menu item: \p v passes menu item label.
		static void cb_bn_menu(Fl_Widget* w, void* v);
		//! Callback for search button
		static void cb_bn_search(Fl_Widget* w, void* v);
		//! Callback for extract button
		static void cb_bn_extract(Fl_Widget* w, void* v);
		//! Call back for parsing and explaining a callsign
		static void cb_bn_explain(Fl_Widget* w, void* v);
		//! Callback to (dis)connect rig
		static void cb_bn_rig(Fl_Widget* w, void* v);
		//! Callback to open international dialog
		static void cb_bn_intl(Fl_Widget* w, void* v);
		//! Supply minimum width of toolbar
		int min_w();
		//! Add selected callsign as default input
		void search_text(int record_num);
		//! Set search text to a specific calllsign
		void search_text(string callsign);
		//! Update button status
		void update_items();

	protected:
		//! text to search for
		string search_text_;
		//! Index of QSO record when stepping the search through the log
		qso_num_t record_num_;
		//! The search input
		Fl_Widget* ip_search_;

		//! Minimum resizable
		int min_w_;
	};
#endif

