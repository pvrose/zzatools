#include "settings.h"
#include "rig_dialog.h"
#include "files_dialog.h"
#include "web_dialog.h"
#include "stn_dialog.h"
#include "fields_dialog.h"
#include "qsl_design.h"
#include "config_tree.h"

#include "../zzalib/utils.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Single_Window.H>

using namespace zzalog;
using namespace zzalib;

extern Fl_Single_Window* main_window_;
extern void add_sub_window(Fl_Window* w);
extern void remove_sub_window(Fl_Window* w);

// Constructor
settings::settings(int W, int H, const char* label, cfg_dialog_t active) :
	Fl_Window(W, H, label)
	, settings_view_(nullptr)
	, active_(true)
{
	updatable_views_.clear();
	// Create the set of tabs
	Fl_Tabs* tabs = new Fl_Tabs(0, 0, W, H - HBUTTON - GAP);
	tabs->labelsize(FONT_SIZE);
	border(true);
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	// get client area 
	tabs->client_area(rx, ry, rw, rh, 0);
	// Rig settings
	rig_dialog* rig = new rig_dialog(rx, ry, rw, rh, "Rig");
	rig->labelsize(FONT_SIZE);
	rig->selection_color(fl_lighter(FL_YELLOW));
	rig->tooltip("Allows the configuration of the rig interface; selection of app and its settings");
	// File location settings
	files_dialog* files = new files_dialog(rx, ry, rw, rh, "File Locations");
	files->labelsize(FONT_SIZE);
	files->selection_color(fl_lighter(FL_YELLOW));
	files->tooltip("Allows the specification of the locations of various resources used by zzalog");
	// Web URLs, user-names and passwords
	web_dialog* web = new web_dialog(rx, ry, rw, rh, "Web Info");
	web->labelsize(FONT_SIZE);
	web->selection_color(fl_lighter(FL_YELLOW));
	web->tooltip("Allows the setting of user details for the various on-line services");
	// Station settings: rig, aerial and QTH details
	stn_dialog* stn = new stn_dialog(rx, ry, rw, rh, "Station");
	stn->labelsize(FONT_SIZE);
	stn->selection_color(fl_lighter(FL_YELLOW));
	stn->tooltip("Allows the definition of the various fields describing station location");
	// Fields settings - fields to display as columns in log views, first few rows in record view
	// and fields to export to TSV files
	fields_dialog* fields = new fields_dialog(rx, ry, rw, rh, "Fields");
	fields->labelsize(FONT_SIZE);
	fields->selection_color(fl_lighter(FL_YELLOW));
	fields->tooltip("Allows the specification of which fields to display in the various applications");
	// QSL Design - allows user to modify the QSL design
	qsl_design* qsl = new qsl_design(rx, ry, rw, rh, "QSL Design");
	qsl->labelsize(FONT_SIZE);
	qsl->selection_color(fl_lighter(FL_YELLOW));
	qsl->tooltip("Allows the simple design of QSL labels");
	// Add to the list of updatable views
	updatable_views_.insert(qsl);

	// Lastly - a tree display showing all settings
	config_tree* all_settings = new config_tree(rx, ry, rw, rh, "All Settings");
	all_settings->labelsize(FONT_SIZE);
	all_settings->selection_color(fl_lighter(FL_RED));
	all_settings->tooltip("Displays the current settings in tree format");
	// Default to show all settings
	settings_view_ = all_settings;
	// Activate the required dialog
	switch (active) {
	case DLG_RIG:
		tabs->value(rig);
		break;
	case DLG_FILES:
		tabs->value(files);
		break;
	case DLG_WEB:
		tabs->value(web);
		break;
	case DLG_STATION:
		tabs->value(stn);
		break;
	case DLG_COLUMN:
		tabs->value(fields);
		break;
	case DLG_QSL:
		tabs->value(qsl);
		break;
	case DLG_ALL:
		tabs->value(all_settings);
		break;
	}
	tabs->end();
	// button - save the current settings and resume
	Fl_Button* save_bn = new Fl_Button(W - 3 * WBUTTON - 2 * GAP, H - HBUTTON, WBUTTON, HBUTTON, "Save");
	save_bn->labelsize(FONT_SIZE);
	save_bn->color(fl_lighter(fl_lighter(FL_BLUE)));
	save_bn->callback(cb_bn_cal, (long)CA_SAVE);
	save_bn->tooltip("Save changes and resume editing");
	add(save_bn);
	// button - save and close
	Fl_Return_Button* ok_bn = new Fl_Return_Button(W - 2 * WBUTTON - 1 * GAP, H - HBUTTON, WBUTTON, HBUTTON, "OK");
	ok_bn->labelsize(FONT_SIZE);
	ok_bn->color(FL_GREEN);
	ok_bn->callback(cb_bn_cal, (long)CA_OK);
	ok_bn->tooltip("Save changes and close dialog");
	add(ok_bn);
	// button - cancel last tab and close
	Fl_Button* cancel_bn = new Fl_Button(W - WBUTTON, H - HBUTTON, WBUTTON, HBUTTON, "Cancel");
	cancel_bn->labelsize(FONT_SIZE);
	cancel_bn->color(FL_RED);
	cancel_bn->callback(cb_bn_cal, (long)CA_CANCEL);
	cancel_bn->tooltip("Cancel changes and close dialog");
	add(cancel_bn);

	end();
	resizable(nullptr);

	callback(cb_bn_cal, (long)CA_CANCEL);

	show();
	add_sub_window(this);
}

// Destructor
settings::~settings()
{
	active_ = false;
}

// Callback - Save, OK or Cancel
void settings::cb_bn_cal(Fl_Widget* w, long arg) {
	// Find the active tab - assume that tabs is child 0
	settings* that = ancestor_view<settings>(w);
	Fl_Tabs* tabs = (Fl_Tabs*)that->child(0);
	Fl_Widget* active_tab = tabs->value();
	// Button selected
	switch ((cfg_action_t)arg) {
	case CA_OK:
		// Save values in active tab, close the config window
		active_tab->do_callback(active_tab);
		remove_sub_window(that);
		Fl::delete_widget(that);
		break;
	case CA_CANCEL:
		// Close the config window
		remove_sub_window(that);
		Fl::delete_widget(that);
		break;
	case CA_SAVE:
		// Save values in active tab - recreate all_settings view
		active_tab->do_callback(active_tab);
		((config_tree*)that->settings_view_)->create_tree();
		break;
	}
}

void settings::update() {
	for (auto it = updatable_views_.begin(); it != updatable_views_.end(); it++) {
		(*it)->update();
	}
}

bool settings::active() {
	return active_;
}

void settings::inactive() {
	active_ = false;
}
