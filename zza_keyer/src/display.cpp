#include "display.h"

#include "decoder.h"
#include "engine.h"
#include "key_handler.h"
#include "wave_gen.h"

#include "drawing.h"
#include "utils.h"

#include <string>

#include <FL/Enumerations.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Preferences.H>

using namespace std;

extern string VENDOR;
extern string PROGRAM_ID;
extern const map<engine_type, string> engine_descriptors_;
extern wave_gen* wave_gen_;
extern decoder* decoder_;
extern engine* engine_;
extern key_handler* key_handler_;

const int NO_DEVICE = -1;

// Constructor - initialise all variable, load saved data, create the form and create the working bits
display::display(int W, int H, const char* L) :
	Fl_Double_Window(W, H, L),
	target_freq_(0.0),
	wpm_(16.0),
	weight_(3.0),
	shape_(RAMP),
	rise_time_(0.0),
	fall_time_(0.0),
	engine_type_(SQUEEZE_B),
	device_number_(NO_DEVICE)
{
	next_send_ = buffer_;
	memset(buffer_, '\0', sizeof(buffer_));
	load_data();
	create_form();
	enable_widgets();
	populate_devices();
	populate_sources();
	update_speed();
	update_engine();
	update_wavegen();
	redraw();
}

// Destructor
display::~display() {
}

// Create the window display - creates the individual section
void display::create_form() {
	int curr_x = GAP;
	int curr_y = GAP;
	int max_x = 0;
	int max_y = 0;
	create_dev_choice(curr_x, curr_y);
	max_x = max(curr_x, max_x);
	max_y = max(curr_y, max_y);
	curr_x = GAP;
	curr_y += GAP;
	create_settings(curr_x, curr_y);
	max_x = max(curr_x, max_x);
	max_y = max(curr_y, max_y);
	curr_x = GAP;
	curr_y += GAP;
	create_shape(curr_x, curr_y);
	max_x = max(curr_x, max_x);
	max_y = max(curr_y, max_y);
	curr_x = max_x + GAP;
	int save_x = curr_x;
	curr_y = GAP;
	create_source(curr_x, curr_y);
	max_x = max(curr_x, max_x);
	max_y = max(curr_y, max_y);
	curr_x = save_x;
	curr_y += GAP;
	create_monitor(curr_x, curr_y);
	max_x = max(curr_x, max_x);
	max_y = max(curr_y, max_y);
	max_y += GAP;
	max_x += GAP;
	resizable(nullptr);
	size(max_x - x(), max_y - y());
	end();
}

// Create the device selection choice 
void display::create_dev_choice(int& curr_x, int& curr_y) {
	int width = 2 * GAP + WEDIT;
	int height = HTEXT + HBUTTON + GAP;
	g_devices_ = new Fl_Group(curr_x, curr_y, width, height, "Devices");
	g_devices_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_devices_->box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += HTEXT;
	// Choose the relevant HID device
	ch_devices_ = new Fl_Choice(curr_x, curr_y, WEDIT, HBUTTON);
	ch_devices_->callback(cb_device, &device_number_);
	ch_devices_->tooltip("Select the device to connect to");

	curr_x += ch_devices_->w() + GAP;
	curr_y += HBUTTON + GAP;

	g_devices_->end();

}

// Create the CW settings: WPM, weight and audio frequency
void display::create_settings(int& curr_x, int& curr_y) {

	int width = 2 * GAP + 3 * WBUTTON;
	int height = HTEXT + 3 * HBUTTON + GAP;
	g_settings_ = new Fl_Group(curr_x, curr_y, width, height, "Settings");
	g_settings_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_settings_->box(FL_BORDER_BOX);

	curr_x += WBUTTON;
	curr_y += HTEXT;
	vs_freq_ = new Fl_Value_Slider(curr_x, curr_y, 2 * WBUTTON, HBUTTON, "Freq (Hz)");
	vs_freq_->align(FL_ALIGN_LEFT);
	vs_freq_->type(FL_HOR_SLIDER);
	vs_freq_->range(300., 1500.);
	vs_freq_->step(10.0);
	vs_freq_->value(target_freq_);
	vs_freq_->callback(cb_freq, (void*)&target_freq_);
	vs_freq_->when(FL_WHEN_CHANGED);
	vs_freq_->tooltip("Specify the target audio frequency");

	curr_y += HBUTTON;
	vs_wpm_ = new Fl_Value_Slider(curr_x, curr_y, 2 * WBUTTON, HBUTTON, "WPM");
	vs_wpm_->align(FL_ALIGN_LEFT);
	vs_wpm_->type(FL_HOR_SLIDER);
	vs_wpm_->range(6.0, 50.0);
	vs_wpm_->step(1.0);
	vs_wpm_->value(wpm_);
	vs_wpm_->callback(cb_speed, (void*)&wpm_);
	vs_wpm_->tooltip("Please select the required speed in words per minute");

	curr_y += HBUTTON;
	vs_weight_ = new Fl_Value_Slider(curr_x, curr_y, 2 * WBUTTON, HBUTTON, "Weight");
	vs_weight_->align(FL_ALIGN_LEFT);
	vs_weight_->type(FL_HOR_SLIDER);
	vs_weight_->range(2.8, 4.8);
	vs_weight_->step(0.1);
	vs_weight_->value(weight_);
	vs_weight_->callback(cb_speed, (void*)&weight_);
	vs_weight_->tooltip("Please select the required weight (Dash to dot ratio)");

	curr_y += HBUTTON + GAP;
	curr_x += 2 * WBUTTON + GAP;
	g_settings_->end();
	g_settings_->show();
}

// Create the waveform shape - edge shape and rise and fall times
void display::create_shape(int& curr_x, int& curr_y) {
	int height = HTEXT + 3 * HBUTTON + GAP;
	g_shape_ = new Fl_Group(curr_x, curr_y, g_settings_->w(), height, "Edge shaping");
	g_shape_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_shape_->box(FL_BORDER_BOX);

	int save_x = curr_x;
	curr_x += GAP;
	curr_y += HTEXT;
	rg_shape_ = new Fl_Group(curr_x, curr_y, 3 * WBUTTON, HBUTTON);
	// TODO: Create images for the radio button lables
	rb_none_ = new Fl_Radio_Round_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Sharp");
	rb_none_->value(shape_ == (char)SHARP);
	rb_none_->callback(cb_shapes, (void*)(char)SHARP);
	rb_none_->tooltip("Select for immediate rise and fall");

	curr_x += WBUTTON;
	rb_ramp_ = new Fl_Radio_Round_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Ramp");
	rb_ramp_->value(shape_ == (char)RAMP);
	rb_ramp_->callback(cb_shapes, (void*)(char)RAMP);
	rb_ramp_->tooltip("Select for straight ramp rise and fall");

	curr_x += WBUTTON;
	rb_cosine_ = new Fl_Radio_Round_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Cosine");
	rb_cosine_->value(shape_ == (char)COSINE);
	rb_cosine_->callback(cb_shapes, (void*)(char)COSINE);
	rb_cosine_->tooltip("Select for curved ramp rise and fall");

	rg_shape_->end();
	
	curr_x = save_x + WBUTTON;
	curr_y += HBUTTON;
	vs_rise_ = new Fl_Value_Slider(curr_x, curr_y, 2 * WBUTTON, HBUTTON, "Rise (ms)");
	vs_rise_->align(FL_ALIGN_LEFT);
	vs_rise_->type(FL_HOR_SLIDER);
	vs_rise_->range(0.5, 10.0);
	vs_rise_->step(0.5);
	vs_rise_->value(rise_time_);
	vs_rise_->callback(cb_times, (void*)&rise_time_);
	vs_rise_->tooltip("Please select the required rise time (in milliseconds)");

	curr_y += HBUTTON;
	vs_fall_ = new Fl_Value_Slider(curr_x, curr_y, 2 * WBUTTON, HBUTTON, "Fall (ms)");
	vs_fall_->align(FL_ALIGN_LEFT);
	vs_fall_->type(FL_HOR_SLIDER);
	vs_fall_->range(0.5, 10.0);
	vs_fall_->step(0.5);
	vs_fall_->value(fall_time_);
	vs_fall_->callback(cb_times, (void*)&fall_time_);
	vs_fall_->tooltip("Please select the required fall time (in milliseconds)");

	curr_x += vs_fall_->w() + GAP;
	curr_y += HBUTTON + GAP;

	g_shape_->end();
}

// Create the selectors for the keying source - input paddle or keyboard entry
// If paddle the choose the type: Straight key, squeeze or bug
void display::create_source(int& curr_x, int& curr_y) {
	int height = HTEXT +  2 * HBUTTON + GAP + 2 * HBUTTON + GAP;
	g_source_ = new Fl_Group(curr_x, curr_y, g_settings_->w(), height, "Source");
	g_source_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_source_->box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += HTEXT;
	int save_x = curr_x;

	ch_engine_ = new Fl_Choice(curr_x, curr_y, 3 * WBUTTON, HBUTTON);
	ch_engine_->callback(cb_engine, &engine_type_);
	ch_engine_->tooltip("Select the engine type");

	curr_y += HBUTTON + GAP;
	curr_x = save_x;

	ip_entry_ = new Fl_Input(curr_x, curr_y, 3 * WBUTTON, HBUTTON);
	ip_entry_->callback(cb_editor);
	ip_entry_->when(FL_WHEN_CHANGED);
	ip_entry_->tooltip("Type in data to send to the remote station");


	curr_y += ip_entry_->h() + GAP;

	g_source_->end();
}

// Display decode of the generated morse code
void display::create_monitor(int& curr_x, int& curr_y) {
	int height = HTEXT + 4 * HBUTTON + GAP;
	g_monitor_ = new Fl_Group(curr_x, curr_y, g_settings_->w(), height, "Monitor");
	g_monitor_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_monitor_->box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += HTEXT;

	td_monitor_ = new Fl_Text_Display(curr_x, curr_y, 3 * WBUTTON, 4 * HBUTTON);
	Fl_Text_Buffer* buffer = new Fl_Text_Buffer;
	td_monitor_->buffer(buffer);
	td_monitor_->tooltip("Displays the code sent");

	curr_x += td_monitor_->w() + GAP;
	curr_y += td_monitor_->h() + GAP;

	g_monitor_->end();

}

// Load any saved data
void display::load_data() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	// This variable is used for char and bool settings.
	int temp_i;
	settings.get("Target Freqency", target_freq_, 700.0);
	settings.get("Words per Minute", wpm_, 16.0);
	settings.get("Weight", weight_, 3.0);
	settings.get("Key Envelope", temp_i, (int)RAMP);
	shape_ = (shape_t)temp_i;
	settings.get("Rise Time", rise_time_, 5.0);
	settings.get("Fall Time", fall_time_, rise_time_);
	settings.get("Engine Type", temp_i, (int)SQUEEZE_A);
	engine_type_ = (engine_type)temp_i;
}

// Configure the various widgets
void display::enable_widgets() {
	enable_rise_fall();
	enable_editor();
	enable_devices();
	enable_engine();
}

// Only enable the rise and fall time selectors for shaped edges
void display::enable_rise_fall() {
	// Rise and fall times are not relevant with shape = NONE
	if (shape_ == SHARP) {
		vs_fall_->deactivate();
		vs_rise_->deactivate();
	}
	else {
		vs_fall_->activate();
		vs_rise_->activate();
	}
}

// Enable the keybaord entry pad if using it
void display::enable_editor() {
	// Keyboard entry
	if (engine_type_ == KEYBOARD) {
		ip_entry_->activate();
	}
	else {
		ip_entry_->deactivate();
	}
}

// Enable HID device selector if we have no valid device
void display::enable_devices() {
	if (device_number_ == NO_DEVICE) {
		ch_devices_->activate();
	}
	else {
		ch_devices_->deactivate();
	}
}

// Enable engine type choice - only if engine is idle
void display::enable_engine() {
	if (engine_->idle()) {
		ch_engine_->activate();
	}
	else {
		ch_engine_->deactivate();
	}
}

// Store data between application calls
void display::save_data() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	// This variable is used for char and bool settings.
	settings.set("Target Freqency", target_freq_);
	settings.set("Words per Minute", wpm_);
	settings.set("Weight", weight_);
	settings.set("Key Envelope", (int)shape_);
	settings.set("Rise Time", rise_time_);
	settings.set("Fall Time", fall_time_);
	settings.set("Engine Type", (int)engine_type_);
}

// Callback - select device
void display::cb_device(Fl_Widget* w, void* v) {
	display* that = ancestor_view<display>(w);
	that->device_number_ = ((Fl_Choice*)w)->value();
	if (!key_handler_->open_device(that->device_number_)) {
		fl_message("Device failed to open!");
		that->device_number_ = NO_DEVICE;
	}
	that->enable_devices();
}

// Callback - speed: save value, update paddle/keyboard SSMs
void display::cb_speed(Fl_Widget* w, void* v) {
	display* that = ancestor_view<display>(w);
	double d = ((Fl_Value_Slider*)w)->value();
	*(double*)v = d;
	that->update_speed();
}

// Callback - frequency: save value - update wave_gen
void display::cb_freq(Fl_Widget* w, void* v) {
	display* that = ancestor_view<display>(w);
	double d = ((Fl_Value_Slider*)w)->value();
	*(double*)v = d;
	that->update_wavegen();
}

// Callback - shapes: save value, enable rise/fall, update wave_gen
void display::cb_shapes(Fl_Widget* w, void* v) {
	display* that = ancestor_view<display>(w);
	that->shape_ = (shape_t)(intptr_t)v;
	that->enable_rise_fall();
	that->update_wavegen();
}

// Callback - times: save value, update wave_gen
void display::cb_times(Fl_Widget* w, void* v) {
	display* that = ancestor_view<display>(w);
	double d = ((Fl_Value_Slider*)w)->value();
	*(double*)v = d;
	that->update_wavegen();
}

// Callback - paddle config: save value, update paddle_sm
void display::cb_engine(Fl_Widget* w, void* v) {
	display* that = ancestor_view<display>(w);
	that->engine_type_ = (engine_type)((Fl_Choice*)w)->value();
	that->update_engine();
	that->enable_editor();
	that->enable_engine();
}

// Callback - editor: save value - start sending data
void display::cb_editor(Fl_Widget* w, void* v) {
	display* that = ancestor_view<display>(w);
	memcpy(that->buffer_, ((Fl_Input*)w)->value(), 1024);
	that->update_editor(NEW_CHARACTER);
}

// Callback - monitor data ready
void display::cb_monitor(void* v) {
	display* that = (display*)v;
	that->update_monitor();
}

// Callback - kb sending done
void display::cb_kb_done(void* v) {
	display* that = (display*)v;
	that->update_editor(SEND_DATA);
}

// Update paddle and keyboard SMs with speed values
void display::update_speed() {
	decoder_->set_speed(wpm_, weight_);
	engine_->set_speed(wpm_, weight_);
}

// Update wave_gen with frequency and shaping values
void display::update_wavegen() {
	wave_gen_->set_params(48000.0, target_freq_, rise_time_ / 1000.0, fall_time_ / 1000.0, shape_);
	// Get and display the actual frequency that got set.
	double actual = wave_gen_->get_audio_freq(true);
	vs_freq_->value(actual);
	vs_freq_->redraw();
}

// Update paddle with configuration
void display::update_engine() {
	engine_->type(engine_type_);
}

// Update monitor with data from decoder
void display::update_monitor() {
	decoder_->get_speed(wpm_, weight_);
	vs_wpm_->value(wpm_);
	vs_weight_->value(weight_);
	char c = decoder_->get_char();
	td_monitor_->buffer()->append(&c, 1);
}

// Update editor - TODO: what was this supposed to do?
void display::update_editor(edit_event e) {
	// If sending is enabled and there is data to be sent
	if (*next_send_ != '\0') {
		// Send it
		engine_->send(*next_send_++);
	}
}

// Populate the devices choice
void display::populate_devices() {
	ch_devices_->clear();
	if (key_handler_) {
		vector<string>* devices = key_handler_->get_devices();
		int ix = 0;
		char text[128];
		for (auto it = devices->begin(); it != devices->end(); it++, ix++) {
			snprintf(text, sizeof(text), "%02d %s", ix, ((*it).c_str()));
			ch_devices_->add(text);
		}
	}
}

// Populate the sources choice
void display::populate_sources() {
	ch_engine_->clear();
	if (engine_) {
		for (auto it = engine_descriptors_.begin(); it != engine_descriptors_.end(); it++) {
			ch_engine_->add((*it).second.c_str());
		}
	}
}


