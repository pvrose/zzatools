#pragma once
#include <FL/Fl_Widget.H>

class qsl_display;

class qsl_widget :
    public Fl_Widget
{
public:
    
    qsl_widget(int X, int Y, int W, int H, const char* L = nullptr);
    ~qsl_widget();

    qsl_display* display();

    virtual void draw();

    virtual int handle(int event);

protected:
    
    qsl_display* display_;

};

