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
    // Get readings by icrementing in app
    bool read_datai(sark_data* data, bool raw, bool add_sign);
     
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
    // Command/response 
    enum command_t {
        CMD_NONE,   // No command pending
        CMD_ON,     // Start DSP and monitor 
        CMD_OFF,    // Stop DSP
        CMD_FREQ,   // Set frequency
        CMD_IMP,    // Return SWR, R, X and Z values
        CMD_RAW,    // Return unprocessed bridge voltages
        CMD_SCAN,   // Scan frequency range return SWR etc
        CMD_SCANR   // Scan frequency range return voltages
    };
    enum resp_state {
        RESP_FLUSH,     // Ignore everything until '>>' or timeout
        RESP_OK_NG,     // Expecting 'OK' or 'Error....'
        RESP_IDATA,     // Expecting data (impedance values)
        RESP_RDATA,     // Expecting data (raw voltage readings)
        RESP_RDATA_ABS, // Expecting data (raw voltage readings - don't guess sign)
        RESP_START,     // Expecting 'Start'
        RESP_END,       // Expecting 'End'
        RESP_PROMPT,    // Expecting '>>'
    };

    // File descriptor for serial port
    int serial_port_;

    // Name of serial port /dev/ttyUSBx
    string device_;

    char buffer_[256];

    sark_data* data_;

    // 

    void write_cmd(const char* cmd);

    // bool check_response();
    // bool check_prompt();

    bool process_response(resp_state state);
};