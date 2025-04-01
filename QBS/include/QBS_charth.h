#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Scrollbar.H>

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

    static void cb_scroll(Fl_Widget* w, void* v);

protected:

    QBS_data* data_;

    // Widgets
    Fl_Chart* chart_;
    Fl_Scrollbar* scroll_;

    Fl_Window* win_tip_;

    void draw_y_axis();
    void draw_legend();
    void draw_average();
    
    void draw_chart();

    // Create a tooltip for chart bar
    void chart_tip();

    int max_;
    // average through range
    double average_;

    // Number of bars in bar chart
    int start_box_;
    int number_boxes_;
    int head_box_;
    int stop_box_;

    string call_;
    
    vector<int> chart_counts_;

};

