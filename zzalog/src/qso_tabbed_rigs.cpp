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

qso_tabbed_rigs::qso_tabbed_rigs(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L)
{
	labeltype(FL_NO_LABEL);
	callback(cb_tabs);
	label_map_.clear();
	load_values();
	create_form(X, Y);
	enable_widgets();

}

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
		if (rig_dataset->data.find(name) != rig_dataset->data.end()) {
			label_map_[string(name)] = nullptr;
		}
	}
	if (label_map_.size() == 0) {
		label_map_["No Rig"] = nullptr;
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
	for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG: Creating rig connection for \"%s\"", (*ix).first.c_str());
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
	if (label() == nullptr || strlen(label()) == 0) {
		name = (*label_map_.begin()).first;
	}
	else {
		name = label();
	}
	printf("Selecting rig control %s\n", name.c_str());
	value(label_map_.at(name));
	for (auto ix = label_map_.begin(); ix != label_map_.end(); ix++) {
		Fl_Widget* w = (*ix).second;
		if (w == value()) {
			w->labelfont((w->labelfont() | FL_BOLD) & (~FL_ITALIC));
		}
		else {
			w->labelfont((w->labelfont() & (~FL_BOLD)) | FL_ITALIC);
		}
	}
}

// Save changes
void qso_tabbed_rigs::save_values() {
}

// Switch the rig on or off
void qso_tabbed_rigs::switch_rig() {
	string rig_name = label();
	if (label_map_.find(rig_name) == label_map_.end() || label_map_.at(rig_name) == nullptr) {
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
	} else {
		// It isn't the current selection so select it
		value(label_map_.at(rig_name));
	}
	enable_widgets();
}

void qso_tabbed_rigs::cb_tabs(Fl_Widget* w, void* v) {
	qso_tabbed_rigs* that = (qso_tabbed_rigs*)w;
	that->label(that->value()->label());
	that->enable_widgets();
}

// 1s clock interface
void qso_tabbed_rigs::ticker() {
	if (!closing_) {
		// Only send ticker to active rig
		((qso_rig*)value())->ticker();
	}
}

// Get the rig
rig_if* qso_tabbed_rigs::rig() {
	return ((qso_rig*)value())->rig();
}

#define BORDER 2
#define EXTRASPACE 10
#define SELECTION_BORDER 5

// Copy of Fl_Tabs - verbatim until noted otherwise
int qso_tabbed_rigs::tab_positions() {
	const int nc = children();
	if (nc != tab_count) {
		clear_tab_positions();
		if (nc) {
			tab_pos = (int*)malloc((nc + 1) * sizeof(int));
			tab_width = (int*)malloc((nc) * sizeof(int));
		}
		tab_count = nc;
	}
	if (nc == 0) return 0;
	int selected = 0;
	Fl_Widget* const* a = array();
	int i;
	char prev_draw_shortcut = fl_draw_shortcut;
	fl_draw_shortcut = 1;

	tab_pos[0] = Fl::box_dx(box());
	for (i = 0; i < nc; i++) {
		Fl_Widget* o = *a++;
		if (o->visible()) selected = i;

		int wt = 0; int ht = 0;
		Fl_Labeltype ot = o->labeltype();
		Fl_Align oa = o->align();
		if (ot == FL_NO_LABEL) {
			o->labeltype(FL_NORMAL_LABEL);
		}
		o->align(tab_align());
		o->measure_label(wt, ht);
		o->labeltype(ot);
		o->align(oa);

		tab_width[i] = wt + EXTRASPACE;
		tab_pos[i + 1] = tab_pos[i] + tab_width[i] + BORDER;
	}
	fl_draw_shortcut = prev_draw_shortcut;

	int r = w();
	if (tab_pos[i] <= r) return selected;

	// Change the algorithm if they are too big
	// Reduce all deselected widths by the apportioned oversize
	// Use fixed point arithmetic (x2^10) to reduce rounding error
	int delta = 1024 * (r - tab_width[selected]) / (tab_pos[i] - tab_width[selected]);
	for (i = 0; i < nc; i++) {
		if (i != selected) {
			tab_width[i] = tab_width[i] * delta / 1024;
		}
		tab_pos[i + 1] = tab_pos[i] + tab_width[i] + BORDER;
	}
	// If we haven't used the full width, increase the size of the last tab to compensate
	if (tab_pos[i] < r) {
		tab_width[i - 1] = r - tab_pos[i];
		tab_pos[i] = r;
	}
	return selected;
}
