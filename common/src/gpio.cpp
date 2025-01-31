#include "gpio.h"


// Include the HIDAPI driver
#include "hidapi/hidapi.h"

#if defined(_WIN32) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <windows/hidapi_winapi.h>
#endif

#if defined(USING_HIDAPI_LIBUSB) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <libusb/hidapi_libusb.h>
#endif
//
#include <cstdio>
#include <chrono>
#include <cwchar>
#include <cstring>

using namespace std;
using namespace std::chrono;

gpio::gpio() {
	for (int ix = 0; ix < NUM_GPIO; ix++) {
		pin_state_[ix] = { OUTPUT, false };
	}
	changed_pins_ = (pin_no_t)0;
	device_ = nullptr;
	memset(manufacturer_, 0, sizeof(manufacturer_));
	memset(product_, 0, sizeof(product_));
	memset(serial_no_, 0, sizeof(serial_no_));
	memset(command_, 0, sizeof(command_));
	memset(response_, 0, sizeof(response_));
	hid_init();
}

gpio::~gpio() {
	hid_close(device_);
	hid_exit();
}

// VID and PID will probably always be 0x04d8, and 0x00dd and ser_no nullptr
// Return true on success
bool gpio::open(unsigned short vid, unsigned short pid, const wchar_t* ser_no) {
	device_ = hid_open(vid, pid, ser_no);
	if (device_ == nullptr) {
		printf("Unable to open device %0x/%0x\n", vid, pid);
		hid_exit();
		return false;
	}
#if defined(_WIN32) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 15, 0)
	hid_winapi_set_write_timeout(device_, 5000);
#endif
	manufacturer_[0] = 0x0000;
	int result = hid_get_manufacturer_string(device_, manufacturer_, sizeof(manufacturer_));
	if (result < 0) {
		printf("Unable to read manufacturer\n");
	}
	product_[0] = 0x0000;
	result = hid_get_product_string(device_, product_, sizeof(product_));
	if (result < 0) {
		printf("Unable to read product\n");
	}
	serial_no_[0] = 0x0000;
	result = hid_get_serial_number_string(device_, serial_no_, sizeof(serial_no_));
	if (result < 0) {
		printf("Unable to read serial number\n");
	}
#ifdef _DEBUG
	printf("Device read %ls %ls %ls\n", manufacturer_, product_, serial_no_);
#endif
	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(device_, 1);
	// Initiatalise the GPIO state - all input
	for (int ix = 0; ix < NUM_GPIO; ix++) {
		pin_state_[ix] = { OUTPUT, true };
	}
	//read_status(CHIP);
	//read_status(GPIO);
	//read_status(MFR);
	//read_status(PRODUCT);
	//read_status(SERIAL);
	//read_status(F_SERIAL);
	write_sram();
	read_sram();
	write_gpio(DIRECTION);
	write_gpio(VALUE);
	read_gpio(VALUE);
	//read_sram();
	return true;
}

bool gpio::open(int index) {
	hid_device_info* dev = device_list_;
	for (int ix = 0; dev->next != nullptr; ix++) {
		if (ix == index) {
			return open(dev->vendor_id, dev->product_id, dev->serial_number);
		}
		dev = dev->next;
	}
	return false;
}

bool gpio::close() {
	if (device_) hid_close(device_);
	return true;
}

vector<string>* gpio::get_devices(unsigned short vid, unsigned short pid) {
	if (device_) {
		hid_close(device_);
		hid_exit();
		hid_init();
	}
	device_list_ = hid_enumerate(vid, pid);
	hid_device_info* dev = device_list_;
	vector<string>* result = new vector<string>;
	wchar_t* src;
	result->clear();
	while (dev && dev->next != nullptr) {
		char name[128];
		memset(name, 0x0, sizeof(name));
		char temp[128];
		src = dev->manufacturer_string;
		wcsrtombs(temp, (const wchar_t**) & src, sizeof(temp), nullptr);
		strcpy(name, temp);
		strncat(name, " - ", sizeof(name));
		src = dev->product_string;
		wcsrtombs(temp, (const wchar_t**)&src, sizeof(temp), nullptr);
		strncat(name, temp, sizeof(name));
		strncat(name, " - ", sizeof(name));
		src = dev->serial_number;
		wcsrtombs(temp, (const wchar_t**)&src, sizeof(temp), nullptr);
		strncat(name, temp, sizeof(name));
		strncat(name, " - ", sizeof(name));
		result->push_back(string(name));
		dev = dev->next;
	}
	return result;
}

// Get direction
gpio::direction_t gpio::direction(pin_no_t pin) {
	switch (pin) {
	case GPIO_0: return pin_state_[0].direction;
	case GPIO_1: return pin_state_[1].direction;
	case GPIO_2: return pin_state_[2].direction;
	case GPIO_3: return pin_state_[3].direction;
	}
	return NOT_GPIO;
}

// Set direction
bool gpio::direction(pin_no_t pin, direction_t d) {
	switch (pin) {
	case GPIO_0:
		pin_state_[0].direction = d;
		break;
	case GPIO_1:
		pin_state_[1].direction = d;
		break;
	case GPIO_2:
		pin_state_[2].direction = d;
		break;
	case GPIO_3:
		pin_state_[3].direction = d;
		break;
	}
	write_gpio(DIRECTION);
	return true;
}

// Get value
bool gpio::value(pin_no_t p) {
	read_gpio(VALUE);
	switch (p) {
	case GPIO_0: return pin_state_[0].value;
	case GPIO_1: return pin_state_[1].value;
	case GPIO_2: return pin_state_[2].value;
	case GPIO_3: return pin_state_[3].value;
	}
	return false;
}

// Get all values
unsigned char gpio::values() {
	read_gpio(VALUE);
	unsigned char c = 0;
	if (pin_state_[0].value) c |= 1;
	if (pin_state_[1].value) c |= 2;
	if (pin_state_[2].value) c |= 4;
	if (pin_state_[3].value) c |= 8;
	return c;
}

// Set value
bool gpio::value(pin_no_t p, bool v) {
	switch (p) {
	case GPIO_0:
		pin_state_[0].value = v;
		break;
	case GPIO_1:
		pin_state_[1].value = v;
		break;
	case GPIO_2:
		pin_state_[2].value = v;
		break;
	case GPIO_3:
		pin_state_[3].value = v;
		break;
	}
	write_gpio(VALUE);
	return true;
}

// Set the GPIO pin direction from the pin_states
void gpio::gen_w_dir_cmd() {
	//command_[0] = 0x01; // Command for GPIO
	int ix = 0;
	command_[ix++] = 0x01;
	command_[ix++] = 0x50; // Set GPIO Output values
	command_[ix++] = 0x00; // Don't care
	for (int p = 0; ix < 18; p++) {
		command_[ix++] = 0x00; // Don't set output value
		command_[ix++] = 0x00; // Ignored if command_[3] = 0
		command_[ix++] = 0x01; // Set direction
		command_[ix++] = pin_state_[p].direction == OUTPUT ? 0x00 : 0x01; // Direction
	}
	for (; ix < 65; ix++) {
		command_[ix] = 0x00; // Reserved
	}
}

// Set the GPIO pin values from the pin states
void gpio::gen_w_val_cmd() {
	//command_[0] = 0x01; // Command for GPIO
	int ix = 0;
	command_[ix++] = 0x01;
	command_[ix++] = 0x50; // Set GPIO Output values
	command_[ix++] = 0x00; // Don't care
	for (int p = 0; ix < 18; p++) {
		command_[ix++] = 0x01; // Set output value
		command_[ix++] = pin_state_[p].value ? 0x01 : 0x00; // Pin value
		command_[ix++] = 0x00; // Don't set direction
		command_[ix++] = 0x00; // Ignored
	}
	for (; ix < 65; ix++) {
		command_[ix] = 0x00; // Reserved
	}

}
void gpio::gen_read_cmd() {
	//command_[0] = 0x01; // GPIO
	int ix = 0;
	command_[ix++] = 0x01;
	command_[ix++] = 0x51; // Get GPIO values
	for (; ix < 65; ix++) {
		command_[ix] = 0x00;   // Ignored
	}
}

// generate GPIO status command
void gpio::gen_status_cmd(status_t type) {
	int ix = 0;
	command_[ix++] = 0x01;
	command_[ix++] = 0xB0;
	command_[ix++] = type;
	for (; ix < 65; ix++) {
		command_[ix] = 0x00;   // Ignored
	}
}


// Check response is as command unless value is 0xEE - returns true if OK
bool gpio::dcd_write_rsp() {
	bool result = true;
	int ix = 0;
	if (response_[ix++] != 0x50) result = false;
	for (int p = 0; ix < 18 && p < NUM_GPIO; p++) {
		for (int iy = 0; iy < 4; iy++) {
			if (response_[ix] == 0xee) {
				pin_state_[p].direction = NOT_GPIO;
			} 
			else {
				if (response_[ix] != command_[ix + 1]) result = false;
			}
			ix++;
		}
	}
	return result;
}

// Read response - check directions as expected - update values
bool gpio::dcd_read_rsp() {
	bool result = true;
	int ix = 0;
	if (response_[ix++] != 0x51) result = false;
	if (response_[ix++] != 0x00) result = false;
	for (int p = 0; p < 4; p++) {
		unsigned val = response_[ix++];
		unsigned dir = response_[ix++];
		if ((val == 0xEE || dir == 0xEF) && pin_state_[p].direction != NOT_GPIO)
			printf("GPIO %d unexpectedly not a GPIO pin\n", p);
		switch (dir) {
		case 0x00: 
			if (pin_state_[p].direction != OUTPUT) {
				printf("GPIO %d unexpectedly an Output pin\n", p);
				pin_state_[p].direction = OUTPUT;
			}
			if (pin_state_[p].value != (bool)val) {
				printf("GPIO %d has value %d not %d as expected", p, val, pin_state_[p].value);
				pin_state_[p].value = val;
			}
			break;
		case 0x01:
			if (pin_state_[p].direction != INPUT) {
				printf("GPIO %d unexpectedly an Input pin\n", p);
				pin_state_[p].direction = INPUT;
			}
#ifdef _DEBUG
			if (pin_state_[p].value != (bool)val) {
				printf("GPIO %d has changed value to %d", p, val);
			}
#endif
			pin_state_[p].value = val;

		}
#ifdef _DEBUG
		printf("GPIO %d set direction %s value %d\n", p, dir == 0x00 ? "out" : dir == 0x01 ? "in" : "Not gpio", val);
#endif
	}
	return result;
}

// write state to GPIO
bool gpio::write_gpio(write_t type) {
	int result = 0;
	bool res = true;
	memset(command_, 0x00, 65);
	memset(response_, 0x00, 65);
	switch (type) {
	case NONE:
		// Do nothing
		break;
	case DIRECTION:
	case VALUE:
		// Generate command
		if (type == DIRECTION) gen_w_dir_cmd();
		else gen_w_val_cmd();
		result = hid_write(device_, command_, 65);
		if (result < 0) {
			// Bad write
			printf("Unexpected error from HID_write - %ls\n", hid_error(device_));
			res = false;
		}
		else {
			result = 0;
			while (result == 0) {
				// Assume it's non-blocking
				result = hid_read(device_, response_, 65);
			}
			if (result < 0) {
				// Bad write
				printf("Unexpected error from HID_read - %ls\n", hid_error(device_));
				res = false;
			}
			else {
				if (!dcd_write_rsp() || result != 64) {
					printf("Incorrect write response\n");
					res = false;
				}
			}
			trace_cr();
		}
		break;
	}
	return res;
}

// read state from GPIO
bool gpio::read_gpio(write_t type) {
	int result = 0;
	bool res = true;
	memset(command_, 0x00, 65);
	memset(response_, 0x00, 65);
	switch (type) {
	case NONE:
		// Do nothing
		break;
	case DIRECTION:
	case VALUE:
		gen_read_cmd();
		result = hid_write(device_, command_, 65);
		if (result < 0) {
			// Bad write
			printf("Unexpected error from HID_write - %ls\n", hid_error(device_));
			res = false;
		}
		else {
			result = 0;
			while (result == 0) {
				// Assume it's non-blocking
				result = hid_read(device_, response_, 65);
			}
			if (result < 0) {
				// Bad write
				printf("Unexpected error from HID_read - %ls\n", hid_error(device_));
				res = false;
			}
			else {
				if (!dcd_read_rsp() || result != 64) {
					printf("Incorrect read response\n");
					res = false;
				}
			}
			trace_cr();
		}
		break;
	}
	return res;
}

void gpio::trace_cr() {
#ifdef _DEBUG
	printf("Command:");
	for (int ix = 0; ix < sizeof(command_); ix++) {
		printf("%02x ", command_[ix]);
	}
	printf("\nResponse:");
	for (int ix = 0; ix < sizeof(response_); ix++) {
		printf("%02x ", response_[ix]);
	}
	printf("\n");
#endif
}

bool gpio::read_status(status_t type) {
	int result = 0;
	bool res = true;
	memset(command_, 0x00, 65);
	memset(response_, 0x00, 65);
	gen_status_cmd(type);
	result = hid_write(device_, command_, 65);
	if (result < 0) {
		// Bad write
		printf("Unexpected error from HID_write - %ls\n", hid_error(device_));
		res = false;
	}
	else {
		result = 0;
		while (result == 0) {
			// Assume it's non-blocking
			result = hid_read(device_, response_, 65);
		}
		if (result < 0) {
			// Bad write
			printf("Unexpected error from HID_read - %ls\n", hid_error(device_));
			res = false;
		}
		else {
			if (result != 64 || !dcd_status_rsp(type)) {
				printf("Incorrect status response\n");
				res = false;
			}
		}
		printf("GPIO status\n");
		trace_cr();
	}
	return res;
}

bool gpio::dcd_status_rsp(status_t type) {
	int ix = 0;
	int len = 0;
	bool result = true;
	unsigned char val;
	unsigned short sval;
	if (response_[ix++] != 0xB0) return false;
	if (response_[ix++] != 0x00) return false;
	len = response_[ix++];
	switch (type) {
	case CHIP: {
		ix++;     // Byte 3 don't care
		val = response_[ix++];
		// Byte 4 - bit significant
		if (val & 0x80) printf("CDC Serial number enabled\n");
		if (val & 0x40) printf("LED UART TX set\n");
		if (val & 0x20) printf("LED UART RX set\n");
		if (val & 0x10) printf("LED I2C set\n");
		if (val & 0x08) printf("SSPND set\n");
		if (val & 0x04) printf("USB CFG set\n");
		if (val & 0x02) printf("Permanently locked\n");
		else if (val & 0x01) printf("Password protected\n");
		val = response_[ix++];
		// Byte 5
		printf("Clock divider value %d\n", val & 0x1F);
		val = response_[ix++];
		// Byte 6
		if ((val & 0xC0) == 0xC0) printf("DAC VREF = 4.096V\n");
		if ((val & 0xC0) == 0x80) printf("DAC VREF = 2.048V\n");
		if ((val & 0xC0) == 0x40) printf("DAC VREF = 1.024V\n");
		if ((val & 0xC0) == 0x00) printf("DAC VREF not set\n");
		if (val & 0x20) printf("DAC voltage = VRM\n");
		val = response_[ix++];
		// Byte 7
		if (val & 0x40) printf("Negedge interrupt\n");
		if (val & 0x20) printf("Posedge interrupt\n");
		if ((val & 0x18) == 0x18) printf("ADC VREF = 4.096V\n");
		if ((val & 0x18) == 0x10) printf("ADC VREF = 2.048V\n");
		if ((val & 0x18) == 0x08) printf("ADC VREF = 1.024V\n");
		if ((val & 0x18) == 0x00) printf("ADC VREF not set\n");
		if (val & 0x04) printf("DAC VREF = VDD\n");
		sval = response_[ix++];
		sval += (response_[ix++] << 8);
		printf("VID = %d\n", sval);
		sval = response_[ix++];
		sval += (response_[ix++] << 8);
		printf("PID = %d\n", sval);
		val = response_[ix++];
		printf("USB Power attributes %x\n", val);
		val = response_[ix++];
		printf("USB current request %d mA\n", (unsigned short)val * 2);
		break;
	}
	case GPIO:{
		ix++; // Byte 3 don't care
		val = response_[ix++];
		// Byte 4 - GPIO power up state
		if (val & 0x10) printf("GPIO 0 set 1\n");
		else printf("GPIO 0 set 0\n");
		if (val & 0x08) printf("GPIO 0 input\n");
		if ((val & 0x07) == 0x02) printf("GPIO 0 LED UART RX\n");
		if ((val & 0x07) == 0x01) printf("GPIO 0 SSPND\n");
		if ((val & 0x07) == 0x00) printf("GPIO 0 GPIO\n");
		val = response_[ix++];
		// Byte 5 - GPIO power up state
		if (val & 0x10) printf("GPIO 1 set 1\n");
		else printf("GPIO 1 set 0\n");
		if (val & 0x08) printf("GPIO 1 input\n");
		if ((val & 0x07) == 0x04) printf("GPIO 1 Interrupt detection\n");
		if ((val & 0x07) == 0x03) printf("GPIO 1 LED UART TX\n");
		if ((val & 0x07) == 0x02) printf("GPIO 1 ADC1\n");
		if ((val & 0x07) == 0x01) printf("GPIO 1 Clock\n");
		if ((val & 0x07) == 0x00) printf("GPIO 1 GPIO\n");
		val = response_[ix++];
		// Byte 4 - GPIO power up state
		if (val & 0x10) printf("GPIO 2 set 1\n");
		else printf("GPIO 2 set 0\n");
		if (val & 0x08) printf("GPIO 2 input\n");
		if ((val & 0x07) == 0x03) printf("GPIO 2 DAC1\n");
		if ((val & 0x07) == 0x02) printf("GPIO 2 ADC2\n");
		if ((val & 0x07) == 0x01) printf("GPIO 2 USB\n");
		if ((val & 0x07) == 0x00) printf("GPIO 2 GPIO\n");
		val = response_[ix++];
		// Byte 5 - GPIO power up state
		if (val & 0x10) printf("GPIO 3 set 1\n");
		else printf("GPIO 3 set 0\n");
		if (val & 0x08) printf("GPIO 3 input\n");
		if ((val & 0x07) == 0x03) printf("GPIO 3 DAC2\n");
		if ((val & 0x07) == 0x02) printf("GPIO 3 ADC3\n");
		if ((val & 0x07) == 0x01) printf("GPIO 3 LED I2C\n");
		if ((val & 0x07) == 0x00) printf("GPIO 3 GPIO\n");
		break;
	}
	case MFR: {
		val = response_[ix++];
		if (val != 0x03) return false;
		for (int iy = 0; iy < len - 2; iy++) {
			val = response_[ix++];
			if (val < 0x7f && val >= 0x20) printf("%c ", val);
			else printf("x%x ", val);
		}
		printf("\n");
		break;
	}
	case PRODUCT: {
		val = response_[ix++];
		if (val != 0x03) return false;
		for (int iy = 0; iy < len - 2; iy++) {
			val = response_[ix++];
			if (val < 0x7f && val >= 0x20) printf("%c ", val);
			else printf("x%x ", val);
		}
		printf("\n");
		break;
	}
	case SERIAL: {
		val = response_[ix++];
		if (val != 0x03) return false;
		for (int iy = 0; iy < len - 2; iy++) {
			val = response_[ix++];
			if (val < 0x7f && val >= 0x20) printf("%c ", val);
			else printf("x%x ", val);
		}
		printf("\n");
		break;
	}
	case F_SERIAL: {
		ix++; // Byte 3 - don't care
		for (int iy = 0; iy < len; iy++) {
			val = response_[ix++];
			printf("%c", val);
		}
		printf("\n");
	}
	}
	return true;
}

// generate the SRAM write to configure GPIO as 4xGPIO pins
void gpio::gen_w_sram_cmd() {
	int ix = 0;
	command_[ix++] = 0x01;
	command_[ix++] = 0x60; // Write SRAM data
	command_[ix++] = 0x00; // Don't care
	command_[ix++] = 0x00; // Leave DAC unchanged
	command_[ix++] = 0x00; // ditto
	command_[ix++] = 0x00; // Leave ADC unchanged
	command_[ix++] = 0x00; // Leave interrupt unchanged
	command_[ix++] = 0x01; // Change GPIO
	unsigned char val = 0x00;
	for (int p = 0; p < NUM_GPIO; p++) {
		val = 0x00;
		if (pin_state_[p].direction == OUTPUT) {
			val |= 0x80;
			if (pin_state_[p].value) val |= 0x10;
		}
		command_[ix++] = val;
	}
}

bool gpio::dcd_w_sram_rsp() {
	int ix = 0;
	if (response_[ix++] != 0x60) return false;
	if (response_[ix++] != 0x00) return false;
	return true;
}

bool gpio::write_sram() {
	int result = 0;
	bool res = true;
	memset(command_, 0x00, 65);
	memset(response_, 0x00, 65);
	gen_w_sram_cmd();
	result = hid_write(device_, command_, 65);
	if (result < 0) {
		// Bad write
		printf("Unexpected error from HID_write - %ls\n", hid_error(device_));
		res = false;
	}
	else {
		result = 0;
		while (result == 0) {
			// Assume it's non-blocking
			result = hid_read(device_, response_, 65);
		}
		if (result < 0) {
			// Bad write
			printf("Unexpected error from HID_read - %ls\n", hid_error(device_));
			res = false;
		}
		else {
			if (result != 64 || !dcd_w_sram_rsp()) {
				printf("Incorrect status response\n");
				res = false;
			}
		}
		trace_cr();
	}
	return res;

}

void gpio::gen_r_sram_cmd() {
	int ix = 0;
	command_[ix++] = 0x01;
	command_[ix++] = 0x61;
}

bool gpio::dcd_r_sram_rsp() {
	int ix = 0;
	unsigned char val;
	if (response_[ix++] != 0x61) return false;
	if (response_[ix++] != 0x00) return false;
	// Skip non-GPIO SRAM fields
	ix = 22;
	val = response_[ix++];
	// Byte 4 - GPIO power up state
	if (val & 0x10) printf("GPIO 0 set 1\n");
	else printf("GPIO 0 set 0\n");
	if (val & 0x08) printf("GPIO 0 input\n");
	if ((val & 0x07) == 0x02) printf("GPIO 0 LED UART RX\n");
	if ((val & 0x07) == 0x01) printf("GPIO 0 SSPND\n");
	if ((val & 0x07) == 0x00) printf("GPIO 0 GPIO\n");
	val = response_[ix++];
	// Byte 5 - GPIO power up state
	if (val & 0x10) printf("GPIO 1 set 1\n");
	else printf("GPIO 1 set 0\n");
	if (val & 0x08) printf("GPIO 1 input\n");
	if ((val & 0x07) == 0x04) printf("GPIO 1 Interrupt detection\n");
	if ((val & 0x07) == 0x03) printf("GPIO 1 LED UART TX\n");
	if ((val & 0x07) == 0x02) printf("GPIO 1 ADC1\n");
	if ((val & 0x07) == 0x01) printf("GPIO 1 Clock\n");
	if ((val & 0x07) == 0x00) printf("GPIO 1 GPIO\n");
	val = response_[ix++];
	// Byte 4 - GPIO power up state
	if (val & 0x10) printf("GPIO 2 set 1\n");
	else printf("GPIO 2 set 0\n");
	if (val & 0x08) printf("GPIO 2 input\n");
	if ((val & 0x07) == 0x03) printf("GPIO 2 DAC1\n");
	if ((val & 0x07) == 0x02) printf("GPIO 2 ADC2\n");
	if ((val & 0x07) == 0x01) printf("GPIO 2 USB\n");
	if ((val & 0x07) == 0x00) printf("GPIO 2 GPIO\n");
	val = response_[ix++];
	// Byte 5 - GPIO power up state
	if (val & 0x10) printf("GPIO 3 set 1\n");
	else printf("GPIO 3 set 0\n");
	if (val & 0x08) printf("GPIO 3 input\n");
	if ((val & 0x07) == 0x03) printf("GPIO 3 DAC2\n");
	if ((val & 0x07) == 0x02) printf("GPIO 3 ADC3\n");
	if ((val & 0x07) == 0x01) printf("GPIO 3 LED I2C\n");
	if ((val & 0x07) == 0x00) printf("GPIO 3 GPIO\n");
	return true;
}

bool gpio::read_sram() {
	int result = 0;
	bool res = true;
	memset(command_, 0x00, 65);
	memset(response_, 0x00, 65);
	gen_r_sram_cmd();
	result = hid_write(device_, command_, 65);
	if (result < 0) {
		// Bad write
		printf("Unexpected error from HID_write - %ls\n", hid_error(device_));
		res = false;
	}
	else {
		result = 0;
		while (result == 0) {
			// Assume it's non-blocking
			result = hid_read(device_, response_, 65);
		}
		if (result < 0) {
			// Bad write
			printf("Unexpected error from HID_read - %ls\n", hid_error(device_));
			res = false;
		}
		else {
			if (result != 64 || !dcd_r_sram_rsp()) {
				printf("Incorrect status response\n");
				res = false;
			}
		}
		trace_cr();
	}
	return res;
}