#pragma once
#include <FL/Fl_Double_Window.H>

class band_widget;

class band_window :
    public Fl_Double_Window
{
public:
    band_window(int X, int Y, int W, int H, const char* L = nullptr);
    ~band_window();

 
    virtual void draw();
    
    // Set the frequency
    void set_frequency(double tx, double rx);

    // Callback from widget
    static void cb_widget(Fl_Widget* w, void* v);

protected:
    band_widget* bw_;

};

