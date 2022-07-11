#include "settings.h"
#include "qso_manager.h"
#include "files_dialog.h"
#include "web_dialog.h"
#include "fields_dialog.h"
#include "qsl_design.h"
#include "user_dialog.h"
#include "config_tree.h"

#include "../zzalib/utils.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Tabs.H>

using namespace zzalog;
using namespace zzalib;

// Constructor
settings::settings(int W, int H, const char* label, cfg_dialog_t active) :
	Fl_Window(W, H, label)
	, settings_view_(nullptr)
	, active_(true)
{
	updatable_views_.clear();
	// Create the set of tabs - leave enough space beneath for OK etc buttons.
	Fl_Tabs* tabs = new Fl_Tabs(0, 0, W, H - HBUTTON - GAP);
	tabs->labelsize(FONT_SIZE);
	tabs->callback(cb_tab);
	border(true);
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	// get client area 
	tabs->client_area(rx, ry, rw, rh, 0);
	// File location settings
	files_dialog* files = new files_dialog(rx, ry, rw, rh, "File Locations");
	files->labelsize(FONT_SIZE);
	files->selection_color(fl_lighter(FL_YELLOW));
	files->tooltip("Allows the specification of the locations of various resources used by zzalog");
	// Web URLs, user-names and passwords
	web_dialog* aweb = new web_dialog(rx, ry, rw, rh, "Web Info");
	aweb->labelsize(FONT_SIZE);
	aweb->selection_color(fl_lighter(FL_YELLOW));
	aweb->tooltip("Allows the setting of user details for the various on-line services");
	// Fields settings - fields to display as columns in log views, first few rows in record view
	// and fields to export to TSV files
	fields_dialog* fields = new fields_dialog(rx, ry, rw, rh, "Fields");
	fields->labelsize(FONT_SIZE);
	fields->selection_color(fl_lighter(FL_YELLOW));
	fields->tooltip("Allows the specification of which fields to display in the various applications");
	// User settings - allows user to control cetain aspects of the displayed information
	user_dialog* user = new user_dialog(rx, ry, rw, rh, "User settings");
	user->labelsize(FONT_SIZE);
	user->selection_color(fl_lighter(FL_YELLOW));
	user->tooltip("Allows limited configuration of fonts and tip timeouts");

	// Lastly - a tree display showing all settings
	config_tree* all_settings = new config_tree(rx, ry, rw, rh, "All Settings");
	all_settings->labelsize(FONT_SIZE);
	all_settings->selection_color(fl_lighter(FL_RED));
	all_settings->tooltip("Displays the current settings in tree format");
	// Default to show all settings
	settings_view_ = all_settings;
	// Activate the required dialog
	switch (active) {
	case DLG_FILES:
		tabs->value(files);
		break;
	case DLG_WEB:
		tabs->value(aweb);
		break;
	case DLG_COLUMN:
		tabs->value(fields);
		break;
	case DLG_USER:
		tabs->value(user);
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
	callback(cb_bn_cal, (long)CA_CANCEL);
	set_label(active);

	resizable(nullptr);

	end();
	show();
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
		Fl::delete_widget(that);
		break;
	case CA_CANCEL:
		// Close the config window
		Fl::delete_widget(that);
		break;
	case CA_SAVE:
		// Save values in active tab - recreate all_settings view
		active_tab->do_callback(active_tab);
		((config_tree*)that->settings_view_)->create_tree();
		break;
	}
}

// Callback on changing tab
void settings::cb_tab(Fl_Widget* w, void* v) {
	settings* that = ancestor_view<settings>(w);
	Fl_Tabs* tabs = (Fl_Tabs*)that->child(0);
	// value() returns the selected widget. We need to test which widget it is.
	Fl_Widget* tab = tabs->value();
	cfg_dialog_t ix;
	if (dynamic_cast<files_dialog*>(tab)) ix = DLG_FILES;
	else if (dynamic_cast<web_dialog*>(tab)) ix = DLG_WEB;
	else if (dynamic_cast<fields_dialog*>(tab)) ix = DLG_COLUMN;
	else if (dynamic_cast<user_dialog*>(tab)) ix = DLG_USER;
	else if (dynamic_cast<config_tree*>(tab)) ix = DLG_ALL;
	else ix = DLG_X;
	// Change the label of the tabs to the appropriate value for the selected widget
	that->set_label(ix);
}

void settings::set_label(settings::cfg_dialog_t active) {
	switch (active) {
	case DLG_FILES:
		label("Configuration: Define location of various data files");
		break;
	case DLG_WEB:
		label("Configuration: Define web locations of QSL and other services");
		break;
	case DLG_COLUMN:
		label("Configuration: Define the fields to be displayed in various views");
		break;
	case DLG_USER:
		label("Configuration: Define the way certain items are viewed");
		break;
	case DLG_ALL:
		label("Configuration: Display all options in tree format");
		break;
	case DLG_X:
		label("Configuration: *** UNKNOWN ***");
		break;
	}
}

// Update all tabs that are selected record dependant
void settings::update() {
	for (auto it = updatable_views_.begin(); it != updatable_views_.end(); it++) {
		(*it)->update();
	}
}

// Return active flag
bool settings::active() {
	return active_;
}

// Clear active flag
void settings::inactive() {
	active_ = false;
}
