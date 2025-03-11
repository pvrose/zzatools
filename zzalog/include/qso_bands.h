#pragma once
#include <FL/Fl_Group.H>

class band_window;
class band_widget;

class qso_bands :
    public Fl_Group
{
public:

    qso_bands(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_bands();

    // LLoad settings
    void load_values();
    // Create widgets
    void create_form();
    // Save settimngs
    void save_values();
    // Configure widgets
    void enable_widgets();

    static void cb_band(Fl_Widget* w, void* v);

    // Ticker callback
    static void cb_ticker(void* v);

protected:
    band_widget* summary_;

    band_window* full_window_;

    // Saved windows coordinates
    int left_, top_, width_, height_;
    bool open_window_;

};

