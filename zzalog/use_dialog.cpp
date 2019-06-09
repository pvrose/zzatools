#include "use_dialog.h"
#include "callback.h"
#include "utils.h"
#include "spec_data.h"


#include <FL/Fl_Preferences.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>

using namespace zzalog;

extern Fl_Preferences* settings_;
extern spec_data* spec_data_;
extern void add_rig_if();

// Constructor - initially sets a default size for the window which is resized once we know how big it needs to be
use_dialog::use_dialog(use_dialog_t type) :
	win_dialog(10, 10)
	, choice_(nullptr)
	, type_(type)
	, setting_path_("")
	, item_name_("")
	, current_value_("")
	, display_all_(false)

{
	// Set the item type and set attributes accordingly
	all_items_.clear();
	labelsize(FONT_SIZE);
	switch (type_)
	{
	case UD_RIG:
		setting_path_ = "Rigs";
		item_name_ = "rig";
		label("Select Current Rig");
		break;
	case UD_AERIAL:
		setting_path_ = "Aerials";
		item_name_ = "aerial";
		label("Select Current Aerial");
		break;
	case UD_QTH:
		setting_path_ = "QTHs";
		item_name_ = "QTH";
		label("Select Current QTH");
		break;
	case UD_PROP:
		setting_path_ = "Propagation Mode";
		item_name_ = "propagation mode";
		label("Select Current Propagation Mode");
		break;
	}
	// Load data from settings and create dialog
	load_data();
	create_form();

	callback(cb_bn_cancel);
}

// Destructor
use_dialog::~use_dialog()
{
}

// OK button callback - save data and close dialog
void use_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	use_dialog* that = ancestor_view<use_dialog>(w);
	that->save_data();
	that->do_button(BN_OK);

}

// Cancel button callback - cloase dialog without saving
void use_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	use_dialog* that = ancestor_view<use_dialog>(w);
	that->do_button(BN_CANCEL);
}

// callback to reload choice on check display all
void use_dialog::cb_bn_all(Fl_Widget* w, void* v) {
	use_dialog* that = ancestor_view<use_dialog>(w);
	// Get value of display all button and reload choice
	that->display_all_ = ((Fl_Check_Button*)w)->value();
	that->load_choice();
	that->redraw();
}

// Get data from settings
void use_dialog::load_data() {
	// Get settings
	Fl_Preferences station_settings(settings_, "Stations");
	Fl_Preferences use_settings(station_settings, setting_path_.c_str());
	char* temp;
	// Get current item
	use_settings.get("Current", temp, "");
	current_value_ = string(temp);
	free(temp);
	if (type_ != UD_PROP) {
		// Get whether to display all
		use_settings.get("Display All", (int&)display_all_, false);
		int num_items = use_settings.groups();
		all_items_.clear();
		// For each item
		for (int i = 0; i < num_items; i++) {
			// Gets its name and activity status
			string name = use_settings.group(i);
			Fl_Preferences item_settings(use_settings, name.c_str());
			int active;
			item_settings.get("Active", active, (int)false);
			all_items_[string(name)] = (bool)active;
		}
	}
}

// Create the dialog
void use_dialog::create_form() {
	// Calculate widget positions
	const int C1 = EDGE;
	const int W1 = WRADIO;
	const int C2 = C1 + W1 + WLABEL + GAP;
	const int W2 = WSMEDIT;
	const int WALL = C2 + W2 + EDGE;
	const int W4 = WBUTTON;
	const int C4 = WALL - GAP - W4;
	const int W3 = WBUTTON;
	const int C3 = C4 - GAP - W3;
	const int R1 = EDGE;
	const int H1 = max(HBUTTON, HTEXT);
	const int R2 = R1 + H1 + GAP;
	const int H2 = HBUTTON;
	const int HALL = R2 + H2 + EDGE;

	// Resize dialog to fit all widgets
	size(WALL, HALL);

	// Display all check button
	Fl_Check_Button* bn11 = new Fl_Check_Button(C1, R1, W1, H1, "Display All");
	bn11->labelsize(FONT_SIZE);
	bn11->align(FL_ALIGN_RIGHT);
	bn11->value(display_all_);
	bn11->callback(cb_bn_all);
	bn11->when(FL_WHEN_RELEASE);
	char temp[128];
	sprintf(temp, "Display active and inactive %ss", item_name_.c_str());
	bn11->copy_tooltip(temp);
	// Choice of items
	Fl_Choice* ch12 = new Fl_Choice(C2, R1, W2, H1);
	ch12->textsize(FONT_SIZE);
	choice_ = ch12;
	load_choice();
	ch12->callback(cb_text<Fl_Choice, string>, (void*)&current_value_);
	ch12->when(FL_WHEN_RELEASE);
	sprintf(temp, "Select %s to use", item_name_.c_str());
	ch12->copy_tooltip(temp);
	if (type_ == UD_PROP) {
		spec_data_->initialise_field_choice(ch12, "Propagation_Mode");
	}
	else {
		for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
			ch12->add((it->first).c_str());
		}
	}
	// OK Button
	Fl_Button* bn23 = new Fl_Button(C3, R2, W3, H2, "OK");
	bn23->labelsize(FONT_SIZE);
	bn23->callback(cb_bn_ok);
	bn23->when(FL_WHEN_RELEASE);
	bn23->color(fl_lighter(FL_GREEN));
	sprintf(temp, "Use %s", item_name_.c_str());
	bn23->copy_tooltip(temp);
	// Cancel button
	Fl_Button* bn24 = new Fl_Button(C4, R2, W4, H2, "Cancel");
	bn24->labelsize(FONT_SIZE);
	bn24->callback(cb_bn_cancel);
	bn24->when(FL_WHEN_RELEASE);
	bn24->color(fl_lighter(FL_RED));
	sprintf(temp, "Cancel %s selection", item_name_.c_str());

	end();
}

// Add item names to the item selector
void use_dialog::load_choice() {
	Fl_Choice* ch = (Fl_Choice*)choice_;
	int ix = 0;
	// For all items
	for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
		if (it->second || display_all_) {
			// Add the item name to the choice
			ch->add(it->first.c_str());
			// If it's the current value then select it in the choice
			if (it->first == current_value_) {
				ch->value(ix);
			}
			ix++;
		}
	}
}

// Write data back to settings
void use_dialog::save_data() {
	// Get settings
	Fl_Preferences station_settings(settings_, "Stations");
	Fl_Preferences use_settings(station_settings, setting_path_.c_str());
	// Set current value and display all flag
	use_settings.set("Current", current_value_.c_str());
	if (type_ != UD_PROP) {
		use_settings.set("Display All", display_all_);
	}
	// If we are using a new rig connect to it.
	if (type_ == UD_RIG) {
		add_rig_if();
	}
}
