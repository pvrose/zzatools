#include "drawing.h"
#include "fllog_emul.h"
#include "wsjtx_handler.h"

#include "qso_server.h"

using namespace std;

extern fllog_emul* fllog_emul_;
extern wsjtx_handler* wsjtx_handler_;

qso_server::qso_server(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    load_values();
    create_form();
    enable_widgets();
}

qso_server::~qso_server() {
    save_values();
}

void qso_server::load_values() {
    // NULL action
}

void qso_server::create_form() {
    int curr_x = x() + GAP + WLABEL;
    int curr_y = y() + GAP;
    
    // Chack button for FLDIGI
    bn_fldigi_on_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "FLDIGI");
    bn_fldigi_on_->when(FL_WHEN_NEVER);
    bn_fldigi_on_->type(FL_NORMAL_BUTTON);
    bn_fldigi_on_->align(FL_ALIGN_LEFT);
    bn_fldigi_on_->tooltip("Indicates that the FLDIGI handler is active");

    curr_x += HBUTTON;
    // Switch state button
    bn_fldigi_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Start");
    bn_fldigi_->callback(cb_bn_change, (void*)FLDIGI);
    bn_fldigi_->tooltip("Change the state of the FLDIGI handler");

    curr_x = x() + GAP + WLABEL;
    curr_y += HBUTTON;

    // Chack button for WSJT-X
    bn_wsjtx_on_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "WSJT-X");
    bn_wsjtx_on_->when(FL_WHEN_NEVER);
    bn_wsjtx_on_->type(FL_NORMAL_BUTTON);
    bn_wsjtx_on_->align(FL_ALIGN_LEFT);
    bn_wsjtx_on_->tooltip("Indicates that the WSJT-X handler is active");

    curr_x += HBUTTON;
    // Switch state button
    bn_wsjtx_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Start");
    bn_wsjtx_->callback(cb_bn_change, (void*)WSJTX);
    bn_fldigi_->tooltip("Change the state of the WSJT-X handler");

    end();
}

void qso_server::save_values() {
    // NULL Function
}

void qso_server::enable_widgets() {
    // Fldigi widgets
    if (fllog_emul_ && fllog_emul_->has_server()) {
        bn_fldigi_on_->value(true);
        bn_fldigi_->label("Stop");
    } else {
        bn_fldigi_on_->value(false);
        bn_fldigi_->label("Start");
    }
    // WSJTX widgets
    if (wsjtx_handler_ && wsjtx_handler_->has_server()) {
        bn_wsjtx_on_->value(true);
        bn_wsjtx_->label("Stop");
    } else {
        bn_wsjtx_on_->value(false);
        bn_wsjtx_->label("Start");
    }
}

// Button callback
void qso_server::cb_bn_change(Fl_Widget* w, void* v) {
    qso_server* that = ancestor_view<qso_server>(w);
    server_t server = (server_t)(intptr_t)v;
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
