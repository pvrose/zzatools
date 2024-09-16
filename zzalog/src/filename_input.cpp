#include "filename_input.h"
#include "utils.h"

#include <FL/Fl_Native_File_Chooser.H>

filename_input::filename_input(int X, int Y, int W, int H, const char* L) :
    button_input(X, Y, W, H, L) ,
    title_(nullptr) ,
    pattern_(nullptr)
{
    bn_->label("@fileopen");
    bn_->callback(cb_button);

}

filename_input::~filename_input() {}

void filename_input::info(const char* title, const char* pattern) {
    title_ = title;
    pattern_ = pattern;
}

void filename_input::cb_button(Fl_Widget* w, void* v) {
    filename_input* that = ancestor_view<filename_input>(w);
    string filename = that->ip_->value();

	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    chooser->title(that->title_);
    chooser->filter(that->pattern_);
    chooser->directory(directory(filename).c_str());
    chooser->preset_file(terminal(filename).c_str());
    // Now display the dialog
    switch(chooser->show()) {
        case 0: 
        that->ip_->value(chooser->filename());
        break;

        case -1:
        // Error
        printf("ERROR: %s\n", chooser->errmsg());
        break;

    }
    that->ip_->do_callback();


}
