#include "qso_tabbed_rigs.h"
#include "qso_rig.h"
#include "qso_manager.h"
#include "rig_data.h"
#include "spec_data.h"
#include "status.h"

#include <FL/Fl_Preferences.H>

extern rig_data* rig_data_;
extern spec_data* spec_data_;
extern status* status_;
extern bool closing_;
extern string VENDOR;
extern string PROGRAM_ID;

// Constructor for the rigs set of tabs
qso_tabbed_rigs::qso_tabbed_rigs(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L)
{
	labeltype(FL_NO_LABEL);
	box(FL_BORDER_BOX);
	handle_overflow(OVERFLOW_PULLDOWN);
	callback(cb_tabs);
	label_map_.clear();
	load_values();
	create_form(X, Y);
	enable_widgets();
}

// Destructor
qso_tabbed_rigs::~qso_tabbed_rigs() {
	for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
		delete ((qso_rig*)(*ix).second);
	}
	save_values();
}

// get settings - find what we've created in the settings
void qso_tabbed_rigs::load_values() {
	// Get existing rig names from spec_data - as seen in log
	spec_dataset* rig_dataset = spec_data_->dataset("Dynamic MY_RIG");
	// Ger the list of rigs as seen in rig.xml
	vector<string> rigs = rig_data_->rigs();
	for (auto it : rigs) {
		// If the rig is in both lists
		rig_data_t* rig_info = rig_data_->get_rig(it);
		if (rig_info->default_app >= 0 && rig_dataset->data.find(it) != rig_dataset->data.end()) {
			label_map_[string(it)] = nullptr;
		}
	}
	// Load default tab value
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	tab_settings.get("Rigs", default_tab_, 0);
}

// Create form
void qso_tabbed_rigs::create_form(int X, int Y) {
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	int delta_w = w() - rw;
	int delta_h = h() - rh;
	if (label_map_.size() == 0) {
		// Create a dummy instance of qao_rig to get its size
		qso_rig* w = new qso_rig(rx, ry, rw, rh, "");
		rw = w->w();
		rh = w->h();
		// And delete it
		// remove(w);
	}
	// For each currently salient rig...
	for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG: Creating rig interface for \"%s\"", (*ix).first.c_str());
		status_->misc_status(ST_NOTE, msg);
		// Create a version of qso_rig 
		qso_rig* w = new qso_rig(rx, ry, rw, rh, (*ix).first.c_str());
		// TODO The following code relies on a later version of FLTK 1.4
		//w->when(FL_WHEN_CLOSED);
		add(w);
		(*ix).second = w;
		// All versions of qso_rig should be the same size, but...
		rw = max(rw, w->w());
		rh = max(rh, w->h());
	}
	resizable(nullptr);
	size(rw + delta_w, rh + delta_h);
	end();
	show();

	if (children() > default_tab_) value(child(default_tab_));

}

// Enable widgets
void qso_tabbed_rigs::enable_widgets() {
	string name;
	if (label_map_.size() > 0) {
		if (label() == nullptr || strlen(label()) == 0) {
			name = (*label_map_.begin()).first;
		}
		else {
			name = label();
		}
		value(label_map_.at(name));
		// Set standard tab label format
		for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
			Fl_Widget* w = (*ix).second;
			if (w == value()) {
				w->labelfont((w->labelfont() | FL_BOLD) & (~FL_ITALIC));
				w->labelcolor(FL_FOREGROUND_COLOR);
			}
			else {
				w->labelfont((w->labelfont() & (~FL_BOLD)) | FL_ITALIC);
				w->labelcolor(FL_FOREGROUND_COLOR);
			}
		}
	}
}

// Save changes
void qso_tabbed_rigs::save_values() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	// Find the current selected tab and save its index
	Fl_Widget* w = value();
	for (int ix = 0; ix != children(); ix++) {
		if (child(ix) == w) {
			tab_settings.set("Rigs", ix);
		}
	}
}

// Switch to the selected rig
void qso_tabbed_rigs::switch_rig() {
	string rig_name = label();
	if (rig_name.length()) {
		if (label_map_.find(rig_name) == label_map_.end() || label_map_.at(rig_name) == nullptr) {
			if (label_map_.size() == 1 && (*label_map_.begin()).first == string("")) {
				// Place holder null string - delete it and its reference
				delete (qso_rig*)(*label_map_.begin()).second;
				label_map_.erase(string(""));
			}
			// Rig does not yet exist, create it and select it
			int rx = 0;
			int ry = 0;
			int rw = 0;
			int rh = 0;
			client_area(rx, ry, rw, rh, 0);
			qso_rig* w = new qso_rig(rx, ry, rw, rh, label());
			add(w);
			label_map_[rig_name] = w;
			value(w);
		}
		else if (label_map_.at(rig_name) == value()) {
			// It exists and is current selection, switch its state
			((qso_rig*)value())->switch_rig();
		}
		else {
			// It isn't the current selection so select it
			value(label_map_.at(rig_name));
		}
	}
	enable_widgets();
}

// Callback when changing tab - causes the dashboard to use the selection as 
// the default rig
// v is not used
void qso_tabbed_rigs::cb_tabs(Fl_Widget* w, void* v) {
	qso_tabbed_rigs* that = (qso_tabbed_rigs*)w;
	that->label(that->value()->label());
	that->enable_widgets();
	qso_manager* mgr = ancestor_view<qso_manager>(that);
	mgr->update_rig();
}

// Get the rig
rig_if* qso_tabbed_rigs::rig() {
	// No child tabs so return no rig
	if (children() == 0) return nullptr;
	// Otherwise return the rig in the selected tab
	return ((qso_rig*)value())->rig();
}

// Deactivae all rigs
void qso_tabbed_rigs::deactivate_rigs() {
	while (children()) {
		qso_rig* rig = (qso_rig*)child(0);
		rig->disconnect();
		delete_child(0);
	}
	label_map_.clear();
	load_values();
	begin();
	create_form(x(), y());
}