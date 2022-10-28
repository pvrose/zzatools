#pragma once
#include <FL/Fl_Window.H>
#include <FL/Fl_Help_View.H>

class QBS_reporter :
    public Fl_Window
{
public:
    QBS_reporter(int W, int H, const char* L);
    ~QBS_reporter();

    void text(const char* value);

protected:
    Fl_Help_View* display_;
};

