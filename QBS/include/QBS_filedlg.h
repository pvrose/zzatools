#pragma once

#include <string>

#include <FL/Fl_Group.H>

using namespace std;

class QBS_filedlg :
    public Fl_Group
{
    QBS_filedlg(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_filedlg();

    // Convert CVS file to QBS file
    static void cb_convert(Fl_Widget* w, void* v);
    // Read QBS file and change state to QUIET
    static void cb_done(Fl_Widget* w, void* v);

    // Attributes
    string cvs_filename_;
    string qbs_filename_;

};

