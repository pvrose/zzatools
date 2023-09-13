#include "sark_control.h"
#include "sark_drawing.h"
#include "sark_data.h"
#include "sark_handler.h"
#include "sark_window.h"

#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl.H>

extern const char* PROGRAM_ID;
extern const char* VENDOR;

const static int64_t STEPS[] = {
    1000, 2000, 5000, 10000, 20000, 50000,
    100000, 200000, 500000, 1000000 
};
const static double SWR_LIMITS[] = {
    1.0, 5.0, 10.0, 20.0, 50.0
};
const static double OHM_LIMITS[] = {
    0.0, 100.0, 200.0, 500.0, 1000.0,
    2000.0, 5000.0, 1.0E4 
};
struct preset_t {
    const char* name;
    int64_t start;
    int64_t end;
    int64_t step;
} PRESETS[] = {
    { "Custom", 0, 0, 0 },
    { "All", 1000000, 60000000, 1000000 },
    { "160 m", 1700000, 2100000, 5000 },
    { "80 m", 3200000, 4000000, 10000 },
    { "60 m", 5100000, 5600000, 5000 },
    { "40 m", 6900000, 7300000, 5000 },
    { "30 m", 10000000, 10200000, 2000 },
    { "20 m", 13800000, 14600000, 10000 },
    { "17 m", 18000000, 18250000, 2000 },
    { "15 m", 20700000, 21700000, 10000 },
    { "12 m", 24700000, 25200000, 5000 },
    { "10 m", 27000000, 31000000, 20000 },
    { "HF (3-30)", 3000000, 30000000, 100000 }
};

sark_control::sark_control(int X, int Y, int W, int H, const char* L) 
    : Fl_Group(X, Y, W, H, L)
    , settings_(nullptr)
    , connected_(false)
    , device_("")
    , start_(1000000)
    , end_(60000000)
    , step_(500000)
    , data_valid_(false)
    , scan_progress_(0.0F)
    , max_swr_level_(25.0)
    , max_ohm_level_(1.0E4)
    , swr_colour_(FL_BLUE)
    , x_colour_(FL_RED)
    , r_colour_(FL_GREEN)
    , z_colour_(FL_BLACK) 
{
    load_settings();
    create();
    Fl::add_timeout(1.0, cb_ticker, this);
}

sark_control::~sark_control()
{
    Fl::remove_timeout(cb_ticker);
    save_settings();
}

void sark_control::load_settings() {
    if (settings_ == nullptr) {
        settings_ = new Fl_Preferences(Fl_Preferences::USER, VENDOR, PROGRAM_ID);
    }
    double d_temp;
    int i_temp;
    // Read all the settings
    char* c_temp;
    settings_->get("Device", c_temp, "");
    device_ = c_temp;
    printf("Device = %s\n", device_.c_str());
    settings_->get("Band", c_temp, "Custom");
    band_ = c_temp;
    settings_->get("Start Frequency", d_temp, 1E6);
    start_ = (int64_t)d_temp;
    settings_->get("End Frequency", d_temp, 60E6);
    end_ = (int64_t)d_temp;
    settings_->get("Frequency Step", d_temp, 500E3);
    step_ = (int64_t)d_temp;
    settings_->get("Maximum SWR Display", max_swr_level_, 25.0);
    settings_->get("Maximum Ohms Display", max_ohm_level_, 1.0E4);
    settings_->get("Colour SWR", (int&)swr_colour_, FL_BLUE);
    settings_->get("Colour Reactance", (int&)x_colour_, FL_RED);
    settings_->get("Colour Resistance", (int&)r_colour_, FL_GREEN);
    settings_->get("Colour Impedance", (int&)z_colour_, FL_BLACK);
}

void sark_control::save_settings() {
    // Store all the settings
    settings_->set("Device", device_.c_str());
    settings_->set("Band", band_.c_str());
    settings_->set("Start Frequency", (double)start_);
    settings_->set("End Frequency", (double)end_);
    settings_->set("Frequency Step", (double)step_);
    settings_->set("Maximum SWR Display", max_swr_level_);
    settings_->set("Maximum Ohms Display", max_ohm_level_);
    settings_->set("Colour SWR", (int)swr_colour_);
    settings_->set("Colour Reactance", (int)x_colour_);
    settings_->set("Colour Resistance", (int)r_colour_);
    settings_->set("Colour Impedance", (int)z_colour_);
    settings_->flush();
}

void sark_control::create() {
    begin();

    int curr_x = x() + GAP;
    int curr_y = y() + GAP;

    // Device connect and choice
    bn_connect_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
    bn_connect_->value(connected_);
    bn_connect_->callback(cb_bn_connect, &connected_);
    bn_connect_->tooltip("Connect to the SARK device");
    curr_x += WBUTTON;
    ch_device_ = new Fl_Choice(curr_x, curr_y, WSSEDIT, HBUTTON);
    ch_device_->callback(cb_ch_device, &device_);
    ch_device_->tooltip("Select the SARK device");
    populate_device();

    // Presets,
    curr_x = x() + GAP + WBUTTON;
    curr_y += HBUTTON + GAP;
    ch_preset_ = new Fl_Choice(curr_x, curr_y, WSSEDIT, HBUTTON, "Presets");
    ch_preset_->align(FL_ALIGN_LEFT);
    ch_preset_->callback(cb_ch_preset);
    ch_preset_->tooltip("Select a pre-programmed range - band \302\26150%");
    populate_preset();

    // Start, end and step
    curr_x = x() + GAP + WBUTTON;
    curr_y += HBUTTON + GAP;
    vl_start_ = new Fl_Spinner(curr_x, curr_y, WSSEDIT, HBUTTON, "Start (Hz)");
    vl_start_->value(start_);
    vl_start_->callback(cb_vl, &start_);
    vl_start_->type(FL_INT_INPUT);
    vl_start_->range(1000000, 60000000);
    vl_start_->step(step_);
    vl_start_->tooltip("Enter the scan start frequency (in Hz)");
    curr_y += HBUTTON;
    vl_end_ = new Fl_Spinner(curr_x, curr_y, WSSEDIT, HBUTTON, "End (Hz)");
    vl_end_->value(end_);
    vl_end_->callback(cb_vl, &end_);
    vl_end_->type(FL_INT_INPUT);
    vl_end_->range(1000000, 60000000);
    vl_end_->step(step_);
    vl_end_->tooltip("Enter the scan stop frequency (in Hz)");
    curr_y += HBUTTON;
    ch_step_ = new Fl_Choice(curr_x, curr_y, WSSEDIT, HBUTTON, "Step");
    ch_step_->callback(cb_ch_step, &step_);
    ch_step_->tooltip("Select the scan frequency step");
    curr_y += HBUTTON;
    op_samples_ = new Fl_Output(curr_x, curr_y, WSSEDIT, HBUTTON, "# samples");
    op_samples_->tooltip("Number of samples");
    populate_step();
    update_scan_params();

    // Do scan and progress bar
    curr_x = x() + GAP;
    curr_y += HBUTTON + GAP;
    bn_scan_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Scan");
    bn_scan_->value(false);
    bn_scan_->callback(cb_bn_scan);
    bn_scan_->tooltip("Initiate scan");
    curr_x += WBUTTON;
    pr_scan_ = new Fl_Progress(curr_x, curr_y, WSSEDIT, HBUTTON);
    pr_scan_->minimum(0.0F);
    pr_scan_->maximum(1.0F);
    pr_scan_->value(0.0F);
    pr_scan_->color(FL_BACKGROUND_COLOR, FL_GREEN);
    pr_scan_->tooltip("Displays the progress reading dta from SARK");

    // max display limits for SWR and Ohms
    curr_x = x() + GAP + WBUTTON;
    curr_y += HBUTTON + GAP;
    ch_max_swr_ = new Fl_Choice(curr_x, curr_y, WSSEDIT, HBUTTON, "Max. SWR");
    ch_max_swr_->callback(cb_ch_max_swr, &max_swr_level_);
    ch_max_swr_->tooltip("Select the maximum SWR to be displayed");
    populate_max_swr();
    curr_y += HBUTTON;
    ch_max_ohm_ = new Fl_Choice(curr_x, curr_y, WSSEDIT, HBUTTON, "Max. \316\251");
    ch_max_ohm_->callback(cb_ch_max_ohm, &max_swr_level_);
    ch_max_ohm_->tooltip("Select the maximum \316\251 value to be displayed");
    populate_max_ohm();

    // Colour buttons
    curr_y += HBUTTON + GAP;
    const int WIDTH = WBUTTON + WSSEDIT;
    const int C1 = WIDTH / 2 - HBUTTON;
    const int C2 = WIDTH - HBUTTON;
    bn_colour_swr_ = new Fl_Button(C1, curr_y, HBUTTON, HBUTTON, "SWR");
    bn_colour_swr_->align(FL_ALIGN_LEFT);
    bn_colour_swr_->callback(cb_bn_colour, &swr_colour_);
    bn_colour_swr_->tooltip("Select colour for SWR line");
    bn_colour_swr_->color(swr_colour_);
    bn_colour_z_ = new Fl_Button(C2, curr_y, HBUTTON, HBUTTON, "Z");
    bn_colour_z_->align(FL_ALIGN_LEFT);
    bn_colour_z_->callback(cb_bn_colour, &z_colour_);
    bn_colour_z_->tooltip("Select colour for impedance line");
    bn_colour_z_->color(z_colour_);
    curr_y += HBUTTON;
    bn_colour_x_ = new Fl_Button(C1, curr_y, HBUTTON, HBUTTON, "X");
    bn_colour_x_->align(FL_ALIGN_LEFT);
    bn_colour_x_->callback(cb_bn_colour, &x_colour_);
    bn_colour_x_->tooltip("Select colour for reactance line");
    bn_colour_x_->color(x_colour_);
    bn_colour_r_ = new Fl_Button(C2, curr_y, HBUTTON, HBUTTON, "R");
    bn_colour_r_->align(FL_ALIGN_LEFT);
    bn_colour_r_->callback(cb_bn_colour, &r_colour_);
    bn_colour_r_->tooltip("Select colour for resistance line");
    bn_colour_r_->color(r_colour_);

    // Now finalise the size of the group
    curr_y += HBUTTON + GAP;
    curr_x = x() + GAP + WIDTH;
    end();
    resizable(nullptr);
    size(curr_x - x(), curr_y - y());
    show();

}

// Populate the device choice with connected serial ports
void sark_control::populate_device() {
    // Get available ports
 	const unsigned int MAX_TTY = 255;
	struct stat st;
	char ttyname[PATH_MAX + 1];
   	const char* tty_fmt[] = {
//		"/dev/tty%u",
		"/dev/ttyS%u",
		"/dev/ttyUSB%u" //,
//		"/dev/usb/ttyUSB%u"
	};
    size_t sel = 0;
	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		for (unsigned j = 0; j < MAX_TTY; j++) {
			snprintf(ttyname, sizeof(ttyname), tty_fmt[i], j);
			if ( !(stat(ttyname, &st) == 0 && S_ISCHR(st.st_mode)) )
				continue;
            // Add tty name to devices
            ch_device_->add(ttyname);
            if (!strcmp(ttyname, device_.c_str())) {
                ch_device_->value(sel);
            }
            sel++;
		}
	}
}

// Populate the pre-programmed band choic
void sark_control::populate_preset() {
    size_t sz = sizeof(PRESETS) / sizeof(PRESETS[0]);
    size_t sel = 0;
    ch_preset_->clear();
    for (size_t ix = 0; ix < sz; ix++) {
        ch_preset_->add(PRESETS[ix].name);
        if (band_ == PRESETS[ix].name) {
            sel = ix;
        }
    }
    ch_preset_->value(sel);
}

// Populate the step choice with fixed values
void sark_control::populate_step() {
    size_t sz = sizeof(STEPS) / sizeof(int64_t);
    char text[10];
    size_t sel = sz - 1;
    ch_step_->clear();
    for (size_t ix = 0; ix < sz; ix++) {
        if (STEPS[ix] >= 1000000) {
            snprintf(text, sizeof(text), "%d MHz", STEPS[ix]/1000000);
        } else if (STEPS[ix] >= 1000) {
            snprintf(text, sizeof(text), "%d kHz", STEPS[ix]/1000);
        } else {
            snprintf(text, sizeof(text), "%d Hz", STEPS[ix]);
        }
        ch_step_->add(text);
        if (step_ == STEPS[ix]) {
            sel = ix;
        }
    }
    ch_step_->value(sel);
}

// Populate the maxmum displayed SWR
void sark_control::populate_max_swr() {
    size_t sz = sizeof(SWR_LIMITS) / sizeof(double);
    char text[10];
    size_t sel = sz - 1;
    ch_max_swr_->clear();
    for (size_t ix = 0; ix < sz; ix++) {
        if (SWR_LIMITS[ix] == 1.0) {
            strcpy(text, "Auto");
        } else {
            snprintf(text, sizeof(text), "%.0f:1", SWR_LIMITS[ix]);
        }
        ch_max_swr_->add(text);
        if (SWR_LIMITS[ix] == max_swr_level_) {
            sel = ix;
        }
    }
    ch_max_swr_->value(sel);
}

// Populate the maximum displayed ohm value
void sark_control::populate_max_ohm() {
    size_t sz = sizeof(OHM_LIMITS) / sizeof(double);
    char text[10];
    size_t sel = sz -1;
    ch_max_ohm_->clear();
    for (size_t ix = 0; ix < sz; ix++) {
        if (OHM_LIMITS[ix] == 0.0) {
            strcpy(text, "Auto");
        }
        else if (OHM_LIMITS[ix] >= 1.0E3) {
            snprintf(text, sizeof(text), "%.0f k\316\251", OHM_LIMITS[ix]/ 1.0E3);
        } else {
            snprintf(text, sizeof(text), "%.0f \316\251", OHM_LIMITS[ix]);
        }
        ch_max_ohm_->add(text);
        if (OHM_LIMITS[ix] == max_ohm_level_) {
            sel = ix;
        }
    }
    ch_max_ohm_->value(sel);
}

// Callbacks

// Connect button
void sark_control::cb_bn_connect(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    sark_window* win = (sark_window*)w->top_window();
    Fl_Light_Button* bn = (Fl_Light_Button*)w;
    bool go = bn->value();
    if (go && win->handler_ || !go && !win->handler_) {
        printf("ERRPOR: Connect button state at odds with device state\n");
    } else if (go) {
        win->handler_ = new sark_handler(that->device_.c_str());
    } else {
        delete win->handler_;
        win->handler_ = nullptr;
    }
    *((bool*)v) = go;
    that->update_device();
    that->update_scan_progress();
}

// Device choice - 
void sark_control::cb_ch_device(Fl_Widget* w, void* v) {
    Fl_Choice* ch = (Fl_Choice*)w;
    *(string*)v = ch->text();
}

// Band preset
void sark_control::cb_ch_preset(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    Fl_Choice* ch = (Fl_Choice*)w;
    int ix = ch->value();
    that->band_ = ch->text();
    if (that->band_ != "Custom") {
        that->start_ = PRESETS[ix].start;
        that->end_ = PRESETS[ix].end;
        that->step_ = PRESETS[ix].step;
        that->update_scan_params();
    }
}

// Start and stop values
void sark_control::cb_vl(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    Fl_Spinner* vl = (Fl_Spinner*)w;
    *(int64_t*)v = (int64_t)vl->value();
    // We have changed from preset value
    that->band_ = "Custom";
    that->update_scan_params();
}

// Scan value
void sark_control::cb_ch_step(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    Fl_Choice* ch = (Fl_Choice*)w;
    int ix = ch->value();
    *(int64_t*)v = STEPS[ix];
    that->update_scan_params();
}

// Initiate scan
void sark_control::cb_bn_scan(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    that->clear_sark_graph();
    that->scan_sark();
    that->update_sark_graph();
}

// Max SWR
void sark_control::cb_ch_max_swr(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    Fl_Choice* ch = (Fl_Choice*)w;
    int ix = ch->value();
    *(double*)v = SWR_LIMITS[ix];
    that->update_sark_graph();
}

// Max ohm value
void sark_control::cb_ch_max_ohm(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    Fl_Choice* ch = (Fl_Choice*)w;
    int ix = ch->value();
    *(double*)v = OHM_LIMITS[ix];
    that->update_sark_graph();
}

// Colour buttons - open colour chooser
void sark_control::cb_bn_colour(Fl_Widget* w, void* v) {
    sark_control* that = (sark_control*)w->parent();
    Fl_Button* bn = (Fl_Button*)w;
    Fl_Color* colour = (Fl_Color*)v;
    uchar r, g, b;
    Fl::get_color(*colour, r, g, b);
    fl_color_chooser("Select colour", r, g, b, -1);
    *colour = fl_rgb_color(r, g, b);
    bn->color(*colour);
    that->update_sark_graph();
}

// Ticker
void sark_control::cb_ticker(void* v) {
    sark_control* that = (sark_control*)v;
    that->update_scan_progress();
    Fl::repeat_timeout(1.0, cb_ticker, v);
}

// Disable the device selector if connected
void sark_control::update_device() {
    if (connected_) {
        ch_device_->deactivate();
    } else {
        ch_device_->activate();
    }
}

// Update scan [progress] - disable scan button when scanning
// update progress bar
void sark_control::update_scan_progress() {
    sark_window* win = (sark_window*)parent();
    if (win->handler_) {
        if (win->data_) {
            size_t max = win->data_->size();
            size_t current = win->data_->index();
            double p = (double)current / (double)max;
            pr_scan_->activate();
            pr_scan_->value(p);
            if (current == 0) {
                bn_scan_->activate();
                bn_scan_->value(false);
            } else if (current == max) {
                bn_scan_->activate();
                bn_scan_->value(true);
            } else {
                bn_scan_->deactivate();
            }
        } else {
            pr_scan_->activate();
            pr_scan_->value(0.0);
            bn_scan_->activate();
        }
    } else {
        pr_scan_->deactivate();
        bn_scan_->deactivate();
    }
}

// Update scan parametrs - set step value in spinners
void sark_control::update_scan_params() {
    size_t sz = sizeof(STEPS) / sizeof(int64_t);
    for (int ix = 0; ix < sz; ix++) {
        if (step_ == STEPS[ix]) {
            ch_step_->value(ix);
        }
    }
    int ix = ch_preset_->find_index(band_.c_str());
    ch_preset_->value(ix);
    // Adjust start value to be a muliplte of step <= its current value
    start_ = start_ - (start_ % step_);
    // Adjust end value to be a multiple of step >= current value
    if (end_ % step_ != 0) {
        end_ = end_ + step_ - (end_ % step_);
    }
    vl_end_->value(end_);
    vl_end_->step((double)step_);
    vl_end_->minimum(vl_start_->value());
    vl_start_->value(start_);
    vl_start_->step((double)step_);
    vl_start_->maximum(vl_end_->value());
    int num_samples = (end_ - start_) / step_;
    char text[16];
    snprintf(text, sizeof(text), "%d", num_samples);
    op_samples_->value(text);
}

// Start the scan
void sark_control::scan_sark() {
    sark_window* win = (sark_window*)parent();
    sark_data::scan_params parms = { start_, end_, step_ };
    pr_scan_->value(0.0);
    redraw();
    win->data_ = new sark_data(parms);
    win->handler_->read_data(win->data_);
}

// Send the data to the sark_graph
void sark_control::update_sark_graph() {
    sark_window* win = (sark_window*)parent();
    win->graph_->colours(swr_colour_, x_colour_, r_colour_, z_colour_);
    win->graph_->swr_bounds(SWR_LIMITS[ch_max_swr_->value()]);
    win->graph_->ohm_bounds(OHM_LIMITS[ch_max_ohm_->value()]);
    win->graph_->MHz_bounds((double)end_/1000000, (double)start_/1000000);
    win->graph_->data(win->data_);

}

// Clear the sark_graph
void sark_control::clear_sark_graph() {
    sark_window* win = (sark_window*)parent();
    win->graph_->clear();
}
