#ifndef __TABBED_FORMS__
#define __TABBED_FORMS__

#include "view.h"
#include "fields.h"
#include "drawing.h"

#include <vector>
#include <map>
#include <FL/Fl_Tabs.H>

class book;
enum hint_t : uchar;
typedef size_t qso_num_t;



	// This class is a view with tabs that contain the specialised views
	class tabbed_forms : public Fl_Tabs
	{
	public:
		tabbed_forms(int X, int Y, int W, int H, const char* label = 0);
		~tabbed_forms();

		// tell all views that record(s) have changed
		void update_views(view* requester, hint_t hint, qso_num_t record_1, qso_num_t record_2 = 0);
		// set the various books into the views
		void books();
		// Return the specific view
		view* get_view(object_t view_name);
		// Minimum resizing
		int min_w();
		int min_h();
		// Activate/deactivate pane
		void activate_pane(object_t pane, bool active);
		// tab callback
		static void cb_tab_change(Fl_Widget* w, void* v);

	protected:
		// Add a view of class VIEW
		template <class VIEW>
		void add_view(const char* label, field_app_t column_data, object_t object, const char* tooltip);
		// Enable widgets
		void enable_widgets();
		// the contained forms as views
		struct view_ptrs {
			view* v;
			Fl_Widget* w;
		};
		map<object_t, view_ptrs> forms_;
		// Last records updated
		qso_num_t last_record_1_;
		qso_num_t last_record_2_;
		// Last hint type (when switching between view) for query
		hint_t last_hint_;
		// Last book (switching in/out of different books) for query
		book* last_book_;
	};
#endif