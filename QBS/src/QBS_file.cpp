#include "QBS_file.h"
#include "QBS_data.h"
#include "QBS_window.h"

#include "filename_input.h"
#include "utils.h"

QBS_file::QBS_file(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    win_ = ancestor_view<QBS_window>(this);
    create_form();
}

QBS_file::~QBS_file() 
{}

// main widget crteation
void QBS_file::create_form() {
    begin();

    int curr_x = x() + GAP;
    int curr_y = y() + GAP;

    // Directory input
    curr_x += WLABEL;
    filename_input* ip_csv = new filename_input(curr_x, curr_y, WEDIT, HBUTTON, "CSV data");
    ip_csv->align(FL_ALIGN_LEFT);
    ip_csv->callback(cb_value < Fl_Input, string>, &win_->csv_directory_);
    ip_csv->when(FL_WHEN_CHANGED);
    ip_csv->type(filename_input::DIRECTORY);
    ip_csv->value(win_->csv_directory_.c_str());
    ip_csv->title("Please enter CSV Directory");
    ip_csv->tooltip("Enter the directory containing the .csv files");

    curr_y += HBUTTON + GAP;
    filename_input* ip_qbs = new filename_input(curr_x, curr_y, WEDIT, HBUTTON, "QBS data");
    ip_qbs->align(FL_ALIGN_LEFT);
    ip_qbs->callback(cb_value<Fl_Input, string>, &win_->qbs_filename_);
    ip_qbs->when(FL_WHEN_CHANGED);
    ip_qbs->type(filename_input::FILE);
    ip_qbs->pattern("QBS files\t*.qbs");
    ip_qbs->value(win_->qbs_filename_.c_str());
    ip_qbs->title("Please enter QBS file");
    ip_qbs->tooltip("Enter the QBS filename");

    curr_y += HBUTTON + GAP;

    curr_x = x() + GAP;

    bn_import_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Inport");
    bn_import_->callback(cb_import, nullptr);
    bn_import_->tooltip("Import data from files in specified CSV directory");

    curr_x += WBUTTON + GAP;
    bn_read_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Read QBS");
    bn_read_->callback(cb_read, nullptr);

    end();
    show();

}

void QBS_file::enable_widgets() {
    if (win_->qbs_filename_.length()) {
        // We have a QBS file name
        bn_import_->deactivate();
    }
    else {
        bn_import_->activate();
    }
}

// Callbacks - import CVS files
void QBS_file::cb_import(Fl_Widget* w, void* v) {
    QBS_file* that = ancestor_view<QBS_file>(w);
    that->win_->data_->import_cvs(that->win_->csv_directory_);
    that->enable_widgets();  
}

// Callback - read QBS file
void QBS_file::cb_read(Fl_Widget* w, void* v) {
    QBS_file* that = ancestor_view<QBS_file>(w);
}


