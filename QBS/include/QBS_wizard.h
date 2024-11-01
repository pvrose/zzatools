#pragma once
#include "QBS_consts.h"

#include <FL/Fl_Wizard.H>

class QBS_wizard :
    public Fl_Wizard
{
public:
    QBS_wizard(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_wizard();

    void enable_widgets() {};

protected:
    process_mode_t process_;
public:
    process_mode_t process() { return process_;  }
    void process(process_mode_t p) { process_ = p;  }

};

