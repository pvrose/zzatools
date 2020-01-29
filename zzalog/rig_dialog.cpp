
#include "rig_dialog.h"

#include "../zzalib/utils.h"
#include "../zzalib/rig_if.h"
#include "status.h"
#include "intl_widgets.h"
#include "drawing.h"

#include "../zzalib/serial.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Spinner.H>

using namespace zzalog;
using namespace zzalib;
using namespace std;

extern Fl_Preferences* settings_;
extern rig_if* rig_if_;
extern status* status_;
void add_rig_if();

// Constructor
rig_dialog::rig_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)
	, selected_rig_if_(RIG_NONE)
	, fast_poll_interval_(nan(""))
	, slow_poll_interval_(nan(""))
	, omnirig_index_(0)
	, ip_port_(0)
	, ip_resource_("")
	, handler_radio_params_(nullptr)
	, omnirig_grp_(nullptr)
	, hamlib_grp_(nullptr)
	, flrig_grp_(nullptr)
	, norig_grp_(nullptr)
	, override_caps_(false)
	, baud_rate_("9600")
	, baud_rate_choice_(nullptr)
	, ip_address_()
	, mfr_choice_(nullptr)
	, model_id_(0)
	, override_check_(nullptr)
	, rig_choice_(nullptr)
	, rig_model_choice_(nullptr)
	, port_if_choice_(nullptr)
	, show_all_ports_(nullptr)
	, all_ports_(false)
	, existing_ports_(nullptr)
{
	actual_rigs_.clear();
	for (int i = 0; i < 4; i++) ip_address_[i] = 0;
	do_creation(X, Y);
}

// Destructor
rig_dialog::~rig_dialog()
{
	delete[] handler_radio_params_;
	delete[] existing_ports_;
}

// Create the dialog display
void rig_dialog::create_form(int X, int Y) {
	// specific position constants
	const int X_RIG_RAD = X + XLEFT;
	const int C1_RIG = X_RIG_RAD + GAP;
	const int W_RIG_RAD = WRADIO + WLABEL;
	const int C2_RIG = X_RIG_RAD + GAP + W_RIG_RAD + GAP;
	const int W_RIG_VAL = WLABEL + GAP + (4 * WBUTTON) + GAP;
	const int W_RIG = C2_RIG + W_RIG_VAL - X_RIG_RAD;
	const int X_POLL = X + XLEFT;
	const int C1_POLL = X_POLL + GAP;
	const int W1_POLL = WRADIO + WLABEL;
	const int C2_POLL = C2_RIG;
	const int W2_POLL = WSMEDIT;
	const int W_POLL = C2_POLL + W2_POLL + GAP - X_POLL; 
	const int WALL = max(W_RIG, W_POLL);

	const int Y_OMNI = Y + YTOP;
	const int Y1_OMNI = Y_OMNI + GAP;
	const int H_OMNI = HTEXT + GAP + GAP;
	const int Y_HAMLIB = Y_OMNI + H_OMNI;
	const int Y1_HAMLIB = Y_HAMLIB + GAP;
	const int H_HAMLIB = (4 * HTEXT) + GAP + GAP;
	const int Y_FLRIG = Y_HAMLIB + H_HAMLIB;
	const int Y1_FLRIG = Y_FLRIG + GAP;
	const int H_FLRIG = (3 * HTEXT) + GAP + GAP;
	const int Y_NORIG = Y_FLRIG + H_FLRIG;
	const int Y1_NORIG = Y_NORIG + GAP;
	const int H_NORIG = HTEXT + GAP + GAP;
	const int Y_POLL = Y_NORIG + H_NORIG;
	const int Y1_POLL = Y_POLL + GAP;
	const int H_POLL = GAP + (3 * HTEXT) + GAP;

	// Group for rig radio buttons: selects the handler type
	Fl_Group* rig_if_grp = new Fl_Group(X_RIG_RAD, Y_OMNI, WRADIO + WLABEL + GAP, H_OMNI + H_HAMLIB + H_FLRIG + H_NORIG);
	handler_radio_params_ = new radio_param_t[4];
	// Radio - select Omnirig
	Fl_Radio_Round_Button* omnirig_radio = new Fl_Radio_Round_Button(C1_RIG, Y1_OMNI, WRADIO, HTEXT);
	handler_radio_params_[0] = { (int)RIG_OMNIRIG, (int*)&selected_rig_if_ };
	omnirig_radio->align(FL_ALIGN_RIGHT);
	omnirig_radio->callback(cb_rad_handler, (void*)&handler_radio_params_[0]);
	omnirig_radio->when(FL_WHEN_RELEASE);
	omnirig_radio->labelsize(FONT_SIZE);
	omnirig_radio->tooltip("Select Omnirig as the rig interface handler");
	omnirig_radio->value(selected_rig_if_ == RIG_OMNIRIG);

	// Radio - Select Hamlib
	Fl_Radio_Round_Button* hamlib_radio = new Fl_Radio_Round_Button(C1_RIG, Y1_HAMLIB, WRADIO, HTEXT);
	handler_radio_params_[1] = { (int)RIG_HAMLIB, (int*)&selected_rig_if_ };
	hamlib_radio->align(FL_ALIGN_RIGHT);
	hamlib_radio->callback(cb_rad_handler, (void*)&handler_radio_params_[1]);
	hamlib_radio->when(FL_WHEN_RELEASE);
	hamlib_radio->labelsize(FONT_SIZE);
	hamlib_radio->tooltip("Select Hamlib as the rig interface handler");
	hamlib_radio->value(selected_rig_if_ == RIG_HAMLIB);
	// Radio - Select Flrig
	Fl_Radio_Round_Button* flrig_radio = new Fl_Radio_Round_Button(C1_RIG, Y1_FLRIG, WRADIO, HTEXT);
	handler_radio_params_[2] = { (int)RIG_FLRIG, (int*)&selected_rig_if_ };
	flrig_radio->align(FL_ALIGN_RIGHT);
	flrig_radio->callback(cb_rad_handler, (void*)&handler_radio_params_[2]);
	flrig_radio->when(FL_WHEN_RELEASE);
	flrig_radio->labelsize(FONT_SIZE);
	flrig_radio->tooltip("Select Flrig as the rig interface handler");
	flrig_radio->value(selected_rig_if_ == RIG_FLRIG);
	// Radio - Select No Rig
	Fl_Radio_Round_Button* norig_radio = new Fl_Radio_Round_Button(C1_RIG, Y1_NORIG, WRADIO, HTEXT);
	handler_radio_params_[3] = { (int)RIG_NONE, (int*)&selected_rig_if_ };
	norig_radio->align(FL_ALIGN_RIGHT);
	norig_radio->callback(cb_rad_handler, (void*)&handler_radio_params_[3]);
	norig_radio->when(FL_WHEN_RELEASE);
	norig_radio->labelsize(FONT_SIZE);
	norig_radio->tooltip("Select no rig interface handler");
	norig_radio->value(selected_rig_if_ == RIG_NONE);

	rig_if_grp->end();

	// Omni rig group
	omnirig_grp_ = new Fl_Group(X_RIG_RAD, Y_OMNI, WALL, H_OMNI, "Omnirig");
	omnirig_grp_->labelsize(FONT_SIZE);
	omnirig_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	omnirig_grp_->box(FL_DOWN_FRAME);
	// choice - between no rig, rig #1 or rig #2
	Fl_Choice* rig_num_choice = new Fl_Choice(C2_RIG, Y1_OMNI, 2 * WBUTTON, HTEXT, "Rig\nNumber");
	rig_num_choice->align(FL_ALIGN_LEFT);
	rig_num_choice->labelsize(FONT_SIZE);
	rig_num_choice->textsize(FONT_SIZE);
	rig_num_choice->tooltip("Select which omnirig radio to use (1 or 2)");
	rig_num_choice->add("None");
	rig_num_choice->add("Rig #1");
	rig_num_choice->add("Rig #2");
	rig_num_choice->callback(cb_value<Fl_Choice, int>, &omnirig_index_);
	rig_num_choice->when(FL_WHEN_RELEASE);
	rig_num_choice->value(omnirig_index_);
	omnirig_grp_->end();

	// Hamlib group
	hamlib_grp_ = new Fl_Group(X_RIG_RAD, Y_HAMLIB, WALL, H_HAMLIB, "Hamlib");
	hamlib_grp_->labelsize(FONT_SIZE);
	hamlib_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	hamlib_grp_->box(FL_DOWN_FRAME);

	// Choice - select rig from users settings
	rig_choice_ = new Fl_Choice(C2_RIG, Y1_HAMLIB, 4 * WBUTTON, HTEXT, "Rig");
	rig_choice_->align(FL_ALIGN_LEFT);
	rig_choice_->labelsize(FONT_SIZE);
	rig_choice_->textsize(FONT_SIZE);
	rig_choice_->tooltip("Select which radio to use");
	rig_choice_->callback(cb_ch_rig);
	rig_choice_->when(FL_WHEN_CHANGED);
	populate_rig_choice();

	// Choice - Select the rig model
	rig_model_choice_ = new Fl_Choice(C2_RIG, Y1_HAMLIB + (HTEXT), 4 * WBUTTON, HTEXT, "Rig model");
	rig_model_choice_->align(FL_ALIGN_LEFT);
	rig_model_choice_->labelsize(FONT_SIZE);
	rig_model_choice_->textsize(FONT_SIZE);
	rig_model_choice_->tooltip("Select the model");
	rig_model_choice_->callback(cb_ch_model , &hamlib_model_);

	// Populate the manufacturere and model choice widgets
	populate_model_choice();

	// Choice port name
	port_if_choice_ = new Fl_Choice(C2_RIG, Y1_HAMLIB + (2 * HTEXT), WBUTTON, HTEXT, "Port");
	port_if_choice_->align(FL_ALIGN_LEFT);
	port_if_choice_->labelsize(FONT_SIZE);
	port_if_choice_->textsize(FONT_SIZE);
	port_if_choice_->callback(cb_text<Fl_Choice, string>, &port_name_);
	port_if_choice_->tooltip("Select the comms port to use");

	// Use all ports
	show_all_ports_ = new Fl_Check_Button(C2_RIG + WBUTTON + GAP, Y1_HAMLIB + (2 * HTEXT), HBUTTON, HBUTTON, "All ports");
	show_all_ports_->align(FL_ALIGN_RIGHT);
	show_all_ports_->labelfont(FONT);
	show_all_ports_->labelsize(FONT_SIZE);
	show_all_ports_->tooltip("Select all existing ports, not just those available");
	show_all_ports_->callback(cb_bn_all, &all_ports_);
	populate_port_choice();

	// Baud rate input 
	baud_rate_choice_ = new Fl_Choice(C2_RIG, Y1_HAMLIB + (3 * HTEXT), WBUTTON, HTEXT, "Baud rate");
	baud_rate_choice_->align(FL_ALIGN_LEFT);
	baud_rate_choice_->labelsize(FONT_SIZE);
	baud_rate_choice_->textsize(FONT_SIZE);
	baud_rate_choice_->tooltip("Enter baud rate");
	baud_rate_choice_->callback(cb_text<Fl_Choice, string>, &baud_rate_);

	// Override caps
	override_check_ = new Fl_Check_Button(C2_RIG + WBUTTON + GAP, baud_rate_choice_->y(), HBUTTON, HBUTTON, "Override rig capabilities");
	override_check_->align(FL_ALIGN_RIGHT);
	override_check_->labelsize(FONT_SIZE);
	override_check_->tooltip("Allow full baud rate selection");
	override_check_->callback(cb_ch_over, &override_caps_);

	populate_baud_choice();

	hamlib_grp_->end();

	// Flrig group
	flrig_grp_ = new Fl_Group(X_RIG_RAD, Y_FLRIG, WALL, H_FLRIG, "Flrig");
	flrig_grp_->labelsize(FONT_SIZE);
	flrig_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	flrig_grp_->box(FL_DOWN_FRAME);
	// 4 separate inputs for the 4 bytes of the IPv4 address of the flrig server
	Fl_Int_Input* ip_addr_in[4];
	for (int i = 0; i < 4; i++) {
		ip_addr_in[i] = new Fl_Int_Input(C2_RIG + (i * WBUTTON / 2), Y1_FLRIG, WBUTTON / 2, HTEXT);
		if (i == 0) {
			ip_addr_in[i]->label("Address");
			ip_addr_in[i]->align(FL_ALIGN_LEFT);
			ip_addr_in[i]->labelsize(FONT_SIZE);
		}
		ip_addr_in[i]->textsize(FONT_SIZE);
		ip_addr_in[i]->tooltip("The IP address of the Flrig server");
		ip_addr_in[i]->value(to_string(ip_address_[i]).c_str());
		ip_addr_in[i]->callback(cb_value_int<Fl_Int_Input>, &ip_address_[i]);
		ip_addr_in[i]->when(FL_WHEN_ENTER_KEY_ALWAYS);
	}
	// IPv4 port number
	Fl_Int_Input* ip_port_in = new Fl_Int_Input(C2_RIG, Y1_FLRIG + HTEXT, WBUTTON, HTEXT, "Port");
	ip_port_in->align(FL_ALIGN_LEFT);
	ip_port_in->labelsize(FONT_SIZE);
	ip_port_in->textsize(FONT_SIZE);
	ip_port_in->tooltip("The IP port number of the Flrig server");
	ip_port_in->value(to_string(ip_port_).c_str());
	ip_port_in->callback(cb_value_int<Fl_Int_Input>, &ip_port_);
	ip_port_in->when(FL_WHEN_ENTER_KEY_ALWAYS);
	// XML-RPC resource name
	intl_input* ip_resource_in = new intl_input(C2_RIG, Y1_FLRIG + (2 * HTEXT), WBUTTON, HTEXT, "Resource");
	ip_resource_in->align(FL_ALIGN_LEFT);
	ip_resource_in->labelsize(FONT_SIZE);
	ip_resource_in->textsize(FONT_SIZE);
	ip_resource_in->tooltip("The IP port number of the Flrig server");
	ip_resource_in->value(ip_resource_.c_str());
	ip_resource_in->callback(cb_value<intl_input, string>, &ip_resource_);
	ip_resource_in->when(FL_WHEN_ENTER_KEY_ALWAYS);
	flrig_grp_->end();

	norig_grp_ = new Fl_Group(X_RIG_RAD, Y_NORIG, WALL, H_FLRIG, "No rig connected");
	norig_grp_->labelsize(FONT_SIZE);
	norig_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	norig_grp_->box(FL_DOWN_FRAME);
	norig_grp_->end();

	// Poll period group;
	Fl_Group* poll_grp = new Fl_Group(X_POLL, Y_POLL, WALL, H_POLL, "Polling interval");
	poll_grp->labelsize(FONT_SIZE);
	poll_grp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	poll_grp->box(FL_DOWN_FRAME);
	// Spinner to select fast polling rate
	Fl_Spinner* poll_fast_spin = new Fl_Spinner(C2_POLL, Y_POLL + HTEXT, W2_POLL, HTEXT, "Interval (s)");
	poll_fast_spin->align(FL_ALIGN_TOP);
	poll_fast_spin->labelsize(FONT_SIZE);
	poll_fast_spin->textsize(FONT_SIZE);
	poll_fast_spin->tooltip("Select the polling period for fast polling");
	poll_fast_spin->type(FL_FLOAT_INPUT);
	poll_fast_spin->minimum(FAST_RIG_MIN);
	poll_fast_spin->maximum(FAST_RIG_MAX);
	poll_fast_spin->step(0.01);
	poll_fast_spin->value(fast_poll_interval_);
	poll_fast_spin->callback(cb_value<Fl_Spinner, double>, &fast_poll_interval_);
	poll_fast_spin->when(FL_WHEN_CHANGED);
	// Spinner to select slow polling rate
	Fl_Spinner* poll_slow_spin = new Fl_Spinner(C2_POLL, Y_POLL + HTEXT + HTEXT, W2_POLL, HTEXT);
	poll_slow_spin->align(FL_ALIGN_LEFT);
	poll_slow_spin->labelsize(FONT_SIZE);
	poll_slow_spin->textsize(FONT_SIZE);
	poll_slow_spin->tooltip("Select the polling period for slow polling");
	poll_slow_spin->type(FL_FLOAT_INPUT);
	poll_slow_spin->minimum(SLOW_RIG_MIN);
	poll_slow_spin->maximum(SLOW_RIG_MAX);
	poll_slow_spin->step(0.5);
	poll_slow_spin->value(slow_poll_interval_);
	poll_slow_spin->callback(cb_value<Fl_Spinner, double>, &slow_poll_interval_);
	poll_slow_spin->when(FL_WHEN_CHANGED);
	poll_grp->end();

	Fl_Group::end();
}

// Enable the widgets - activate the group associated with each rig handler when that
// handler is enabled
void rig_dialog::enable_widgets() {
	if (selected_rig_if_ == RIG_OMNIRIG) {
		omnirig_grp_->activate();
	}
	else {
		omnirig_grp_->deactivate();
	}
	if (selected_rig_if_ == RIG_HAMLIB) {
		hamlib_grp_->activate();
	}
	else {
		hamlib_grp_->deactivate();
	}
	if (selected_rig_if_ == RIG_FLRIG) {
		flrig_grp_->activate();
	}
	else {
		flrig_grp_->deactivate();
	}
	if (selected_rig_if_ == RIG_NONE) {
		norig_grp_->activate();
	}
	else {
		norig_grp_->deactivate();
	}
}

// Populate the rig choice
void rig_dialog::populate_rig_choice() {
	// Initialise the rig_choice
	rig_choice_->clear();
	int i = 0;
	for (auto it = actual_rigs_.begin(); it != actual_rigs_.end(); it++, i++) {
		rig_choice_->add(it->first.c_str());
		if (current_rig_ == it->first) {
			rig_choice_->value(i);
		}
	}
}

// Populate manufacturer and model choices
void rig_dialog::populate_model_choice() {
	// Get hamlib Model number and populate control with all model names
	// See if we need to load back-ends - the handler is not hamlib or the rig isn't open
	if (dynamic_cast<rig_hamlib*>(rig_if_) == nullptr || !rig_if_->is_open()) {
		rig_load_all_backends();
	}
	rig_model_choice_->clear();
	char* target_pathname = nullptr;
	// For each possible rig ids in hamlib (step by 100 as manufacturers are separated into groups)
	for (rig_model_t i = 1; i < 4000; i+=1) {
		// Get the capabilities of this rig ID (at ID #xx01)
		const rig_caps* capabilities = rig_get_caps(i);
		if (capabilities != nullptr) {
			// There is a rig - add the model name to that choice
			char status[16];
			switch (capabilities->status) {
			case RIG_STATUS_ALPHA:
				strcpy(status, " (Alpha)");
				break;
			case RIG_STATUS_UNTESTED:
				strcpy(status, " (Untested)");
				break;
			case RIG_STATUS_BETA:
				strcpy(status, " (Beta)");
				break;
			case RIG_STATUS_STABLE:
				strcpy(status, "");
				break;
			case RIG_STATUS_BUGGY:
				strcpy(status, " (Buggy)");
				break;
			}
			// Generate the item pathname - e.g. Icom/IC-736 (untested)
			char* temp = new char[strlen(status) + 10 + strlen(capabilities->model_name) + strlen(capabilities->mfg_name)];
			sprintf(temp, "%s/%s%s", capabilities->mfg_name, capabilities->model_name, status);
			rig_model_choice_->add(temp);
			if (strcmp(hamlib_model_.c_str(), (capabilities->model_name)) == 0 &&
				strcmp(hamlib_mfr_.c_str(), (capabilities->mfg_name)) == 0) {
				// Remember the pathname of the wanted menu item and the hamlib reference number
				target_pathname = new char[strlen(temp) + 1];
				strcpy(target_pathname, temp);
				model_id_ = i;
			}
		}
	}
	bool found = false;
	// Go through all the menu items until we find our remembered pathname, and set the choice value to that item number
	for (int i = 0; i < rig_model_choice_->size() && !found && target_pathname; i++) {
		char item_pathname[128];
		rig_model_choice_->item_pathname(item_pathname, 127, &rig_model_choice_->menu()[i]);
		if (strcmp(item_pathname, target_pathname) == 0) {
			found = true;
			rig_model_choice_->value(i);
		}
	}
	delete[] target_pathname;
}

// Rig handler radio button clicked
void rig_dialog::cb_rad_handler(Fl_Widget* w, void* v) {
	rig_dialog* that = ancestor_view<rig_dialog>(w);
	// Get the selected radio button into *v
	cb_radio(w, v);
	// Enable the appropriate widget group for the selected handler
	that->enable_widgets();
}

// Rig input choice selected
void rig_dialog::cb_ch_rig(Fl_Widget* w, void* v) {
	rig_dialog* that = ancestor_view<rig_dialog>(w);
	// First save hamlib values back to rig
	that->save_hamlib_values();

	cb_text<Fl_Choice, string>(w, &that->current_rig_);

	// Set the data values
	hamlib_data& data = that->actual_rigs_[that->current_rig_];
	that->hamlib_mfr_ = data.mfr;
	that->hamlib_model_ = data.model;
	that->port_name_ = data.port_name;
	that->baud_rate_ = data.baud_rate;
	that->override_caps_ = data.override_caps;

	// Set the widgets values
	that->populate_model_choice();
	that->port_if_choice_->value(that->port_if_choice_->find_item(data.port_name.c_str()));
	that->populate_baud_choice();
}

// Mode input choice selected
void rig_dialog::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	rig_dialog* that = ancestor_view<rig_dialog>(w);
	// Get the full item name - e.g. Icom/IC-736 (untested)
	char path[128];
	ch->item_pathname(path, sizeof(path) - 1);
	// Get the manufacturer - i.e. upto the / character
	char* pos_stroke = strchr(path, '/');
	that->hamlib_mfr_ = string(path, pos_stroke - path);
	// Get the mode - upto the " (" - returns nullptr if it's not there
	char* pos_bracket = strstr(pos_stroke + 1, " (");
	if (pos_bracket == nullptr) {
		that->hamlib_model_ = string(pos_stroke + 1);
	}
	else {
		that->hamlib_model_ = string(pos_stroke + 1, pos_bracket - pos_stroke - 1);
	}
	// For each possible rig ids in hamlib 
	bool found = false;
	for (rig_model_t i = 1; i < 4000 && !found; i += 1) {
		// Get the capabilities of this rig ID (at ID #xx01)
		const rig_caps* capabilities = rig_get_caps(i);
		if (capabilities != nullptr) {
			if (strcmp(that->hamlib_model_.c_str(), (capabilities->model_name)) == 0 &&
				strcmp(that->hamlib_mfr_.c_str(), (capabilities->mfg_name)) == 0) {
				that->model_id_ = i;
				found = true;
			}
		}
	}
	that->populate_baud_choice();
}

// Override rig capabilities selected - repopulate the baud choice
void rig_dialog::cb_ch_over(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	rig_dialog* that = ancestor_view<rig_dialog>(w);
	that->populate_baud_choice();
}

// Select all ports in port choice
void rig_dialog::cb_bn_all(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	rig_dialog* that = ancestor_view<rig_dialog>(w);
	that->populate_port_choice();
}

// Get the settings to preload the widget values
void rig_dialog::load_values() {
	// Get the handler-independent settings
	Fl_Preferences rig_settings(settings_, "Rig");
	rig_settings.get("Handler", (int&)selected_rig_if_, RIG_OMNIRIG);
	rig_settings.get("Polling Interval", fast_poll_interval_, FAST_RIG_DEF);
	rig_settings.get("Slow Polling Interval", slow_poll_interval_, SLOW_RIG_DEF);
	// Omnirig settings
	Fl_Preferences omnirig_settings(rig_settings, "Omnirig");
	omnirig_settings.get("Rig Number", omnirig_index_, 1);
	// Hamlib settings
	Fl_Preferences hamlib_settings(rig_settings, "Hamlib");
	// Get the rig names from the stations settings
	char * temp;
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");
	rigs_settings.get("Current", temp, "");
	current_rig_ = temp;
	free(temp);
	// Read all the groups
	int num_rigs = rigs_settings.groups();
	for (int i = 0; i < num_rigs; i++) {
		string rig_name = rigs_settings.group(i);
		// Check if the rig is active
		int active = (int)false;
		Fl_Preferences rig_settings(rigs_settings, rig_name.c_str());
		rig_settings.get("Active", active, (int)false);
		if (active) {
			// If it is active, add it to the list of rigs
			Fl_Preferences act_settings(hamlib_settings, rig_name.c_str());
			act_settings.get("Manufacturer", temp, "Hamlib");
			hamlib_data& data = actual_rigs_[rig_name];
			data.mfr = temp;
			free(temp);
			act_settings.get("Rig Model", temp, "Dummy");
			data.model = temp;
			free(temp);
			act_settings.get("Port", temp, "COM6");
			data.port_name = temp;
			free(temp);
			act_settings.get("Baud Rate", temp, "9600");
			data.baud_rate = temp;
			free(temp);
			act_settings.get("Override Rig Caps", (int&)data.override_caps, false);
			if (rig_name == current_rig_) {
				hamlib_mfr_ = data.mfr;
				hamlib_model_ = data.model;
				port_name_ = data.port_name;
				baud_rate_ = data.baud_rate;
				override_caps_ = data.override_caps;
			}
		}
	}
	// Flrig settings
	Fl_Preferences flrig_settings(rig_settings, "FLRig");
	unsigned long ip_address = 0;
	flrig_settings.get("IP Address", (int&)ip_address, 0x7F000001);
	for (int i = 3; i >= 0; i--) {
		ip_address_[i] = ip_address & 0x000000FF;
		ip_address >>= 8;
	}
	flrig_settings.get("Port Number", ip_port_, 12345);
	flrig_settings.get("Resource", temp, "/RPC2");
	ip_resource_ = temp;
	free(temp);

}

// Write back values to settings
void rig_dialog::save_values() {

	// Handler-indepenedent settings
	Fl_Preferences rig_settings(settings_, "Rig");
	rig_settings.set("Handler", selected_rig_if_);
	rig_settings.set("Polling Interval", fast_poll_interval_);
	rig_settings.set("Slow Polling Interval", slow_poll_interval_);
	if (selected_rig_if_ == RIG_OMNIRIG) {
		// Omnirig settings
		Fl_Preferences omnirig_settings(rig_settings, "Omnirig");
		omnirig_settings.set("Rig Number", omnirig_index_);
	}
	else if (selected_rig_if_ == RIG_HAMLIB) {
		save_hamlib_values();
		// Change current rig
		Fl_Preferences stations_settings(settings_, "Stations");
		Fl_Preferences rigs_settings(stations_settings, "Rigs");
		rigs_settings.set("Current", current_rig_.c_str());
		// Hamlib settings
		Fl_Preferences hamlib_settings(rig_settings, "Hamlib");
		for (auto it = actual_rigs_.begin(); it != actual_rigs_.end(); it++) {
			Fl_Preferences act_settings(hamlib_settings, it->first.c_str());
			hamlib_data& data = it->second;
			act_settings.set("Manufacturer", data.mfr.c_str());
			act_settings.set("Rig Model", data.model.c_str());
			act_settings.set("Port", data.port_name.c_str());
			act_settings.set("Baud Rate", data.baud_rate.c_str());
			act_settings.set("Override Rig Caps", data.override_caps);
		}
	}
	else if (selected_rig_if_ == RIG_FLRIG) {
		// Flrig settings
		Fl_Preferences flrig_settings(rig_settings, "FLRig");
		unsigned long ip_address = 0;
		for (int i = 3; i >= 0; i--) {
			ip_address <<= 8;
			ip_address |= ip_address_[i] & 0xFF;
		}
		flrig_settings.set("IP Address", (int)ip_address);
		flrig_settings.set("Port Number", ip_port_);
		flrig_settings.set("Resource", ip_resource_.c_str());

	}

	// restart rig interface
	add_rig_if();
}


// POpulate the chice with the available ports
void rig_dialog::populate_port_choice() {
	port_if_choice_->clear();
	port_if_choice_->add("NONE");
	port_if_choice_->value(0);
	delete[] existing_ports_;
	int num_ports = 1;
	existing_ports_ = new string[1];
	serial serial;
	while (!serial.available_ports(num_ports, existing_ports_, all_ports_, num_ports)) {
		delete[] existing_ports_;
		existing_ports_ = new string[num_ports];
	}
	// now for the returned ports
	for (int i = 0; i < num_ports; i++) {
		// Add the name onto the choice drop-down list
		char message[100];
		const char* port = existing_ports_[i].c_str();
		snprintf(message, sizeof(message), "RIG: Found port %s", port);
		status_->misc_status(ST_LOG, message);
		port_if_choice_->add(port);
		// Set the value to the list of ports
		if (strcmp(port, port_name_.c_str()) == 0) {
			port_if_choice_->value(i);
		}
	}
}

// Populate the baud rate choice menu
void rig_dialog::populate_baud_choice() {
	baud_rate_choice_->clear();
	// Override rig's capabilities?
	override_check_->value(override_caps_);

	// Get the baud-rates supported by the rig
	const rig_caps* caps = rig_get_caps(model_id_);
	int min_baud_rate = 300;
	int max_baud_rate = 460800;
	if (caps) {
		min_baud_rate = caps->serial_rate_min;
		max_baud_rate = caps->serial_rate_max;
	}
	// Default baud-rates
	const int baud_rates[] = { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800 };
	int num_rates = sizeof(baud_rates) / sizeof(int);
	int index = 0;
	baud_rate_choice_->value(0);
	// For all possible rates
	for (int i = 0; i < num_rates; i++) {
		int rate = baud_rates[i];
		if (override_caps_ || (rate >= min_baud_rate && rate <= max_baud_rate)) {
			// capabilities overridden or within the range supported by capabilities
			baud_rate_choice_->add(to_string(rate).c_str());
			if (to_string(rate) == baud_rate_) {
				baud_rate_choice_->value(index);
				index++;
			}
		}
	}
}

// Save the widget values back to the actual rig values
void rig_dialog::save_hamlib_values() {
	hamlib_data& data = actual_rigs_[current_rig_];
	data.mfr = hamlib_mfr_;
	data.model = hamlib_model_;
	data.port_name = port_name_;
	data.baud_rate = baud_rate_;
	data.override_caps = override_caps_;
}