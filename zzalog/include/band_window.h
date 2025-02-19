#pragma once
#include <FL/Fl_Double_Window.H>

class band_widget;

class band_window :
    public Fl_Double_Window
{
public:
    band_window(int X, int Y, int W, int H, const char* L = nullptr);
    ~band_window();

    // Set the frequency
    void set_frequency(double f);

protected:
    band_widget* bw_;

};

