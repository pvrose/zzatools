#include "tabbed_forms.h"

#include "book.h"
#include "log_table.h"
#include "extract_data.h"
#include "import_data.h"
#include "main.h"
#include "spec_tree.h"
#include "report_tree.h"
#include "toolbar.h"
#include "config.h"
#include "qso_manager.h"
#include "dxcc_view.h"

// Constructor
tabbed_forms::tabbed_forms(int X, int Y, int W, int H, const char* label) :
	Fl_Tabs(X, Y, W, H, label)
{	
	handle_overflow(OVERFLOW_PULLDOWN);
	forms_.clear();
	// create the views -
	// Full log view - displays selected items of all records
	add_view<log_table>("Full log view", FO_MAINLOG, OT_MAIN, "Displays all log records");
	// Imported data - view before merging
	add_view<log_table>("Records for import", FO_IMPORTLOG, OT_IMPORT, "Displays all the records currently being imported");
	// Extracted data - subset of records for search or exporting
	add_view<log_table>("Extracted records", FO_EXTRACTLOG, OT_EXTRACT, "Displays the records extracted according to the current criteria");
	// ADIF reference data
	add_view<spec_tree>("Specifications", FO_LAST, OT_ADIF, "Displays the ADIF specification data in tree format");
	// Report view
	add_view<report_tree>("Log analysis", FO_LAST, OT_REPORT, "Displays an analysis of the log in tree format");
	// DXCC Status report
	add_view<dxcc_view>("DXCC Status", FO_LAST, OT_DXCC, "Displays an analysis of DXCC status in table form");
	// Documented workround (see Fl_Tabs) to keep label heights constant
	resizable(forms_[OT_MAIN].w);
	// Set the callback for changing tabs
	callback(cb_tab_change);
	// Set the selected label colour the same as the view being selected
	// selection_color(value()->color());
	enable_widgets();

	end();
}

// Desctructor
tabbed_forms::~tabbed_forms()
{
}

// Template method to add a view of class VIEW, 
template <class VIEW>
void tabbed_forms::add_view(const char* label, field_app_t column_data, object_t object, const char* tooltip) {
	// Get the available area of the tabs. 
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	Fl_Group* container = new Fl_Group(rx, ry, rw, rh, label);
	container->labelsize(FL_NORMAL_SIZE + 2);
	// Create the view
	VIEW* view = new VIEW(rx, ry, rw, rh, label, column_data);
	// standard colour used to represent this view - its tab, selected record/item and progress bar
	Fl_Color bg_colour = OBJECT_COLOURS.at(object);
	view->selection_color(bg_colour);
	// Add the tooltip
	view->tooltip(tooltip);
	// Add the view to the Fl_Tabs widget
	container->add(view);
	// std::map the object type to the particular instance of view and Fl_Widget (they inherit both)
	add(container);
	forms_[object] = { view, view };
}

// tell all views and others that record(s) have changed and why
void tabbed_forms::update_views(view* requester, hint_t hint, qso_num_t record_1, qso_num_t record_2) {
	if (hint != HT_IGNORE) {
		// Remeber the records to update non-visible views when they become visible
		last_record_1_ = record_1;
		last_record_2_ = record_2;
		last_hint_ = hint;
		last_book_ = navigation_book_;
		// Pass to each view in turn - note update() is a method in view.
		for (auto fx = forms_.begin(); fx != forms_.end() && !closing_; fx++) {
			// If the requesting view is this view don't update it, it will have done its own updating
			if (requester == (void*)0 || requester != fx->second.v) {
				switch (fx->first) {
				case OT_MAIN:
				case OT_EXTRACT:
					// Always update these views
					fx->second.v->update(hint, record_1, record_2);
					break;
				default:
					// Only update a view if it's the current selected one
					if (fx->second.w == value()) {
						fx->second.v->update(hint, record_1, record_2);
					}
					break;
				}
			}
		}
		// Update all the other objects that use the current selection
		if (record_1 != -1) {
			// Add the current record's callsign to the search box in the tool bar
			toolbar_->search_text(record_1);
		}
		// Update the settngs config dialog
		if (config_) config_->update();
		// Update qso_manager - avoid recursion
		if (qso_manager_) qso_manager_->update_qso(hint, record_1, record_2);
	}
}

// Activate or deactivate the named object - if selecting another log_view change the navigation_book_
void tabbed_forms::activate_pane(object_t pane, bool active) {
	view* v = forms_[pane].v;
	Fl_Widget* w = forms_[pane].w;
	Fl_Group* g = w->parent();
	if (active) {
		// Make the pane active

		if (!g->active()) {
			g->activate();
		}
		// Update and Select the pane
		value(g);
		// selection_color(value()->color());
		// w->labelcolor(FL_BLACK);
		switch (pane) {
		case OT_MAIN:
			navigation_book_ = book_;
			break;
		case OT_EXTRACT:
			navigation_book_ = extract_records_;
			break;
		case OT_IMPORT:
			navigation_book_ = import_data_;
			break;
		default:
			break;
		}
		// Restore any query
		v->update(last_hint_, last_record_1_, last_record_2_);
		g->redraw();
		w->redraw();
		Fl::check();
	}
	else {
		// "Hide" the object by switching to the main log
		if (pane != OT_MAIN) {
			Fl_Widget* wm = forms_[OT_MAIN].w;
			Fl_Group* gm = wm->parent();
			value(gm);
			// selection_color(value()->color());
			navigation_book_ = book_;
		}
		g->deactivate();
	}
	enable_widgets();
}

// std::set the books into the various views
void tabbed_forms::books() {
	// For each view (as view)
	for (auto ix = forms_.begin(); ix != forms_.end(); ix++) {
		view* v = ix->second.v;
		// Set the particular book into each view
		switch (ix->first) {
		case OT_MAIN:
			v->set_book(book_);
			break;
		case OT_EXTRACT:
			v->set_book(extract_records_);
			break;
		case OT_IMPORT:
			v->set_book(import_data_);
			break;
		default:
			break;
		}
	}
}

// Returns the specified view
view* tabbed_forms::get_view(object_t view_name) {
	return forms_[view_name].v;
}

// Callback - change selected colour when tab changes
// v is unused
void tabbed_forms::cb_tab_change(Fl_Widget* w, void* v) {
	tabbed_forms* that = (tabbed_forms*)w;
	that->enable_widgets();

	log_table* table = dynamic_cast<log_table*>(that->value());
	if (table) {
		// The pane is a log book viewer. Change the navigation controls to affect that pane
		switch (table->get_book()->book_type()) {
		case OT_MAIN:
			navigation_book_ = book_;
			break;
		case OT_EXTRACT:
			navigation_book_ = extract_records_;
			break;
		case OT_IMPORT:
			navigation_book_ = import_data_;
			break;
		default:
			break;
		}
	}
	view* as_view = dynamic_cast<view*>(((Fl_Group*)that->value())->child(0));
	as_view->update(that->last_hint_, that->last_record_1_, that->last_record_2_);
}

// Minimum width resizing - sets to the largest minimum width of all the panes
int tabbed_forms::min_w() {
	int result = 0;
	// For each view (as view) get its minimum width
	for (auto ix = forms_.begin(); ix != forms_.end(); ix++) {
		result = std::max<int>(result, ix->second.v->min_w());
	}
	return result;
}

// Minimum height resizing - sets to the larges minimum height of all the panes
int tabbed_forms::min_h() {
	int result = 0;
	// For each view (as view) get its minimum height
	for (auto ix = forms_.begin(); ix != forms_.end(); ix++) {
		result = std::max<int>(result, ix->second.v->min_h());
	}
	return result;
}

// Enable widgets
void tabbed_forms::enable_widgets() {
	// value() returns the selected widget. We need to test which widget it is.
	Fl_Widget* tab = value();
	// Set the application standard format for labels in tabbed items
	for (int ix = 0; ix < children(); ix++) {
		Fl_Widget* wx = child(ix);
		if (wx == tab) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
	}
}
