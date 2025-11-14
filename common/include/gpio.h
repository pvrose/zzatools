#pragma once

#include "hidapi/hidapi.h"
#include <atomic>
#include <cwchar>
#include <thread>
#include <vector>
#include <string>



// Class to support the GPIO functionality of the MCP2221A chip
class gpio
{
public:
	gpio();
	~gpio();

	// VID and PID will probably always be 0x04d8, and 0x00dd and ser_no nullptr
	bool open(unsigned short vid, unsigned short pid, const wchar_t* ser_no);
	// Open by index
	bool open(int ix);

	// Close current HID
	bool close();

	// The direction the GPIO pin will take
	enum direction_t : char {
		OUTPUT = 0,
		INPUT = 1,
		NOT_GPIO = 2,
	};
	// The pin number - the enum is commentary
	enum pin_no_t : unsigned char {
		NO_PINS = 0,
		GPIO_0 = 1,
		GPIO_1 = 2,
		GPIO_2 = 4,
		GPIO_3 = 8,
	};
	static const int NUM_GPIO = 4;

	// Get direction
	direction_t direction(pin_no_t pin);
	// Set direction
	bool direction(pin_no_t pin, direction_t d);
	// Get value
	bool value(pin_no_t p);
	// Set value
	bool value(pin_no_t p, bool v);
	// Get values - all pins
	unsigned char values();
	// Get list of devices
	vector<string>* get_devices(unsigned short vid, unsigned short pid);

protected:

	enum write_t : unsigned char {
		NONE = 0,
		DIRECTION = 1,
		VALUE = 2,
		BOTH = 3
	};
	// Read GPIO status
	enum status_t : unsigned char {
		CHIP = 0,
		GPIO = 1,
		MFR = 2,
		PRODUCT = 3,
		SERIAL = 4,
		F_SERIAL = 5
	};
	// Generate the write data from the pin_states
	void gen_w_dir_cmd();
	void gen_w_val_cmd();
	void gen_read_cmd();
	void gen_w_sram_cmd();
	void gen_r_sram_cmd();
	// Decode the responses from the commands
	bool dcd_write_rsp();
	bool dcd_read_rsp();
	bool dcd_status_rsp(status_t type);
	bool dcd_w_sram_rsp();
	bool dcd_r_sram_rsp();
	// gen status command
	void gen_status_cmd(status_t type);

	// write state to GPIO
	bool write_gpio(write_t type);
	bool write_sram();
	// read state from GPIO
	bool read_gpio(write_t type);
	bool read_status(status_t type);
	bool read_sram();


	// Display command and response
	void trace_cr();

	// Write GPIO direction or 
	struct pin_state {
		direction_t direction;
		bool value;
	};
	pin_state pin_state_[NUM_GPIO];
	// pin values have change
	pin_no_t changed_pins_;

	// The device
	hid_device* device_;
	// Linked List of possible devices
	hid_device_info* device_list_;

	wchar_t manufacturer_[256];
	wchar_t product_[256];
	wchar_t serial_no_[256];

	// Command and response buffers
	unsigned char command_[65];
	unsigned char response_[65];


};

