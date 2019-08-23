#ifndef __TABBED_FORMS__
#define __TABBED_FORMS__

#include "view.h"
#include "book.h"
#include "fields.h"
#include "drawing.h"

#include <vector>
#include <map>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Widget.H>

namespace zzalog {

	// This class is a view with tabs that contain the specialised views
	class tabbed_forms : public Fl_Tabs
	{
	public:
		tabbed_forms(int X, int Y, int W, int H, const char* label = 0);
		~tabbed_forms();

		// tell all views that record(s) have changed
		void update_views(view* requester, hint_t hint, record_num_t record_1, record_num_t record_2 = 0);
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
		void add_view(const char* label, field_ordering_t column_data, object_t object, const char* tooltip);
		// the contained forms as views
		map<object_t, view*> forms_;
		// The contained forms as widgets
		map<object_t, Fl_Widget*> widgets_;
	};

}
#endif