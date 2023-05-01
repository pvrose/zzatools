#include "qso_tabbed_qth.h"
#include "qso_qth.h"

#include "spec_data.h"

extern spec_data* spec_data_;

qso_tabbed_qth::qso_tabbed_qth(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L)
{
	labeltype(FL_NO_LABEL);
	callback(cb_tabs);
	label_map_.clear();
	load_values();
	create_form(X, Y);
	enable_widgets();

}

qso_tabbed_qth::~qso_tabbed_qth() {
	for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
		delete ((qso_qth*)(*ix).second);
	}
	save_values();
}

// get settings - find what we've created in the settings
void qso_tabbed_qth::load_values() {
	// Get existing rig names from spec_data
	spec_dataset* qth_dataset = spec_data_->dataset("Macro APP_ZZA_QTH");
	for (auto it = qth_dataset->data.begin(); it != qth_dataset->data.end(); it++) {
		label_map_[(*it).first] = nullptr;
	}
}

// Create form
void qso_tabbed_qth::create_form(int X, int Y) {
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;
	for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
		// Create a version of qso_rig 
		qso_qth* w = new qso_qth(rx, ry, rw, rh, (*ix).first.c_str());
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
void qso_tabbed_qth::enable_widgets() {
	string name;
	if (label_map_.size() > 0) {
		if (label() == nullptr || strlen(label()) == 0) {
			name = (*label_map_.begin()).first;
		}
		else {
			name = label();
		}
		printf("Selecting QTH %s\n", name.c_str());
		qso_qth* q = (qso_qth*)label_map_.at(name);
		value(q);
		for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
			Fl_Widget* w = (*ix).second;
			if (w == value()) {
				w->labelfont((w->labelfont() | FL_BOLD) & (~FL_ITALIC));
			}
			else {
				w->labelfont((w->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			}
		}
		q->enable_widgets();
	}
}

// Save changes
void qso_tabbed_qth::save_values() {
}

void qso_tabbed_qth::cb_tabs(Fl_Widget* w, void* v) {
	qso_tabbed_qth* that = (qso_tabbed_qth*)w;
	that->label(that->value()->label());
	that->enable_widgets();
}

// Switch the qth on or off
void qso_tabbed_qth::switch_qth() {
	string qth_name = label();
	if (qth_name.length()) {
		if (label_map_.find(qth_name) == label_map_.end() || label_map_.at(qth_name) == nullptr) {
			if (label_map_.size() == 1 && (*label_map_.begin()).first == string("")) {
				// Place holder null string - delete it and its reference
				delete (qso_qth*)(*label_map_.begin()).second;
				label_map_.erase(string(""));
			}
			// Rig does not yet exist, create it and select it
			int rx = 0;
			int ry = 0;
			int rw = 0;
			int rh = 0;
			client_area(rx, ry, rw, rh, 0);
			qso_qth* w = new qso_qth(rx, ry, rw, rh, label());
			add(w);
			label_map_[qth_name] = w;
			value(w);
		}
		else if (label_map_.at(qth_name) == value()) {
		}
		else {
			// It isn't the current selection so select it
			value(label_map_.at(qth_name));
		}
	}
	enable_widgets();
}

