#ifndef __TOOLBAR__
#define __TOOLBAR__

#include "drawing.h"
#include "record.h"

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

using namespace std;


	// This class provides a tool-bar providing buttons providing shortcuts to certain menu items
	class toolbar : public Fl_Group
	{
	public:
		toolbar(int X, int Y, int W, int H, const char* label = 0);
		~toolbar();

		// Button callback - calls specific menu callback
		static void cb_bn_menu(Fl_Widget* w, void* v);
		// Call back for search button
		static void cb_bn_search(Fl_Widget* w, void* v);
		// Call back for extract button
		static void cb_bn_extract(Fl_Widget* w, void* v);
		// Call back for parsing and explaining a callsign
		static void cb_bn_explain(Fl_Widget* w, void* v);
		// Callback for starting auto-import
		static void cb_bn_import(Fl_Widget* w, void* v);
		// Callback to (dis)connect rig
		static void cb_bn_rig(Fl_Widget* w, void* v);
		// Callback to open international dialog
		static void cb_bn_intl(Fl_Widget* w, void* v);
		// Supply minimum width of toolbar
		int min_w();
		// Add selected callsign as default input
		void search_text(int record_num);
		// Set search text to a specific calllsign
		void search_text(string callsign);
		// Update button status
		void update_items();

	protected:
		// text to search for
		string search_text_;
		// Record number when stepping the search through the log
		qso_num_t record_num_;
		// The search input
		Fl_Widget* ip_search_;

		// Minimum resizable
		int min_w_;
	};
#endif

