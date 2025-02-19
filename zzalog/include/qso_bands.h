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

    band_widget* summary_;

    band_window* full_window_;
};

