#include "drawing.h"
#include "fllog_emul.h"
#include "wsjtx_handler.h"
#include "status.h"
#include "qso_server.h"
#include "qso_manager.h"
#include "qso_rig.h"
#include "utils.h"
#include "modems.h"

using namespace std;

extern fllog_emul* fllog_emul_;
extern wsjtx_handler* wsjtx_handler_;
extern status* status_;

// Constructor for one set of modem controls
server_grp::server_grp(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
}

// Destructor
server_grp::~server_grp() {

}

// Load settings
void server_grp::load_values() {
    // Null
}

// Save settings
void server_grp::save_values() {
    // Null
}

// Create fthe widgets
void server_grp::create_form() {
    int curr_x = x();
    int curr_y = y();

    box(FL_BORDER_FRAME);

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

     resizable(nullptr);

    w(curr_x + WBUTTON - x());
    h(curr_y + HBUTTON - y());

    end();
}

// Configure the widgets
void server_grp::enable_widgets() {
    bool listening = false;
    bool hearing = false;
    // Read the appropriate modem states
    switch(modem()) {
        case FLDIGI:
        {
            listening = fllog_emul_ && fllog_emul_->has_server();
            hearing = fllog_emul_ && fllog_emul_->has_data();
            break;
        }
        case WSJTX:
        {
            listening = wsjtx_handler_ && wsjtx_handler_->has_server();
            hearing = wsjtx_handler_ && wsjtx_handler_->has_data();
            break;
        }
    }
    if (listening) {
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
qso_server::qso_server(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    load_values();
    create_form();
    enable_widgets();
}

// Destructor - save settings
qso_server::~qso_server() {
    fldigi_->save_values();
    wsjtx_->save_values();
}

// Load settings
void qso_server::load_values() {
    fldigi_->load_values();
    wsjtx_->load_values();
}

// Create the tab form
void qso_server::create_form() {
    int curr_x = x() + GAP + WBUTTON;
    int curr_y = y() + GAP;

    // Set the label as the rig. The appropriate command to launch the modem
    // app will be used
    rig_ = new Fl_Box(curr_x, curr_y, WBUTTON, HBUTTON);
    rig_->box(FL_FLAT_BOX);
    rig_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

    curr_y += HBUTTON;

    // fldigi Controls
    fldigi_ = new server_grp(curr_x, curr_y, 100, 100);
    fldigi_->copy_label(MODEM_NAMES[FLDIGI].c_str());
    fldigi_->align(FL_ALIGN_LEFT | FL_ALIGN_CENTER);
    fldigi_->modem(FLDIGI);
    curr_y += fldigi_->h();

    // WSJT-X Controls
    wsjtx_ = new server_grp(curr_x, curr_y, 100, 200);
    wsjtx_->copy_label(MODEM_NAMES[WSJTX].c_str());
    wsjtx_->align(FL_ALIGN_LEFT | FL_ALIGN_CENTER);
    wsjtx_->modem(WSJTX);
    curr_y += wsjtx_->h();

    resizable(nullptr);
    curr_x += fldigi_->w() + GAP;
    w(curr_x - x());
    h(curr_y - y());
    end();
}

// save settings
void qso_server::save_values() {
    fldigi_->save_values();
    wsjtx_->save_values();
}

// Configure widgets
void qso_server::enable_widgets() {
    // Set the current 
    qso_manager* mgr = ancestor_view<qso_manager>(this);
    string name = mgr->rig_control()->label();
    rig_->copy_label(name.c_str());
    fldigi_->enable_widgets();
    wsjtx_->enable_widgets();
}

// Listen button callback (per modem)
// v is not used
void server_grp::cb_bn_listen(Fl_Widget* w, void* v) {
    server_grp* that = ancestor_view<server_grp>(w);
    modem_t server = that->modem();
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
void server_grp::cb_bn_connect(Fl_Widget* w, void* v) {
    server_grp* that = ancestor_view<server_grp>(w);
    modem_t modem = that->modem();
    qso_manager* mgr = ancestor_view<qso_manager>(that);
    string app = "bash -i -c " + mgr->rig_control()->app(modem);
    char msg[128];
    snprintf(msg, sizeof(msg), "DASH: Starting app: %s", app.c_str());
    status_->misc_status(ST_NOTE, msg);
    system(app.c_str());
    that->enable_widgets();
  
}

// Set modem
void server_grp::modem(modem_t t) {
    modem_ = t;
}

// Get modem
modem_t server_grp::modem() {
    return modem_;
}
