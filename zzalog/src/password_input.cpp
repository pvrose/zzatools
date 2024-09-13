#include "password_input.h"

password_input::password_input(int X, int Y, int W, int H, const char* L) :
	button_input(X, Y, W, H, L)
{
	ip_->type(FL_SECRET_INPUT);
	bn_->label("@eyeshut");
	bn_->callback(cb_button);
}

void password_input::cb_button(Fl_Widget* w, void* v) {
	password_input* g = (password_input*)w->parent();
	Fl_Input* ip = g->ip_;
	Fl_Button* bn = g->bn_;
	switch (ip->type()) {
	case FL_SECRET_INPUT:
		ip->type(FL_NORMAL_INPUT);
		bn->label("@eyeopen");
		break;
	case FL_NORMAL_INPUT:
		ip->type(FL_SECRET_INPUT);
		bn->label("@eyeshut");
		break;
	}
	ip->redraw();
}
