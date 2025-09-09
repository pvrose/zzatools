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



	//! This class is a view with tabs that contain the specialised views
	class tabbed_forms : public Fl_Tabs
	{
	public:
		//! Construcor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		tabbed_forms(int X, int Y, int W, int H, const char* L = 0);
		//! Destructor.
		~tabbed_forms();

		//! tell all views that record(s) have changed
		 
		//! \param requester The view that made the change.
		//! \param hint An indication of what has changed.
		//! \param record_1 The index of the QSO record that has specifically ben modified.
		//! \param record_2 The index of an asscoiated QSO record. 
		void update_views(view* requester, hint_t hint, qso_num_t record_1, qso_num_t record_2 = 0);
		//! set the various books into the views
		void books();
		//! Returns the specific view with identifier \p view_name.
		view* get_view(object_t view_name);
		//! Minimum width for resizing.
		int min_w();
		//! Minimum height for resizing.
		int min_h();
		//! Activate/deactivate pane.
		
		//! \param pane Identifier of the tab to change active status.
		//! \param active If true make the identified tab active and diplayed.
		//! If false, make it inactive and displaye the full log viewing tab.
		void activate_pane(object_t pane, bool active);
		//! Callback when selected tab changes.
		static void cb_tab_change(Fl_Widget* w, void* v);

	protected:
		//! Add a view of class VIEW
		
		//! \param label The label of the view.
		//! \param column_data The fields to be displayed in the tab, where applicable.
		//! \param object An identifier for the tab.
		//! \param tooltip The tooltip displayed when hovering over the tab.
		template <class VIEW>
		void add_view(const char* label, field_app_t column_data, object_t object, const char* tooltip);
		//! Configure the tabs: change the label font used for the selected tab and unselected tabs.
		void enable_widgets();
		//! An entry that describes a tab as a view object and as a Fl_Widget object.
		struct view_ptrs {
			view* v;
			Fl_Widget* w;
		};
		//! Mapping the indeifier to the objects as views and Fl_Widgets.
		map<object_t, view_ptrs> forms_;
		//! Last QSO record index sent as record_1.
		qso_num_t last_record_1_;
		//! Last QSO record index sent as record_2.
		qso_num_t last_record_2_;
		//! Last hint type (when switching between view) for query
		hint_t last_hint_;
		//! Last book (switching in/out of different books) for query
		book* last_book_;
	};
#endif