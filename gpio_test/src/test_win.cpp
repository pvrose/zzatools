#include "test_win.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Radio_Round_Button.H>

test_win::test_win() :
	Fl_Window(100, 100),
	devices_(nullptr),
	device_num_(0)
{
	label("GPIO Driver test");
	for (int ix = 0; ix < gpio::NUM_GPIO; ix++) {
		is_output_[ix] = true;
		pin_value_[ix] = false;
	}
	// Get all the devices
	gpio_driver_ = new gpio;
	create_form();
	enable_widgets();
}

test_win::~test_win() {
	delete gpio_driver_;
	devices_ = nullptr;
}

// Create the widgets
void test_win::create_form() {
	// Calculate the sizes and set windo size accordingly
	const int WPIN = WBUTTON;
	const int HPIN = 2 * GAP + 2 * HBUTTON;
	const int WWIN = (gpio::NUM_GPIO * WPIN) + (5 * GAP);
	const int WCHOICE = WWIN - 3 * GAP - WBUTTON;
	const int HWIN = HTEXT + HBUTTON + GAP + HPIN + GAP;
	size(WWIN, HWIN);

	int curr_x = GAP;
	int curr_y = HTEXT;

	ch_device_ = new Fl_Choice(curr_x, curr_y, WCHOICE, HBUTTON, "Device");
	ch_device_->align(FL_ALIGN_TOP);
	ch_device_->callback(cb_devices, &device_num_);
	ch_device_->when(FL_WHEN_RELEASE_ALWAYS);

	populate_devices(ch_device_);

	curr_x += ch_device_->w() + GAP;
	bn_rescan_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Rescan");
	bn_rescan_->callback(cb_rescan, nullptr);

	curr_x = GAP;
	curr_y += HBUTTON + GAP;
	for (int ix = 0; ix < gpio::NUM_GPIO; ix++) {
		g_pin_[ix] = new Fl_Group(curr_x, curr_y, WPIN, HPIN);
		g_pin_[ix]->align(FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE);
		char l[16];
		snprintf(l , sizeof(l), "Pin %d", ix);
		g_pin_[ix]->copy_label(l);
		// Now the buttons
		int save_x = curr_x;
		int save_y = curr_y;
		rb_in_[ix] = new Fl_Radio_Round_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "I");
		rb_in_[ix]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		rb_in_[ix]->callback(cb_inout, (void*)(intptr_t)ix);
		curr_x += WBUTTON / 2;
		rb_out_[ix] = new Fl_Radio_Round_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "O");
		rb_out_[ix]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		rb_out_[ix]->callback(cb_inout, (void*)(intptr_t)ix);
		curr_y += HBUTTON;
		curr_x = save_x;
		lb_value_[ix] = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Value");
		lb_value_[ix]->callback(cb_op_drive, (void*)(intptr_t)ix);
		g_pin_[ix]->end();
		curr_x = save_x + WPIN + GAP;
		curr_y = save_y;
	}
	end();
}

// Configure the values
void test_win::enable_widgets() {
	for (int ix = 0; ix < gpio::NUM_GPIO; ix++) {
		if (is_output_[ix]) {
			rb_in_[ix]->value(false);
			rb_out_[ix]->value(true);
			lb_value_[ix]->when(FL_WHEN_CHANGED);
			lb_value_[ix]->value(pin_value_[ix]);
			lb_value_[ix]->selection_color(FL_GREEN);
		}
		else {
			rb_in_[ix]->value(true);
			rb_out_[ix]->value(false);
			lb_value_[ix]->when(FL_WHEN_NEVER);
			lb_value_[ix]->value(pin_value_[ix]);
			lb_value_[ix]->selection_color(FL_RED);
		}
	}
}

// Callbacks
// choice select device to connect to
void test_win::cb_devices(Fl_Widget* w, void* v) {
	test_win* that = ancestor_view<test_win>(w);
	Fl_Choice* ch = (Fl_Choice*)w;
	that->device_num_ = ch->value();
	// and open it
	that->gpio_driver_->open(that->device_num_);
	Fl::add_timeout(0.0, cb_timer, that);
	that->enable_widgets();
}

// radio button selection - v is pin number, how do we pass whether in or out (widget label)
void test_win::cb_inout(Fl_Widget* w, void* v) {
	test_win* that = ancestor_view<test_win>(w);
	Fl_Radio_Round_Button* rb = (Fl_Radio_Round_Button*)w;
	int pin = (int)(intptr_t)v;
	if (rb->value() == 1) {
		if (rb->label()[0] == 'I') {
			that->is_output_[pin] = false;
		}
		else {
			that->is_output_[pin] = true;
		}
	}
	that->enable_widgets();
	that->update_gpio(pin);
}

// output pin drive - v is pin number
void test_win::cb_op_drive(Fl_Widget* w, void* v) {
	test_win* that = ancestor_view<test_win>(w);
	Fl_Light_Button* lb = (Fl_Light_Button*)w;
	int pin = (int)(intptr_t)v;
	if (that->is_output_[pin]) {
		that->pin_value_[pin] = (bool)(intptr_t)lb->value();
		that->enable_widgets();
		that->update_gpio(pin);
	}
	else {
		that->enable_widgets();
	}
}

// Timer callback
const double POLLING_PERIOD = 5.0;
void test_win::cb_timer(void* v) {
	test_win* that = (test_win*)v;
	that->read_pin_values();
	Fl::repeat_timeout(POLLING_PERIOD, cb_timer, that);
}

// Rescan call back
void test_win::cb_rescan(Fl_Widget* w, void* v) {
	test_win* that = ancestor_view<test_win>(w);
	Fl::remove_timeout(cb_timer, that);
	that->gpio_driver_->close();
	that->devices_ = that->gpio_driver_->get_devices(0, 0);
	that->populate_devices(that->ch_device_);
}

// Read pin values
void test_win::read_pin_values() {
	pin_value_[0] = gpio_driver_->value(gpio::GPIO_0);
	pin_value_[1] = gpio_driver_->value(gpio::GPIO_1);
	pin_value_[2] = gpio_driver_->value(gpio::GPIO_2);
	pin_value_[3] = gpio_driver_->value(gpio::GPIO_3);
	enable_widgets();
}

void test_win::populate_devices(Fl_Choice* ch) {
	if (devices_) devices_->clear();
	devices_ = gpio_driver_->get_devices(0, 0);
	ch->clear();
	for (int ix = 0; ix < devices_->size(); ix++) {
		char temp[128];
		snprintf(temp, sizeof(temp), "%02d - %s", ix, (*devices_)[ix].c_str());
		string name = escape_menu(temp);
		ch->add(name.c_str());
	}
}

void test_win::update_gpio(int pin) {
	gpio::pin_no_t pin_bit = (gpio::pin_no_t)(1 << pin);
	// Send commands for the pin
	if (is_output_[pin]) {
		gpio_driver_->direction(pin_bit, gpio::OUTPUT);
		gpio_driver_->value(pin_bit, pin_value_[pin]);
	}
	else {
		gpio_driver_->direction(pin_bit, gpio::INPUT);

	}
}
