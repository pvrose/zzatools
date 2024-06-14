#include "qso_rig.h"
#include "rig_if.h"
#include "serial.h"
#include "status.h"
#include "qso_manager.h"
#include "band_data.h"
#include "spec_data.h"

#include <set>
#include <string>

#include <FL/Fl_Preferences.H>

using namespace std;

extern Fl_Preferences* settings_;
extern status* status_;
extern band_data* band_data_;
extern spec_data* spec_data_;
extern bool DARK;


// Constructor
qso_rig::qso_rig(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, nullptr),
	modify_freq_(false),
	freq_offset_(0.0),
	modify_gain_(false),
	gain_(0),
	modify_power_(false),
	power_(0.0)
{
	// If no name is provided then get from qso_manager
	if (L == nullptr || strlen(L) == 0) copy_label(ancestor_view<qso_manager>(this)->get_default(qso_manager::RIG).c_str());
	// Otherwise copy that supplied as it is probably a transient string
	else copy_label(L);
	// CAT control group
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	load_values();
	rig_ = new rig_if(label(), &hamlib_data_);
	create_form(X, Y);
	enable_widgets();
}

// DEstructor
qso_rig::~qso_rig() {
	save_values();
	delete rig_;
}

// Get initial data from settings
void qso_rig::load_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	Fl_Preferences cat_settings(settings_, "CAT");
	if (cat_settings.groupExists(label())) {
		Fl_Preferences rig_settings(cat_settings, label());
		Fl_Preferences hamlib_settings(rig_settings, "Hamlib");
		char* temp;
		hamlib_settings.get("Rig Model", temp, "dummy");
		hamlib_data_.model = temp;
		free(temp);
		hamlib_settings.get("Manufacturer", temp, "Hamlib");
		hamlib_data_.mfr = temp;
		free(temp);
		hamlib_settings.get("Port", temp, "COM6");
		hamlib_data_.port_name = temp;
		free(temp);
		hamlib_settings.get("Baud Rate", hamlib_data_.baud_rate, 9600);
		hamlib_settings.get("Model ID", (int&)hamlib_data_.model_id, -1);

		// Check that hamlib is currently OK
		const rig_caps* capabilities = rig_get_caps(hamlib_data_.model_id);
		if (capabilities->model_name != hamlib_data_.model ||
			capabilities->mfg_name != hamlib_data_.mfr) {
			char msg[128];
			snprintf(msg, 128, "RIG: Saved model id %d does not match supplied rig model %s/%s",
				hamlib_data_.model_id,
				hamlib_data_.mfr.c_str(),
				hamlib_data_.model.c_str());
			status_->misc_status(ST_WARNING, msg);
			find_hamlib_data();
		}
		else {
			hamlib_data_.port_type = capabilities->port_type;
		};
		
		// If hamlib and FLRig - start parameters
		Fl_Preferences app_settings(rig_settings, "Apps");
		app_settings.get("FLRig", temp, "");
		app_flrig_ = temp;
		free(temp);
		for (int i = 0; i < NUMBER_APPS; i++) {
			app_settings.get(MODEM_NAMES[i].c_str(), temp, "");
			apps_[(modem_t)i] = temp;
			free(temp);
		}
		// Preferred antenna
		rig_settings.get("Antenna", temp, "");
		antenna_ = temp;
		free(temp);

		Fl_Preferences modifier_settings(rig_settings, "Modifiers");
		if (modifier_settings.get("Frequency", freq_offset_, 0.0)) {
			modify_freq_ = true;
		} else {
			modify_freq_ = false;
		}
		if (modifier_settings.get("Gain", gain_, 0)) {
			modify_gain_ = true;
		} else {
			modify_gain_ = false;
		}
		if (modifier_settings.get("Power", power_, 0.0)) {
			modify_power_ = true;
		} else {
			modify_power_ = false;
		}
		// If we cannot read power from hamlib - force modify_power_
		if (!(capabilities->has_get_level & RIG_LEVEL_RFPOWER_METER_WATTS)) {
			char msg[128];
			snprintf(msg, sizeof(msg), "DASH: Rig %s does not supply power - set it", 
				hamlib_data_.model.c_str());
			status_->misc_status(ST_WARNING, msg);
			modify_power_ = true;
			modify_gain_ = false;
		}
	}
	else {
		// New hamlib data
		hamlib_data_ = rig_if::hamlib_data_t();
		modify_freq_ = false;
		modify_power_ = false;
		modify_gain_ = false;
	}
	mode_ = hamlib_data_.port_type;
}

void qso_rig::find_hamlib_data() {
	// Now search hamlib for the model_id
	// Search through the rig database until we find the required rig.
	const rig_caps* capabilities = nullptr;
	rig_model_t max_rig_num = 40 * MAX_MODELS_PER_BACKEND;
	for (rig_model_t i = 0; i < max_rig_num && hamlib_data_.model_id == -1; i++) {
		// Read each rig's capabilities
		capabilities = rig_get_caps(i);
		try {
			if (capabilities != nullptr) {
				// Not all numbers represent a rig as the mapping is sparse
				// Check the model name
				if (capabilities->model_name == hamlib_data_.model &&
					capabilities->mfg_name == hamlib_data_.mfr) hamlib_data_.model_id = i;
			}
		}
		catch (exception*) {}
	}
	if (hamlib_data_.model_id == -1) {
		char msg[128];
		snprintf(msg, 128, "RIG: %s/%s not found in hamlib capabilities",
			hamlib_data_.mfr.c_str(),
			hamlib_data_.model.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

const int WDISPLAY = 3 * WBUTTON;
// Create 
void qso_rig::create_status(int& curr_x, int& curr_y) {
	// "Label" - rig status
	op_status_ = new Fl_Output(curr_x, curr_y, WDISPLAY, HBUTTON);
	op_status_->color(FL_BACKGROUND_COLOR);
	op_status_->textfont(FL_BOLD);
	op_status_->textsize(FL_NORMAL_SIZE + 2);
	op_status_->box(FL_FLAT_BOX);
	
	curr_y += op_status_->h();

	op_freq_mode_ = new Fl_Box(curr_x, curr_y, WDISPLAY, HTEXT * 3);
	op_freq_mode_->tooltip("Current displayed mode");
	op_freq_mode_->box(FL_FLAT_BOX);
	op_freq_mode_->color(FL_BLACK);
	op_freq_mode_->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	op_freq_mode_->labelcolor(FL_YELLOW);
	op_freq_mode_->labelfont(FL_BOLD);
	op_freq_mode_->labelsize(FL_NORMAL_SIZE + 10);

	curr_y += op_freq_mode_->h();
	curr_x += op_freq_mode_->w();
}
void qso_rig::create_buttons(int& curr_x, int& curr_y) {
	// First button - connect/disconnect or add
	bn_connect_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
	bn_connect_->tooltip("Select to attempt to connect/disconnect rig");
	bn_connect_->callback(cb_bn_connect, nullptr);
	
	curr_x += bn_connect_->w();
	bn_select_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Select");
	bn_select_->color(FL_YELLOW);
	bn_select_->tooltip("Select the rig to connect");
	bn_select_->value(false);
	bn_select_->callback(cb_bn_select, nullptr);

	curr_x += bn_select_->w();
	bn_start_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Start Flrig");
	bn_start_->color(FL_DARK_GREEN);
	bn_start_->labelcolor(FL_WHITE);
	bn_start_->tooltip("Start flrig for this connection");
	bn_start_->callback(cb_bn_start, &app_flrig_);

	curr_x += bn_start_->w();
	curr_y += bn_start_->h();
}

void qso_rig::create_rig_ant(int& curr_x, int& curr_y) {
	curr_x += WBUTTON;
	// Choice - Select the rig model (Manufacturer/Model)
	ch_rig_model_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HTEXT, "Rig");
	ch_rig_model_->align(FL_ALIGN_LEFT);
	ch_rig_model_->tooltip("Select the model - for Hamlib");
	ch_rig_model_->callback(cb_ch_model, nullptr);

	// Populate the manufacturere and model choice widgets
	populate_model_choice();

	curr_y += HBUTTON;

	ip_antenna_ = new field_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Antenna");
	ip_antenna_->align(FL_ALIGN_LEFT);
	ip_antenna_->callback(cb_value<field_input, string>, &antenna_);
	ip_antenna_->tooltip("Select the preferred antenna for this rig");
	ip_antenna_->field_name("MY_ANTENNA");
	ip_antenna_->value(antenna_.c_str());

	curr_x += WSMEDIT;
	curr_y += HBUTTON;

}
void qso_rig::create_config(int& curr_x, int& curr_y) {
	config_tabs_ = new Fl_Tabs(curr_x, curr_y, 10, 10);
	config_tabs_->callback(cb_config);
	config_tabs_->when(FL_WHEN_CHANGED);
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	config_tabs_->client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;
	curr_x = rx;
	curr_y = ry;
	create_connex(curr_x, curr_y);
	rw = max(rw, curr_x - rx);
	rh = max(rh, curr_y - ry);
	curr_x = rx;
	curr_y = ry;
	create_apps(curr_x, curr_y);
	rw = max(rw, curr_x - rx);
	rh = max(rh, curr_y - ry);
	curr_x = rx;
	curr_y = ry;
	create_modifier(curr_x, curr_y);
	rw = max(rw, curr_x - rx);
	rh = max(rh, curr_y - ry);
	config_tabs_->resizable(nullptr);
	config_tabs_->size(config_tabs_->w() + rw - saved_rw, config_tabs_->h() + rh - saved_rh);
	end();
	curr_x = config_tabs_->x() + config_tabs_->w();
	curr_y = config_tabs_->y() + config_tabs_->h();

}

void qso_rig::create_connex(int& curr_x, int& curr_y) {
	connect_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Connection");
	curr_y += GAP;
	int max_x = curr_x;
	int max_y = curr_y;
	create_serial(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);
	curr_x = connect_tab_->x();
	curr_y = connect_tab_->y() + GAP;
	create_network(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	connect_tab_->resizable(nullptr);
	connect_tab_->size(max_x - connect_tab_->x(), max_y - connect_tab_->y());

	connect_tab_->end();

}

void qso_rig::create_serial(int& curr_x, int& curr_y) {
	serial_grp_ = new Fl_Group(curr_x, curr_y, 10, 10);
	serial_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	serial_grp_->box(FL_NO_BOX);

	// Choice port name - serial
	curr_x = serial_grp_->x() + WBUTTON;
	curr_y = serial_grp_->y();
	ch_port_name_ = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Port");
	ch_port_name_->align(FL_ALIGN_LEFT);
	ch_port_name_->callback(cb_ch_port, nullptr);
	ch_port_name_->tooltip("Select the comms port to use");

	// Calculate  width necessary for "All"
	int tw = 0;
	int th = 0;
	fl_measure("All", tw, th);
	// Use all ports
	int save_x = curr_x;
	curr_x += WBUTTON + GAP;
	bn_all_ports_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All");
	bn_all_ports_->align(FL_ALIGN_RIGHT);
	bn_all_ports_->tooltip("Select all existing ports, not just those available");
	bn_all_ports_->callback(cb_bn_all, &use_all_ports_);
	populate_port_choice();
	curr_x += HBUTTON + tw + GAP;
	int max_x = curr_x;

	// Baud rate input 
	curr_x = save_x;
	curr_y += HBUTTON;
	int max_y = curr_y;
	ch_baud_rate_ = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Baud rate");
	ch_baud_rate_->align(FL_ALIGN_LEFT);
	ch_baud_rate_->tooltip("Enter baud rate");
	ch_baud_rate_->callback(cb_ch_baud, nullptr);

	// Override capabilities (as coded in hamlib)
	curr_x += WBUTTON + GAP;
	bn_all_rates_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All");
	bn_all_rates_->align(FL_ALIGN_RIGHT);
	bn_all_rates_->tooltip("Allow full baud rate selection");
	bn_all_rates_->callback(cb_ch_over, &use_all_rates_);
	populate_baud_choice();
	curr_x += HBUTTON + tw;
	curr_x = max(max_x, curr_x);

	curr_y += HBUTTON + GAP;
	curr_y = max(max_y, curr_y);
	serial_grp_->resizable(nullptr);
	serial_grp_->size(curr_x - serial_grp_->x(), curr_y - serial_grp_->y());

	serial_grp_->end();

}

void qso_rig::create_network(int& curr_x, int& curr_y) {
	network_grp_ = new Fl_Group(curr_x, curr_y, 10, 10);
	network_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	network_grp_->box(FL_NO_BOX);

	// Input port name - network
	curr_x = network_grp_->x() + WBUTTON;
	curr_y = network_grp_->y();
	ip_port_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HTEXT, "Port");
	ip_port_->align(FL_ALIGN_LEFT);
	ip_port_->callback(cb_ip_port, nullptr);
	ip_port_->tooltip("Enter the network/USB port to use");
	ip_port_->value(hamlib_data_.port_name.c_str());

	curr_y += HTEXT + GAP;

	curr_x += WSMEDIT;
	network_grp_->resizable(nullptr);
	network_grp_->size(curr_x - network_grp_->x(), curr_y - network_grp_->y());

	network_grp_->end();

}

void qso_rig::create_apps(int& curr_x, int& curr_y) {
	app_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Apps");
	curr_x += WBUTTON;
	curr_y += GAP;

	ip_rig_app_= new Fl_Input(curr_x, curr_y, WSMEDIT, HTEXT, "FlRig");
	ip_rig_app_->align(FL_ALIGN_LEFT);
	ip_rig_app_->callback(cb_value<Fl_Input, string>, &app_flrig_);
	ip_rig_app_->tooltip("Enter the command for flrig to connect to the rig");
	ip_rig_app_->value(app_flrig_.c_str());
	
	curr_y += HTEXT;
	for(int i = 0; i < NUMBER_APPS; i++) {
		ip_app_[i]= new Fl_Input(curr_x, curr_y, WSMEDIT, HTEXT);
		ip_app_[i]->copy_label(MODEM_NAMES[i].c_str());
		ip_app_[i]->align(FL_ALIGN_LEFT);
		ip_app_[i]->callback(cb_value<Fl_Input, string>, &apps_[(modem_t)i]);
		ip_app_[i]->tooltip("Enter the command for modem app to connect to the rig");
		ip_app_[i]->value(apps_[i].c_str());
		
		curr_y += HTEXT;
	}
	
	curr_x += WSMEDIT;

	app_tab_->resizable(nullptr);
	app_tab_->size(curr_x - app_tab_->x(), curr_y - app_tab_->y());
	app_tab_->end();


}
void qso_rig::create_modifier(int& curr_x, int& curr_y) {
	modifier_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Transverter/Amp");
	modifier_tab_->box(FL_FLAT_BOX);

	curr_x = modifier_tab_->x() + WBUTTON;
	curr_y += (HTEXT + GAP)/2;

	bn_mod_freq_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "\316\224F (MHz)");
	bn_mod_freq_->align(FL_ALIGN_LEFT);
	bn_mod_freq_->callback(cb_bn_mod_freq, (void*)&modify_freq_);
	bn_mod_freq_->tooltip("Allow frequency to be offset - eg transverter");
	bn_mod_freq_->value(modify_freq_);
	
	curr_x += WRADIO;
	ip_freq_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_freq_->callback(cb_ip_freq, (void*)&freq_offset_);
	ip_freq_->tooltip("Enter frequency offset");
	ip_freq_->when(FL_WHEN_CHANGED);
	char text[20];
	snprintf(text, sizeof(text), "%g", freq_offset_);
	ip_freq_->value(text);

	curr_y += HBUTTON;
	curr_x = modifier_tab_->x() + WBUTTON;

	bn_gain_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Gain (dB)");
	bn_gain_->align(FL_ALIGN_LEFT);
	bn_gain_->callback(cb_bn_power, (void*)(intptr_t)false);
	bn_gain_->tooltip("Allow amplifier");
	bn_gain_->value(modify_gain_);

	curr_x += WRADIO;
	ip_gain_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_gain_->callback(cb_ip_gain, (void*)&gain_);
	ip_gain_->when(FL_WHEN_CHANGED);
	ip_gain_->tooltip("Specify the amplifier gain (dB)");
	snprintf(text, sizeof(text), "%d", gain_);
	ip_gain_->value(text);

	curr_y += HBUTTON;
	curr_x = modifier_tab_->x() + WBUTTON;

	bn_power_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Pwr (W)");
	bn_power_->align(FL_ALIGN_LEFT);
	bn_power_->callback(cb_bn_power, (void*)(intptr_t)true);
	bn_power_->tooltip("Specify absolute power");
	bn_power_->value(modify_power_);

	curr_x += WRADIO;
	ip_power_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_power_->callback(cb_ip_power, (void*)&power_);
	ip_power_->when(FL_WHEN_CHANGED);
	ip_power_->tooltip("Specify power out ");
	snprintf(text, sizeof(text), "%g", power_);
	ip_power_->value(text);

	curr_x += WBUTTON;
	curr_y += HBUTTON + GAP;
	modifier_tab_->resizable(nullptr);
	modifier_tab_->size(curr_x - modifier_tab_->x(), curr_y - modifier_tab_->y());
}

// Create CAT control widgets
void qso_rig::create_form(int X, int Y) {

	begin();

	int max_x = 0;
	int max_y = 0;

	int curr_x = X + GAP;
	int curr_y = Y + 1;


	create_status(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);
	
	curr_x = X + GAP;
	create_buttons(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = X + GAP;
	curr_y += GAP;

	create_rig_ant(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = X + GAP;
	curr_y += GAP;

	create_config(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);




	// Connected status

	// Display hamlib ot flrig settings as selected
	modify_rig();
	enable_widgets();

	max_x += GAP;
	max_y += GAP;

	resizable(nullptr);
	size(max_x - x(), max_y - y());
	end();
}

// Save values in settings
void qso_rig::save_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	if (hamlib_data_.port_type != RIG_PORT_NONE) {
		Fl_Preferences cat_settings(settings_, "CAT");
		Fl_Preferences rig_settings(cat_settings, label());
		Fl_Preferences hamlib_settings(rig_settings, "Hamlib");
		hamlib_settings.set("Rig Model", hamlib_data_.model.c_str());
		hamlib_settings.set("Manufacturer", hamlib_data_.mfr.c_str());
		hamlib_settings.set("Port", hamlib_data_.port_name.c_str());
		hamlib_settings.set("Baud Rate", hamlib_data_.baud_rate);
		hamlib_settings.set("Model ID", (int)hamlib_data_.model_id);
		// Apps
		Fl_Preferences app_settings(rig_settings, "Apps");
		app_settings.set("FLRig", app_flrig_.c_str());
		for (int i = 0; i < NUMBER_APPS; i++) {
			app_settings.set(MODEM_NAMES[i].c_str(), apps_[i].c_str());
			char app_num[10];
		}
		// Preferred antenna
		rig_settings.set("Antenna", antenna_.c_str());
		// Modifier settings
		Fl_Preferences modr_settings(rig_settings, "Modifiers");
		modr_settings.clear();
		if (modify_freq_) {
			modr_settings.set("Frequency", freq_offset_);
		}
		if (modify_gain_) {
			modr_settings.set("Gain", gain_);
		}
		if (modify_power_) {
			modr_settings.set("Power", power_);
		}
		settings_->flush();
	}
}

// Enable CAT Connection widgets
void qso_rig::enable_widgets() {
	// CAT access buttons
	if (!rig_) {
		bn_connect_->deactivate();
		bn_connect_->color(FL_BACKGROUND_COLOR);
		bn_connect_->label("");
		bn_select_->activate();
		if (bn_select_->value()) {
			bn_select_->label("Use");
		} else {
			bn_select_->label("Select");
		}
		bn_start_->activate();
	} else if (rig_->is_open()) {
		bn_connect_->activate();
		bn_connect_->color(fl_lighter(FL_RED));
		bn_connect_->label("Disconnect");
		bn_select_->deactivate();
		bn_select_->label("");
		bn_select_->value(false);
		bn_start_->deactivate();
	} else {
		bn_connect_->activate();
		bn_connect_->color(fl_lighter(FL_GREEN));
		bn_connect_->label("Connect");
		bn_select_->activate();
		if (bn_select_->value()) {
			bn_select_->label("Use");
		} else {
			bn_select_->label("Select");
		}
		bn_start_->activate();
	}
	bn_connect_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_connect_->color()));
	bn_select_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_select_->color()));
	double freq;
	// Status
	if (!rig_) {
		op_status_->value("No rig specified");
		op_status_->textcolor(FL_MAGENTA);
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	} else if (rig_->is_opening()) {
		op_status_->value("Opening rig");
		op_status_->textcolor(DARK ? FL_CYAN : fl_darker(FL_CYAN));
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	} else if (rig_->is_open()) {
		string rig_text;
		if (rig_->get_ptt()) {
			rig_text = "Transmitting: ";
			op_status_->textcolor(FL_RED);
		} else {
			rig_text = "Receiving: ";
			op_status_->textcolor(DARK ? FL_GREEN : fl_darker(FL_GREEN));
		}
		freq = rig_->get_dfrequency(true);
		uint64_t freq_Hz = (uint64_t)(freq * 1000000) % 1000;
		band_data::band_entry_t* entry = band_data_->get_entry(freq * 1000);
		if (entry) {
			char l[50];
			snprintf(l, sizeof(l), "%s", spec_data_->band_for_freq(freq).c_str());
			rig_text += string(l);
		}
		else {
			rig_text += "Out of band!";
			op_status_->textcolor(DARK ? COLOUR_ORANGE : fl_darker(COLOUR_ORANGE));
		}
		op_status_->value(rig_text.c_str());
		op_freq_mode_->activate();
		op_freq_mode_->color(FL_BLACK);
		if (rig_->get_ptt()) {
			op_freq_mode_->labelcolor(FL_RED);
		} else {
			op_freq_mode_->labelcolor(FL_YELLOW);
		}
		char msg[100];
		string rig_mode;
		string submode;
		rig_->get_string_mode(rig_mode, submode);
		snprintf(msg, sizeof(msg), "%0.3f.%03d MHz\n%s %sW" , 
			freq, freq_Hz,
			submode.length() ? submode.c_str() : rig_mode.c_str(),
			rig_->get_tx_power(true).c_str()
		);
		op_freq_mode_->copy_label(msg);
	} else if (rig_->has_no_cat()) {
		op_status_->value("No CAT Available");
		op_status_->textcolor(FL_MAGENTA);
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	} else {
		op_status_->value("Disconnected");
		op_status_->textcolor(DARK ? COLOUR_ORANGE : fl_darker(COLOUR_ORANGE));
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	}
	// CAT control widgets - allow only when select button active
	if (rig_->is_open()) {
		ch_rig_model_->deactivate();
		serial_grp_->deactivate();
		network_grp_->deactivate();
	} else {
		ch_rig_model_->activate();
		serial_grp_->activate();
		network_grp_->activate();
		if (bn_select_->value()) {
			serial_grp_->clear_output();
			network_grp_->clear_output();
		} else {
			serial_grp_->set_output();
			network_grp_->set_output();
		}
	}
	switch (mode_) {
	case RIG_PORT_SERIAL:
		serial_grp_->show();
		network_grp_->hide();
		break;
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		serial_grp_->hide();
		network_grp_->show();
		if (!bn_select_->value()) {
			ip_port_->value(hamlib_data_.port_name.c_str());
		}
		if (hamlib_data_.model == "FLRig") {
			bn_start_->show();
		} else {
			bn_start_->hide();
		}
		break;
	case RIG_PORT_NONE:
		serial_grp_->hide();
		network_grp_->hide();
		break;
	}
	// Update modifier values
	if (modify_freq_) {
		ip_freq_->activate();
	} else {
		ip_freq_->deactivate();
	}
	// We have modified the values so reload them
	bn_gain_->value(modify_gain_);
	bn_power_->value(modify_power_);
	if (modify_gain_) {
		ip_gain_->activate();
	} else {
		ip_gain_->deactivate();
	}
	if (modify_power_) {
		ip_power_->activate();
	} else {
		ip_power_->deactivate();
	}
	// Now use standard TAB highlighting
	for (int ix = 0; ix < config_tabs_->children(); ix++) {
		Fl_Widget* wx = config_tabs_->child(ix);
		if (wx == config_tabs_->value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
	}

	redraw();
}

// Populate manufacturer and model choices - hierarchical menu: first manufacturer, then model
void qso_rig::populate_model_choice() {
	// Get hamlib Model number and populate control with all model names
	ch_rig_model_->clear();
	set<string> rig_list;
	map<string, rig_model_t> rig_ids;
	rig_list.clear();
	rig_ids.clear();
	// For each possible rig ids in hamlib
	rig_model_t max_rig_num = 40 * MAX_MODELS_PER_BACKEND;
	for (rig_model_t i = 1; i < max_rig_num; i += 1) {
		// Get the capabilities of this rig ID (at ID #xx01)
		const rig_caps* capabilities = rig_get_caps(i);
		if (capabilities != nullptr) {
			// There is a rig - add the model name to that choice
			// What is the status of the handler for this particular rig?
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
			// Generate the item pathname - e.g. "Icom/IC-736 (untested)"
			char* temp = new char[256];
			// The '/' ensures all rigs from same manufacturer are in a sub-menu to Icom
			string mfg = escape_menu(capabilities->mfg_name);
			string model = escape_menu(capabilities->model_name);
			snprintf(temp, 256, "%s/%s%s", mfg.c_str(), model.c_str(), status);
			rig_list.insert(temp);
			rig_ids[temp] = capabilities->rig_model;
			if (string(capabilities->mfg_name) == hamlib_data_.mfr &&
				string(capabilities->model_name) == hamlib_data_.model) {
				hamlib_data_.model_id = capabilities->rig_model;
				hamlib_data_.port_type = capabilities->port_type;
			}
		}
	}
	// Add the rigs in alphabetical order to the choice widget, set widget's value to intended
	ch_rig_model_->add("");
	for (auto ix = rig_list.begin(); ix != rig_list.end(); ix++) {
		string name = *ix;
		rig_model_t id = rig_ids.at(name);
		int pos = ch_rig_model_->add(name.c_str(), 0, nullptr, (void*)(intptr_t)id);
		if (id == hamlib_data_.model_id) {
			ch_rig_model_->value(pos);
		}
	}
	bool found = false;
}

// Populate the choice with the available ports
void qso_rig::populate_port_choice() {
	if (hamlib_data_.port_type == RIG_PORT_SERIAL) {
		ch_port_name_->clear();
		ch_port_name_->add("NONE");
		ch_port_name_->value(0);
		int num_ports = 1;
		string* existing_ports = new string[1];
		serial serial;
		// Get the list of all ports or available (not in use) ports
		while (!serial.available_ports(num_ports, existing_ports, use_all_ports_, num_ports)) {
			delete[] existing_ports;
			existing_ports = new string[num_ports];
		}
		// now for the returned ports
		for (int i = 0; i < num_ports; i++) {
			// Add the name onto the choice drop-down list
			char message[100];
			const char* port = existing_ports[i].c_str();
			snprintf(message, sizeof(message), "DASH: Found port %s", port);
			status_->misc_status(ST_LOG, message);
			ch_port_name_->add(port);
			// Set the value to the list of ports
			if (strcmp(port, hamlib_data_.port_name.c_str()) == 0) {
				ch_port_name_->value(i);
			}
		}
	}
}

// Populate the baud rate choice menu
void qso_rig::populate_baud_choice() {
	if (hamlib_data_.port_type == RIG_PORT_SERIAL) {
		ch_baud_rate_->clear();
		// Override rig's capabilities?
		bn_all_rates_->value(use_all_rates_);

		// Get the baud-rates supported by the rig
		const rig_caps* caps = rig_get_caps(hamlib_data_.model_id);
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
		ch_baud_rate_->value(0);
		// If no values add an empty value
		if (num_rates == 0)	ch_baud_rate_->add("");
		// For all possible rates
		for (int i = 0; i < num_rates; i++) {
			int rate = baud_rates[i];
			if (use_all_rates_ || (rate >= min_baud_rate && rate <= max_baud_rate)) {
				// capabilities overridden or within the range supported by capabilities
				ch_baud_rate_->add(to_string(rate).c_str());
				if (rate == hamlib_data_.baud_rate) {
					ch_baud_rate_->value(index);
					index++;
				}
			}
		}
	}
}


// Model input choice selected
// v is not used
void qso_rig::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	qso_rig* that = ancestor_view<qso_rig>(w);
	const Fl_Menu_Item* item = ch->mvalue();
	rig_model_t id = (intptr_t)item->user_data();
	const char* label = ch->text();
	const rig_caps* capabilities = rig_get_caps(id);
	if (capabilities != nullptr) {
		that->hamlib_data_.model = capabilities->model_name;
		that->hamlib_data_.mfr = capabilities->mfg_name;
		that->hamlib_data_.model_id = id;
		that->hamlib_data_.port_type = capabilities->port_type;
	}
	else {
		char message[128];
		snprintf(message, 128, "DASH: Error reading hamlib details selecting %s", label);
		status_->misc_status(ST_ERROR, message);
	}
	switch (that->hamlib_data_.port_type) {
	case RIG_PORT_SERIAL:
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		that->mode_ = that->hamlib_data_.port_type;
		break;
	case RIG_PORT_NONE:
		// Retain mode = NONE until the add button is clicked
		break;
	}
	that->populate_baud_choice();
	that->populate_port_choice();
	that->enable_widgets();
}

// Callback selecting port
// v is unused
void qso_rig::cb_ch_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_text<Fl_Choice, string>(w, (void*)&that->hamlib_data_.port_name);
}

// Callback entering port
// v is unused
void qso_rig::cb_ip_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value<Fl_Input, string>(w, (void*)&that->hamlib_data_.port_name);
}

// Callback selecting baud-rate
// v is unused
void qso_rig::cb_ch_baud(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	const char* text = ((Fl_Choice*)w)->text();
	that->hamlib_data_.baud_rate = atoi(text);
}

// Override rig capabilities selected - repopulate the baud choice
// v is uused
void qso_rig::cb_ch_over(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->populate_baud_choice();
}

// Select "display all ports" in port choice
// v is a pointer to the all ports flag
void qso_rig::cb_bn_all(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->populate_port_choice();
}

// Pressed the connect button - this is also called from toolbar
// v is not used 
void qso_rig::cb_bn_connect(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	if (!that->rig_) {
		that->rig_ = new rig_if(that->label(), &that->hamlib_data_);
		printf("Connected rig %s\n", that->label());
	}
	else if (that->rig_->is_open()) {
		that->rig_->close();

	} else {
		that->rig_->open();
	}
	that->enable_widgets();
}

// Pressed the select button 
// v is not used 
void qso_rig::cb_bn_select(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	Fl_Light_Button* bn = (Fl_Light_Button*)w;
	if (bn->value()) {
		// This is called in switch_rig
		that->enable_widgets();
	} else {
		that->switch_rig();
	}
}

// Pressed the modify frequency
// v is the bool
void qso_rig::cb_bn_mod_freq(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value<Fl_Check_Button, bool>(w, v);
	that->modify_rig();
	that->enable_widgets();
}

// Typed in freaquency value
// v is the input
void qso_rig::cb_ip_freq(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	float f;
	cb_value_float<Fl_Float_Input>(w, &f);
	*(double*)v = (double)f;
	that->modify_rig();
	that->enable_widgets();
}

// Pressed either power or gain check button
// v: false = gain, true = power
void qso_rig::cb_bn_power(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	bool bn_p = (bool)(intptr_t)v;
	if (bn_p) {
		bool v = that->bn_power_->value();
		if (v) {
			that->modify_power_ = true;
			that->modify_gain_ = false;
		} else {
			that->modify_power_ = false;
			that->modify_gain_ = false;
		}
	} else {
		bool v = that->bn_gain_->value();
		if (v) {
			that->modify_power_ = false;
			that->modify_gain_ = true;
		} else {
			that->modify_power_ = false;
			that->modify_gain_ = false;
		}
	}
	that->modify_rig();
	that->enable_widgets();
}

// Entered gain value
void qso_rig::cb_ip_gain(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value_int<Fl_Int_Input>(w, v);
	that->modify_rig();
	that->enable_widgets();
}

// Entered power value
void qso_rig::cb_ip_power(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	float f;
	cb_value_float<Fl_Float_Input>(w, &f);
	*(double*)v = (double)f;
	that->modify_rig();
	that->enable_widgets();
}

// Clicked start button
void qso_rig::cb_bn_start(Fl_Widget* w, void * v) {
	string* app = (string*)v;
	string command = "bash -i -c " + *app + "&";
	int result = system(command.c_str());
	char msg[100];
	if (result == 0) {
		snprintf(msg, sizeof(msg), "RIG: Started %s OK", command.c_str());
		status_->misc_status(ST_OK, msg);
	} else {
		snprintf(msg, sizeof(msg), "RIG: Failed to start %s", command.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

// Selecting config tab
void qso_rig::cb_config(Fl_Widget* w, void *v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->enable_widgets();
}

// Connect rig if disconnected and vice-versa
void qso_rig::switch_rig() {
	if (rig_) {
		delete rig_;
		rig_ = nullptr;
		rig_ = new rig_if(label(), &hamlib_data_);
	}
	ancestor_view<qso_manager>(this)->update_rig();
	modify_rig();
	enable_widgets();
}

// 1 s clock interface - read rig and update status
//RS_OFF,              // The rig is off or disconnected
//RS_ERROR,            // An error with the rig occured
//RS_RX,               // The rig is connected and in RX mode
//RS_TX,               // The rig is connected and in TX mode
//RS_HIGH              // The rig is in TX mode but SWR is too high
void qso_rig::ticker() {
	if (rig_) {
		rig_->ticker();
		// The rig may have disconnected - update connect/select buttons
		enable_widgets();
	}
}

// New rig 
void qso_rig::new_rig() {
	mode_ = hamlib_data_.port_type;
	enable_widgets();
}

// Return rig pointer
rig_if* qso_rig::rig() {
	return rig_;
}

// Return the colour used in the connect button as its alert
Fl_Color qso_rig::alert_colour() {
	return bn_connect_->color();
}

// Enable rig values
void qso_rig::modify_rig() {
	if (rig_) {
		if (modify_freq_) {
			rig_->set_freq_modifier(freq_offset_);
		} else {
			rig_->clear_freq_modifier();
		}
		if (modify_gain_ && modify_power_) {
			status_->misc_status(ST_SEVERE, "DASH: Trying to set both gain and absolute power");
		} else if (modify_gain_) {
			rig_->set_power_modifier(gain_);
		} else if (modify_power_) {
			rig_->set_power_modifier(power_);
		} else {
			rig_->clear_power_modifier();
		}
	}
}

// Return the preferred antenna
string qso_rig::antenna() {
	return antenna_;
}

// Return call for particular app
string qso_rig::app(modem_t m) {
	return apps_[(int)m];
}

