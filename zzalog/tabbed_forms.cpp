#include "tabbed_forms.h"

#include "book.h"
#include "log_table.h"
#include "record_form.h"
#include "extract_data.h"
#include "import_data.h"
#include "pfx_tree.h"
#include "spec_tree.h"
#include "report_tree.h"
#include "toolbar.h"
#include "dxa_if.h"

using namespace zzalog;
using namespace zzalib;

extern book* book_;
extern book* navigation_book_;
extern extract_data* extract_records_;
extern import_data* import_data_;
extern toolbar* toolbar_;
extern dxa_if* dxatlas_;
extern bool closing_;

// Constructor
tabbed_forms::tabbed_forms(int X, int Y, int W, int H, const char* label) :
	Fl_Tabs(X, Y, W, H, label)
{	
	forms_.clear();
	widgets_.clear();
	labelsize(FONT_SIZE);
	// create the views -
	// Full log view - displays selected items of all records
	add_view<log_table>("Full log view", FO_MAINLOG, OT_MAIN, "Displays all log records");
	// Record view - displays all items of selected record, record selection within subset
	add_view<record_form>("Selected record", FO_QSOVIEW, OT_RECORD, "Displays all the fields and other info in selected record");
	// Imported data - view before merging
	add_view<log_table>("Records for import", FO_IMPORTLOG, OT_IMPORT, "Displays all the records currently being imported");
	// Extracted data - subset of records for search or exporting
	add_view<log_table>("Extracted records", FO_EXTRACTLOG, OT_EXTRACT, "Displays the records extracted according to the current criteria");
	// Prefix reference data
	add_view<pfx_tree>("Prefix Reference", FO_LAST, OT_PREFIX, "Displays the prefix reference data in tree format");
	// ADIF reference data
	add_view<spec_tree>("Specifications", FO_LAST, OT_ADIF, "Displays the ADIF specification data in tree format");
	// Report view
	add_view<report_tree>("Log analysis", FO_LAST, OT_REPORT, "Displays an analysis of the log in tree format");
	// Documented workround (see Fl_Tabs) to keep label heights constant
	resizable(widgets_[OT_MAIN]);
	// Set the callback for changing tabs
	callback(cb_tab_change);
	// Set the selected label colour the same as the view being selected
	selection_color(value()->color());
}

// Desctructor
tabbed_forms::~tabbed_forms()
{
}

// Template method to add a view of class VIEW, 
template <class VIEW>
void tabbed_forms::add_view(const char* label, field_ordering_t column_data, object_t object, const char* tooltip) {
	// Get the available area of the tabs. 
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	// Create the view
	VIEW* view = new VIEW(rx, ry, rw, rh, label, column_data);
	// label - a bit bigger than text font size
	view->labelsize(FONT_SIZE + 1);
	// standard colour used to represent this view - its tab and selected record/item
	Fl_Color bg_colour = OBJECT_COLOURS.at(object);
	view->selection_color(bg_colour);
	// The colour for undrawn parst of the view - 25% colour, 75% white
	view->color(fl_color_average(bg_colour, FL_WHITE, 0.25));
	// Draw the label text in a contrasting colour
	view->labelcolor(fl_contrast(FL_BLACK, bg_colour));
	// Add the tooltip
	view->tooltip(tooltip);
	// Add the view to the Fl_Tabs widget
	add(view);
	// map the object type to the particular instance of view and Fl_Widget (they inherit both)
	forms_[object] = view;
	widgets_[object] = view;
}

// tell all views and others that record(s) have changed
void tabbed_forms::update_views(view* requester, hint_t hint, record_num_t record_1, record_num_t record_2) {
	// Pass to each view in turn - note update() is a method in view.
	for (auto ix = forms_.begin() ; ix != forms_.end() && !closing_; ix++) {
		if (requester == (void*)0 || requester != ix->second) {
			ix->second->update(hint, record_1, record_2);
		}
	}
	if (record_1 != -1) {
		toolbar_->search_text(record_1);
	} 
#ifdef _WIN32
	if (dxatlas_) dxatlas_->update(hint);
#endif
}

// Activate or deactivate the named object
void tabbed_forms::activate_pane(object_t pane, bool active) {
	if (active) {
		// Switch to the specified view
		if (!widgets_[pane]->active()) {
			widgets_[pane]->activate();
		}
		value(widgets_[pane]);
		selection_color(value()->color());
	}
	else {
		// "Hide" the object by switching to the main log
		if (pane != OT_MAIN) {
			value(widgets_[OT_MAIN]);
			selection_color(value()->color());
		}
		widgets_[pane]->deactivate();
	}
}

// set the books into the various views
void tabbed_forms::books() {
	// For each view (as view)
	for (auto ix = forms_.begin(); ix != forms_.end(); ix++) {
		// Set the particular book into each view
		switch (ix->first) {
		case OT_MAIN:
			ix->second->set_book(book_);
			break;
		case OT_RECORD:
			ix->second->set_book(navigation_book_);
			break;
		case OT_EXTRACT:
			ix->second->set_book(extract_records_);
			break;
		case OT_IMPORT:
			ix->second->set_book(import_data_);
			break;
		}
	}
}

// Returns the specified view
view* tabbed_forms::get_view(object_t view_name) {
	return forms_[view_name];
}

// Callback - change selected colour when tab changes
void tabbed_forms::cb_tab_change(Fl_Widget* w, void* v) {
	Fl_Tabs* that = (Fl_Tabs*)w;
	that->selection_color(that->value()->color());
}

// Minimum width resizing
int tabbed_forms::min_w() {
	int min_w = 0;
	// For each view (as view) get its minimum width
	for (auto ix = forms_.begin(); ix != forms_.end(); ix++) {
		min_w = max(min_w, ix->second->min_w());
	}
	return min_w;
}

// Minimum height resizing
int tabbed_forms::min_h() {
	int min_h = 0;
	// For each view (as view) get its minimum height
	for (auto ix = forms_.begin(); ix != forms_.end(); ix++) {
		min_h = max(min_h, ix->second->min_h());
	}
	return min_h;
}