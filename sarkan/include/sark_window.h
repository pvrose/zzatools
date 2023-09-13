#pragma once

#include "sark_control.h"
#include "sark_graph.h"
#include "sark_data.h"
#include "sark_handler.h"

#include <FL/Fl_Double_Window.H>

class sark_window : public Fl_Double_Window {

public:
    sark_window(int W, int H, const char* L = nullptr);
    ~sark_window();

    sark_graph* graph_;
    sark_control* control_;

    sark_data* data_;
    sark_handler* handler_;

protected:

    static void cb_close(Fl_Widget* w, void* v);

};
