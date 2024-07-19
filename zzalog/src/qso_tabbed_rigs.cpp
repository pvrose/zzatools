#include "qso_tabbed_rigs.h"
#include "qso_rig.h"
#include "qso_manager.h"
#include "spec_data.h"
#include "status.h"

#include <FL/Fl_Preferences.H>


extern Fl_Preferences* settings_;
extern spec_data* spec_data_;
extern status* status_;
extern bool closing_;

// Constructor for the rigs set of tabs
qso_tabbed_rigs::qso_tabbed_rigs(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L)
{
	labeltype(FL_NO_LABEL);
	box(FL_BORDER_BOX);
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
	// Get existing rig names from spec_data
	spec_dataset* rig_dataset = spec_data_->dataset("Dynamic MY_RIG");
	Fl_Preferences cat_settings(settings_, "CAT");
	for (int g = 0; g < cat_settings.groups(); g++) {
		const char* name = cat_settings.group(g);
		// If the names is both in settings and in the list read from the log add it to the list
		if (rig_dataset && rig_dataset->data.find(name) != rig_dataset->data.end()) {
			label_map_[string(name)] = nullptr;
		}
	}
	if (label_map_.size() == 0) {
		label_map_[string("")] = nullptr;
	}
}

// Create form
void qso_tabbed_rigs::create_form(int X, int Y) {
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;
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
	size(w() + rw - saved_rw, h() + rh - saved_rh);
	end();
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