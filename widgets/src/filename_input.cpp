#include "filename_input.h"
#include "utils.h"

#include <FL/Fl_Native_File_Chooser.H>

filename_input::filename_input(int X, int Y, int W, int H, const char* L) :
    button_input(X, Y, W, H, L) ,
    title_(nullptr) ,
    pattern_(nullptr) ,
    type_(FILE)
{
    bn_->label("@fileopen");
    bn_->callback(cb_button);

}

filename_input::~filename_input() {}

void filename_input::title(const char* value) {
    title_ = value;
}

void filename_input::pattern(const char* value) {
    pattern_ = value;
}

void filename_input::type(filename_input::type_t value) {
    type_ = value;
}

void filename_input::cb_button(Fl_Widget* w, void* v) {
    filename_input* that = ancestor_view<filename_input>(w);
    std::string filename = that->ip_->value();
    Fl_Native_File_Chooser::Type fb_type = Fl_Native_File_Chooser::BROWSE_FILE;
    switch(that->type_) {
        case FILE:
        fb_type = Fl_Native_File_Chooser::BROWSE_FILE;
        break;
        case DIRECTORY:
        fb_type = Fl_Native_File_Chooser::BROWSE_DIRECTORY;
        break;
    }

	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(fb_type);
    chooser->title(that->title_);
    if (that->type_ == FILE) {
        chooser->filter(that->pattern_);
        chooser->directory(directory(filename).c_str());
    } else {
        chooser->directory(filename.c_str());
    }
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
