#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Chart.H>

using namespace std;

class QBS_data;

class QBS_charth :
    public Fl_Group
{

public:
    QBS_charth(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_charth();

    void create_form();

    void data(QBS_data* d);

    void update(string call);

    virtual void draw();

    virtual int handle(int event);

protected:

    QBS_data* data_;

    // Widgets
    Fl_Chart* ct_received_;
    Fl_Chart* ct_recycled_;
    Fl_Chart* ct_sent_;

    Fl_Window* win_tip_;

    void draw_y_axis();

    // Create a tooltip for chart bar
    void chart_tip();

    int max_;

    // Number of bars in bar chart
    int start_box_;
    int number_boxes_;
    
    // Counts in the charts
    struct counts {
        int rcvd;
        int rcyc;
        int sent;
    };
    vector<counts> chart_counts_;

};

