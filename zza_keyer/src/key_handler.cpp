#include "key_handler.h"

#include "gpio.h"

#include <chrono>

using namespace std::chrono;

const int SAMPLE_PERIOD_MS = 10;

key_handler::key_handler() {
	gpio_ = new gpio;
	t_key_ = nullptr;
	close_ = false;
	current_keys_ = NEITHER;
}

key_handler::~key_handler() {
	close_device();
}

// Get the list of devices
vector<string>* key_handler::get_devices() {
	return gpio_->get_devices(0, 0);
}

// Open the nth device
bool key_handler::open_device(int index) {
	if (gpio_->open(index)) {
		gpio_->direction(gpio::GPIO_0, gpio::INPUT);
		gpio_->direction(gpio::GPIO_1, gpio::INPUT);
		gpio_->direction(gpio::GPIO_2, gpio::OUTPUT);
		// Now start the polling thread
		t_key_ = new thread(run_thread, this);
		return true;
	}
	else {
		return false;
	}
}

// Close the device
bool key_handler::close_device() {
	// Let thread wind down
	close_ = true;
	if (t_key_) {
		t_key_->join();
	}
	// Close the GPIO
	if (gpio_) {
		gpio_->close();
	}
	delete gpio_;
	gpio_ = nullptr;
	return true;
}

// Set the LED out
void key_handler::drive_led(bool b) {
	if (gpio_) {
		gpio_->value(gpio::GPIO_2, b);
	}
}

// Set/get reversed
void key_handler::reversed(bool b) {
	reversed_ = b;
}

bool key_handler::reversed() {
	return reversed_;
}

// Get the pin state
key_state key_handler::get_state() {
	return current_keys_;
}

// Thread to run
void key_handler::run_thread(key_handler* that) {
	that->run_key();
}

// And non-static version
void key_handler::run_key() {
	current_keys_ = NEITHER;
	while (!close_) {
		// Continue sampling pins every few milliseconds
		unsigned char c = gpio_->values();
		switch (c & 3) {
		case 0: 
			current_keys_ = NEITHER;
			break;
		case 1:
			if (reversed_) current_keys_ = RIGHT;
			else current_keys_ = LEFT;
			break;
		case 2:
			if (reversed_) current_keys_ = LEFT;
			else current_keys_ = RIGHT;
			break;
		case 3:
			current_keys_ = BOTH;
			break;
		}
		this_thread::sleep_for(milliseconds(SAMPLE_PERIOD_MS));
	}

}
