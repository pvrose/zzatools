#pragma once

#include <FL/Fl_Double_Window.H>

// Forward declaration - FLTK
class Fl_Button;
class Fl_Choice;
class Fl_Float_Input;
class Fl_Group;
class Fl_Input;
class Fl_Radio_Round_Button;
class Fl_Text_Display;
class Fl_Value_Slider;

enum engine_type : char;
enum shape_t : char;

class display :
    public Fl_Double_Window
{
public:

    display(int W, int H, const char* L = nullptr);
    ~display();

    void create_form();
    void load_data();
    void enable_widgets();
    void save_data();

    // Create the individuela groups
    void create_dev_choice(int& curr_x, int& curr_y);
    void create_settings(int& curr_x, int& curr_y);
    void create_shape(int& curr_x, int& curr_y);
    void create_source(int& curr_x, int& curr_y);
    void create_monitor(int& curr_x, int& curr_y);

    // Callback - select device - update key_handler
    static void cb_device(Fl_Widget* w, void* v);
    // Callback - speed: save value, update paddle/keyboard SSMs
    static void cb_speed(Fl_Widget* w, void* v);
    // Callback - frequency: save value - update wave_gen
    static void cb_freq(Fl_Widget* w, void* v);
    // Callback - shapes: save value, enable rise/fall, update wave_gen
    static void cb_shapes(Fl_Widget* w, void* v);
    // Callback - times: save value, update wave_gen
    static void cb_times(Fl_Widget* w, void* v);
    // Callback - paddle config: save value, update paddle_sm
    static void cb_engine(Fl_Widget* w, void* v);
    // Callback - editor: save value - start sending data
    static void cb_editor(Fl_Widget* w, void* v);

    // Callback - monitor data ready
    static void cb_monitor(void* v);
    // Callback - kb sending done
    static void cb_kb_done(void* v);

protected:
    // Update paddle and keyboard SMs with speed values
    void update_speed();
    // Update wave_gen with frequency and shaping values
    void update_wavegen();
    // Update paddle with configuration
    void update_engine();
    // Update monitor with data from decoder
    void update_monitor();
    // Update editor with edit mode
    enum edit_event : char {
        NEW_CHARACTER,           // a new character has been typed
        ENTER,                   // The enter key has been sent
        SEND_DATA,               // Keyboard SM is empty send new data
    };
    void update_editor(edit_event e);
    // Enable rise and fall times
    void enable_rise_fall();
    // Enable editor
    void enable_editor();
    // Enable device choice
    void enable_devices();
    // Enable engine choice
    void enable_engine();
    // SEt settings
    
     // Populate the device choice
    void populate_devices();
    // Populate source choice
    void populate_sources();

    // Attributes
    // Target audio frequency
    double target_freq_;
    // Words per minute
    double wpm_;
    // dash/dot weighting
    double weight_;
    // Key waveform shape
    shape_t shape_;
    // Rise and fall time
    double rise_time_;
    double fall_time_;
    // Paddle handedness
    bool reversed_;
    // Engine type
    engine_type engine_type_;
    // Device number
    int device_number_;
    // Keyboard entry buffer
    char buffer_[1024];
    char* next_send_;
    char* last_typed_;

    // Widgets
    Fl_Group* g_devices_;
    Fl_Choice* ch_devices_;

    Fl_Group* g_settings_;
    Fl_Value_Slider* vs_freq_;
    Fl_Value_Slider* vs_wpm_;
    Fl_Value_Slider* vs_weight_;

    Fl_Group* g_shape_;
    Fl_Group* rg_shape_;
    Fl_Radio_Round_Button* rb_none_;
    Fl_Radio_Round_Button* rb_ramp_;
    Fl_Radio_Round_Button* rb_cosine_;
    Fl_Value_Slider* vs_rise_;
    Fl_Value_Slider* vs_fall_;

    Fl_Group* g_source_;
    Fl_Choice* ch_engine_;
    Fl_Input* ip_entry_;

    Fl_Group* g_monitor_;
    Fl_Text_Display* td_monitor_;

};

