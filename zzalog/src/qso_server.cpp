#include "drawing.h"
#include "fllog_emul.h"
#include "wsjtx_handler.h"
#include "status.h"
#include "qso_server.h"
#include "qso_manager.h"
#include "qso_rig.h"
#include "utils.h"

using namespace std;

extern fllog_emul* fllog_emul_;
extern wsjtx_handler* wsjtx_handler_;
extern status* status_;

server_grp::server_grp(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
}

server_grp::~server_grp() {

}

void server_grp::load_values() {
    // Null
}

void server_grp::save_values() {
    // Null
}

void server_grp::create_form() {
    int curr_x = x();
    int curr_y = y() + HTEXT;

    box(FL_BORDER_FRAME);
    align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

    bn_listening_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Listen");
    bn_listening_->selection_color(FL_GREEN);
    bn_listening_->callback(cb_bn_listen);
    bn_listening_->tooltip("Listen or block messages from this modem");

    curr_x += WBUTTON;
    bn_connect_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
    bn_connect_->callback(cb_bn_connect);
    bn_connect_->tooltip("Launch the modem app");

     resizable(nullptr);

    w(curr_x + WBUTTON - x());
    h(curr_y + HBUTTON + GAP - y());

    end();
}

void server_grp::enable_widgets() {
    bool listening = false;
    bool hearing = false;
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
        bn_listening_->value(true);
        bn_connect_->activate();
        if (hearing) {
            bn_connect_->value(true);
        } else {
            bn_connect_->value(false);
        }
    } else {
        bn_listening_->value(false);
        bn_connect_->deactivate();
    }
}


qso_server::qso_server(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    load_values();
    create_form();
    enable_widgets();
}

qso_server::~qso_server() {
    fldigi_->save_values();
    wsjtx_->save_values();
}

void qso_server::load_values() {
    fldigi_->load_values();
    wsjtx_->load_values();
}

void qso_server::create_form() {
    int curr_x = x() + GAP;
    int curr_y = y() + GAP;

    fldigi_ = new server_grp(curr_x, curr_y, 100, 100, "FLDIGI");
    fldigi_->modem(FLDIGI);
    curr_y += fldigi_->h();

    wsjtx_ = new server_grp(curr_x, curr_y, 100, 200, "WSJT-X");
    wsjtx_->modem(WSJTX);
    curr_y += wsjtx_->h();

    resizable(nullptr);
    curr_x += fldigi_->w() + GAP;
    w(curr_x - x());
    curr_y += GAP;
    h(curr_y - y());
    end();
}

void qso_server::save_values() {
    fldigi_->save_values();
    wsjtx_->save_values();
}

void qso_server::enable_widgets() {
    fldigi_->enable_widgets();
    wsjtx_->enable_widgets();
}

// Button callback
void server_grp::cb_bn_listen(Fl_Widget* w, void* v) {
    server_grp* that = ancestor_view<server_grp>(w);
    server_t server = that->modem();
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
void server_grp::cb_bn_connect(Fl_Widget* w, void* v) {
    server_grp* that = ancestor_view<server_grp>(w);
    server_t server = that->modem();
    qso_manager* mgr = ancestor_view<qso_manager>(that);
    string suffix = mgr->rig_control()->app_suffix();
    // Defualt app is a comment
    string app = "bash -i -c ";
    switch (server) {
    case FLDIGI: 
        app += "fldigi" + suffix + " &";
        break;
    case WSJTX:
        app += "wsjtx" + suffix + " &";
        break;
    default:
        app = "# ";
    }
    char msg[128];
    snprintf(msg, sizeof(msg), "DASH: Starting app: %s", app.c_str());
    status_->misc_status(ST_NOTE, msg);
    system(app.c_str());
    that->enable_widgets();
  
}

// Set modem
void server_grp::modem(server_t t) {
    modem_ = t;
}

server_t server_grp::modem() {
    return modem_;
}