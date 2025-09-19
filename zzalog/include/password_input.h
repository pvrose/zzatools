#pragma once
#include "button_input.h"

//! This class implements a combined input and button.

//! The button chnages the display mode of the input between clear text and hidden text. 
class password_input :
    public button_input
{

public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    password_input(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~password_input() {};

    //! Callback to switch input between FL_NORMAL_INPUT and FL_SECRET_INPUT modes.
    static void cb_button(Fl_Widget* w, void* v);



};

