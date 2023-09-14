#pragma once

#include "sark_data.h"

#include <string>

using namespace std;

class sark_handler {

public:

    sark_handler(const char* device);
    ~sark_handler();

    // Get the readings
    bool read_data(sark_data* data, bool raw, bool add_sign);
 
protected:
    // SARK command set
    const char* cmd_on = "on\r\n";
    const char* cmd_off = "off\r\n";
    const char* cmd_freq = "freq %d\r\n"; // Frequency
    const char* cmd_imp = "imp\r\n";
    const char* cmd_raw = "raw\r\n";
    const char* cmd_scan = "scan %d %d %d\r\n"; // Start stop step
    const char* cmd_scanr = "scanr %d %d %d\r\n"; // Start stop step
    // Expected responses
    const char* resp_ok = "OK";
    const char* resp_start = "start";
    const char* resp_end = "end";

    // File descriptor for serial port
    int serial_port_;

    // Name of serial port /dev/ttyUSBx
    string device_;

    char buffer_[256];

    size_t read_line();

    void write_cmd(const char* cmd, size_t size);

    bool check_response();
    bool check_prompt();
};