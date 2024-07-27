#include "drawing.h"
#include "fllog_emul.h"
#include "wsjtx_handler.h"
#include "status.h"
#include "qso_apps.h"
#include "qso_manager.h"
#include "qso_rig.h"
#include "utils.h"
#include "callback.h"

#include <FL/Fl_Preferences.H>

using namespace std;

extern fllog_emul* fllog_emul_;
extern wsjtx_handler* wsjtx_handler_;
extern status* status_;
extern Fl_Preferences* settings_;

// Constructor for one set of modem controls
app_grp::app_grp(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
}

// Destructor
app_grp::~app_grp() {

}

// Load settings
void app_grp::load_values() {
    qso_manager* mgr = ancestor_view<qso_manager>(this);
    const char* rig_name = mgr->rig_control()->label();
    char sett_name[128];
    snprintf(sett_name, sizeof(sett_name), "CAT/%s/Apps", rig_name);
    Fl_Preferences app_settings(settings_, sett_name);
    char* temp;
    app_settings.get(label(), temp, "");
    app_name_ = temp;
    free(temp);
}

// Save settings
void app_grp::save_values() {
    qso_manager* mgr = ancestor_view<qso_manager>(this);
    const char* rig_name = mgr->rig_control()->label();
    char sett_name[128];
    snprintf(sett_name, sizeof(sett_name), "CAT/%s/Apps", rig_name);
    Fl_Preferences app_settings(settings_, sett_name);
    app_settings.set(label(), app_name_.c_str());
    settings_->flush();
}

// Create fthe widgets
void app_grp::create_form() {
    int curr_x = x() + WLABEL;
    int curr_y = y();

    box(FL_BORDER_BOX);

    // Input to specify the command for invoking the app
    ip_app_name_ = new Fl_Input(curr_x, curr_y, WBUTTON * 2, HBUTTON, "Command");
    ip_app_name_->callback(cb_ip_app, &app_name_);
    ip_app_name_->tooltip("Enter the command to invoke the app for the selected rig");
    ip_app_name_->when(FL_WHEN_CHANGED);

    curr_y += HBUTTON;

    // Light to say that we are listening for the modem app to connect
    // Button will switch this on or off
    bn_listening_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Listen");
    bn_listening_->selection_color(FL_GREEN);
    bn_listening_->callback(cb_bn_listen);
    bn_listening_->tooltip("Listen or block messages from this modem");

    curr_x += WBUTTON;
    // Light to say we have received a packet from the app
    // Button will launch the mode app
    bn_connect_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
    bn_connect_->callback(cb_bn_connect);
    bn_connect_->tooltip("Launch the modem app");

    curr_y += HBUTTON + GAP;

    resizable(nullptr);
    size(w(), curr_y - y());

    end();
}

// Configure the widgets
void app_grp::enable_widgets() {
    load_values();
    ip_app_name_->value(app_name_.c_str());
    bool listening = false;
    bool hearing = false;
    bool listenable = false;
    // Read the appropriate modem states
    switch(modem()) {
        case FLDIGI:
        {
            listenable = true;
            listening = fllog_emul_ && fllog_emul_->has_server();
            hearing = fllog_emul_ && fllog_emul_->has_data();
            break;
        }
        case WSJTX:
        {
            listenable = true;
            listening = wsjtx_handler_ && wsjtx_handler_->has_server();
            hearing = wsjtx_handler_ && wsjtx_handler_->has_data();
            break;
        }
    }
    if (!listenable) {
        bn_listening_->deactivate();
        bn_connect_->activate();
    }
    else if (listening) {
        // We are listening - set the light, activate the launch button
        bn_listening_->value(true);
        bn_connect_->activate();
        if (hearing) {
            // We have received a packet - set the connected light
            bn_connect_->value(true);
        } else {
            bn_connect_->value(false);
        }
    } else {
        bn_listening_->value(false);
        bn_connect_->deactivate();
    }
}

// Constructor for the tab form
qso_apps::qso_apps(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
    load_values();
    enable_widgets();
}

// Destructor - save settings
qso_apps::~qso_apps() {
}

// Load settings
void qso_apps::load_values() {
    for (int ix = 0; ix < NUMBER_APPS; ix++) apps_[ix]->load_values();
}

// Create the tab form
void qso_apps::create_form() {
    int curr_x = x() + GAP + WLABEL;
    int curr_y = y() + GAP;

    // Set the label as the rig. The appropriate command to launch the modem
    // app will be used
    rig_ = new Fl_Box(curr_x, curr_y, WBUTTON, HBUTTON);
    rig_->box(FL_FLAT_BOX);
    rig_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

    curr_y += HBUTTON + GAP;

    int avail_w = w() - GAP - GAP;
    int avail_h = h() + y() - curr_y - GAP;

    curr_x = x() + GAP;

    tabs_ = new Fl_Tabs(curr_x, curr_y, avail_w, avail_h);
    tabs_->box(FL_BORDER_BOX);
    int rx = 0;
    int ry = 0;
    int rw = 0;
    int rh = 0;
    tabs_->client_area(rx, ry, rw, rh, 0);
    int saved_rh = rh;
    int saved_rw = rw;

    for (int ix = 0; ix < NUMBER_APPS; ix++) {
        apps_[ix] = new app_grp(rx, ry, rw, rh, APP_NAMES[ix].c_str());
        apps_[ix]->modem((app_t)ix);
        apps_[ix]->labelsize(FL_NORMAL_SIZE + 2);
        rh = max(rh, apps_[ix]->h());
        rw = max(rw, apps_[ix]->w());
    }

    for (int ix = 0; ix < NUMBER_APPS; ix++) {
        apps_[ix]->size(rw, rh);
    }

    tabs_->resizable(nullptr);
    tabs_->size(avail_w + rw - saved_rw, avail_h + rh - saved_rh);
    tabs_->end();

    resizable(nullptr);
    int max_x = tabs_->x() + tabs_->w() + GAP;
    int max_y = tabs_->y() + tabs_->h() + GAP;

    size(max_x - x(), max_y - y());
    end();

    redraw();
}

// save settings
void qso_apps::save_values() {
    for (int ix = 0; ix < NUMBER_APPS; ix++) apps_[ix]->save_values();
}

// Configure widgets
void qso_apps::enable_widgets() {
    // Set the current 
    qso_manager* mgr = ancestor_view<qso_manager>(this);
    string name = mgr->rig_control()->label();
    rig_->copy_label(name.c_str());

    for (int ix = 0; ix < NUMBER_APPS; ix++) {
        app_grp* ax = apps_[ix];
        ax->enable_widgets();
        if (ax == tabs_->value()) {
            ax->labelfont((ax->labelfont() | FL_BOLD) & (~FL_ITALIC));
            ax->labelcolor(FL_FOREGROUND_COLOR);
        }
        else {
            ax->labelfont((ax->labelfont() & (~FL_BOLD)) | FL_ITALIC);
            ax->labelcolor(FL_FOREGROUND_COLOR);
        }
    }
}

// Listen button callback (per modem)
// v is not used
void app_grp::cb_bn_listen(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    app_t server = that->modem();
    switch (server) {
    case FLDIGI:
        if (fllog_emul_->has_server()) {
            fllog_emul_->close_server();
        } else {
            fllog_emul_->run_server();
        }
        break;
    case WSJTX:
        if (wsjtx_handler_->has_server()) {
            wsjtx_handler_->close_server();
        } else {
            wsjtx_handler_->run_server();
        }
        break;
    }
    that->enable_widgets();
}

// Start button callback
// v is not used
void app_grp::cb_bn_connect(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    app_t modem = that->modem();
    qso_manager* mgr = ancestor_view<qso_manager>(that);
    string app = "bash -i -c " + that->app_name_;
    char msg[128];
    snprintf(msg, sizeof(msg), "DASH: Starting app: %s", app.c_str());
    status_->misc_status(ST_NOTE, msg);
    system(app.c_str());
    that->enable_widgets();
  
}

// App typed in
void app_grp::cb_ip_app(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    cb_value<Fl_Input, string>(w, v);
    that->save_values();
}

// Set modem
void app_grp::modem(app_t t) {
    modem_ = t;
}

// Get modem
app_t app_grp::modem() {
    return modem_;
}
