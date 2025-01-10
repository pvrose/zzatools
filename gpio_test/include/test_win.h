#pragma once

#include "gpio.h"

#include <FL/Fl_Window.H>

class Fl_Choice;
class Fl_Radio_Round_Button;
class Fl_Light_Button;
class Fl_Button;
class Fl_Group;

class test_win :
    public Fl_Window
{
public:
    test_win();
    ~test_win();

    void create_form();
    void enable_widgets();

    // Callbacks
    // choice select device to connect to
    static void cb_devices(Fl_Widget* w, void* v);
    // radio button selection - v is pin number, how do we pass whether in or out (widget label)
    static void cb_inout(Fl_Widget* w, void* v);
    // output pin drive - v is pin number
    static void cb_op_drive(Fl_Widget* w, void* v);
    // Timer callback
    static void cb_timer(void* v);
    // Rescan
    static void cb_rescan(Fl_Widget* w, void* v);

protected:

    void populate_devices(Fl_Choice* ch);
    // Update gpio
    void update_gpio(int pin);
    // Read pin values
    void read_pin_values();

    // All the discovered HID devices
    // The selected HID device
    int device_num_;
    vector<string>* devices_;

    gpio* gpio_driver_;

    // The pic configurations
    bool is_output_[gpio::NUM_GPIO];
    // The pin valuse
    bool pin_value_[gpio::NUM_GPIO];

    // The widgets
    Fl_Choice* ch_device_;
    Fl_Group* g_pin_[gpio::NUM_GPIO];
    Fl_Radio_Round_Button* rb_in_[gpio::NUM_GPIO];
    Fl_Radio_Round_Button* rb_out_[gpio::NUM_GPIO];
    Fl_Light_Button* lb_value_[gpio::NUM_GPIO];
    Fl_Button* bn_rescan_;

};

