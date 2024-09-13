#pragma once
#include "button_input.h"
class password_input :
    public button_input
{

public:
    password_input(int X, int Y, int W, int H, const char* L = nullptr);
    ~password_input() {};

    static void cb_button(Fl_Widget* w, void* v);



};

