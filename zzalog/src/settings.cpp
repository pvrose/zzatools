#include "settings.h"
#include "files_dialog.h"
#include "web_dialog.h"
#include "fields_dialog.h"
#include "user_dialog.h"
#include "config_tree.h"

#include "utils.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Preferences.H>

extern Fl_Preferences* settings_;

// Constructor
settings::settings(int W, int H, const char* label, cfg_dialog_t active) :
	Fl_Window(W, H, label)
	, settings_view_(nullptr)
	, active_(true)
{
	updatable_views_.clear();

	// Set position on screen
	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences window_settings(windows_settings, "Settings");
	int left, top;
	window_settings.get("Left", left, 0);
	window_settings.get("Top", top, 100);
	position(left, top);

	children_ids_.clear();
	// Create the set of tabs - leave enough space beneath for OK etc buttons.
	Fl_Tabs* tabs = new Fl_Tabs(0, 0, W, H - HBUTTON - GAP);
	tabs->callback(cb_tab);
	tabs->box(FL_FLAT_BOX);
	border(true);
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	// get client area 
	tabs->client_area(rx, ry, rw, rh, 0);
	// File location settings
	files_dialog* files = new files_dialog(rx, ry, rw, rh, "File Locations");
	files->labelfont(FL_BOLD);
	files->labelsize(FL_NORMAL_SIZE + 2);
	files->tooltip("Allows the specification of the locations of various resources used by zzalog");
	children_ids_.push_back(DLG_FILES);
	// Web URLs, user-names and passwords
	web_dialog* aweb = new web_dialog(rx, ry, rw, rh, "Web/Network");
	aweb->labelfont(FL_BOLD);
	aweb->labelsize(FL_NORMAL_SIZE + 2);
	aweb->tooltip("Allows the setting of user details for the various on-line services");
	children_ids_.push_back(DLG_WEB);
	// Fields settings - fields to display as columns in log views, first few rows in record view
	// and fields to export to TSV files
	fields_dialog* fields = new fields_dialog(rx, ry, rw, rh, "Fields");
	fields->labelfont(FL_BOLD);
	fields->labelsize(FL_NORMAL_SIZE + 2);
	fields->tooltip("Allows the specification of which fields to display in the various applications");
	children_ids_.push_back(DLG_FORMAT);
	// User settings - allows user to control cetain aspects of the displayed information
	user_dialog* user = new user_dialog(rx, ry, rw, rh, "User settings");
	user->labelfont(FL_BOLD);
	user->labelsize(FL_NORMAL_SIZE + 2);
	user->tooltip("Allows limited configuration of fonts and tip timeouts");
	children_ids_.push_back(DLG_USER);

	// Lastly - a tree display showing all settings
	Fl_Group* all_settings = new Fl_Group(rx, ry, rw, rh, "All Settings");
	all_settings->labelfont(FL_BOLD);
	all_settings->labelsize(FL_NORMAL_SIZE + 2);
	all_settings->tooltip("Displays the current settings in tree format");
	children_ids_.push_back(DLG_ALL);

	config_tree* all_tree = new config_tree(rx, ry, rw, rh);

	all_settings->end();

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
	save_bn->color(fl_lighter(fl_lighter(FL_BLUE)));
	save_bn->callback(cb_bn_cal, (long)CA_SAVE);
	save_bn->labelcolor(FL_BLACK);
	save_bn->tooltip("Save changes and resume editing");
	add(save_bn);
	// button - save and close
	Fl_Return_Button* ok_bn = new Fl_Return_Button(W - 2 * WBUTTON - 1 * GAP, H - HBUTTON, WBUTTON, HBUTTON, "OK");
	ok_bn->color(FL_GREEN);
	ok_bn->callback(cb_bn_cal, (long)CA_OK);
	ok_bn->tooltip("Save changes and close dialog");
	ok_bn->labelcolor(FL_BLACK);
	add(ok_bn);
	// button - cancel last tab and close
	Fl_Button* cancel_bn = new Fl_Button(W - WBUTTON, H - HBUTTON, WBUTTON, HBUTTON, "Cancel");
	cancel_bn->color(FL_RED);
	cancel_bn->callback(cb_bn_cal, (long)CA_CANCEL);
	cancel_bn->tooltip("Cancel changes and close dialog");
	add(cancel_bn);
	callback(cb_bn_cal, (long)CA_CANCEL);
	set_label(active);

	resizable(nullptr);

	end();
	show();

	enable_widgets();
}

// Destructor
settings::~settings()
{
	active_ = false;
	// Rememeber window position
	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences window_settings(windows_settings, "Settings");
	window_settings.set("Left", x_root());
	window_settings.set("Top", y_root());
}

// Enable widgets
void settings::enable_widgets() {
	Fl_Tabs* tabs = (Fl_Tabs*)child(0);
	// value() returns the selected widget. We need to test which widget it is.
	Fl_Widget* tab = tabs->value();
	cfg_dialog_t dlg = DLG_X;
	for (int ix = 0; ix < tabs->children(); ix++) {
		Fl_Widget* wx = tabs->child(ix);
		if (wx == tab) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
			dlg = children_ids_[ix];
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(COLOUR_GREY);
		}
	}
	set_label(dlg);
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
	that->enable_widgets();
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
