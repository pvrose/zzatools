#pragma once

#include <vector>
#include <string>
#include <thread>
#include <atomic>

using namespace std;

enum key_state : char {
    NEITHER,
    LEFT,
    RIGHT,
    BOTH
};

class gpio;

class key_handler
{
public:
    key_handler();
    ~key_handler();

    // Get the list of devices
    vector<string>* get_devices();
    // Open the nth device
    bool open_device(int index);

    // Close the device
    bool close_device();

    // Set the LED out
    void drive_led(bool b);

    // Set/get reversed
    void reversed(bool b);
    bool reversed();

    // Get the pin state
    key_state get_state();

protected:

    // Thread to run
    static void run_thread(key_handler* that);
    // And non-static version
    void run_key();
    
    // The thread
    thread* t_key_;
    // Closing
    atomic<bool> close_;

    // Current key_state
    key_state current_keys_;

    // Reversed mode: GPIO0 - RIGHT, GPIO1 - LEFT
    bool reversed_;

    // GPIO driver
    gpio* gpio_;

};

