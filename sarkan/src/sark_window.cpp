#include "sark_window.h"
#include "sark_graph.h"
#include "sark_control.h"
#include "sark_drawing.h"

sark_window::sark_window(int W, int H, const char* L)
    : Fl_Double_Window(W, H, L) 
    , data_(nullptr)
    , handler_(nullptr)
{
    callback(cb_close);

    control_ = new sark_control(GAP, GAP, 10, 10);
    int w = control_->x() + control_->w() + GAP;
    int h = control_->y() + control_->h();

    graph_ = new sark_graph(w, GAP, 600, 450);
    
    w += graph_->w();
    h = max(h, graph_->y() + graph_->h());

    w += GAP;
    h += GAP;

    resizable(nullptr);
    size(w, h);

    control_->update_sark_graph();

}

sark_window::~sark_window() {}
    
void sark_window::cb_close(Fl_Widget* w, void* v) {
    sark_window* win = (sark_window*)w;
    delete win->control_;
    default_callback(win, v);
}