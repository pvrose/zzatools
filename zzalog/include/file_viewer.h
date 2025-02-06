#pragma once

#include <string>

#include <FL/Fl_Window.H>

using namespace std;
class Fl_Text_Editor;
class Fl_Text_Buffer;
class Fl_Button;

class file_viewer :
    public Fl_Window
{
public:
    file_viewer(int W, int H, const char* L = nullptr);
    ~file_viewer();

    void create();

    void enable_widgets();

    void load_file(string name);

    // File has been changed
    bool is_dirty();

    static void cb_close(Fl_Widget* w, void* v);
    static void cb_save(Fl_Widget* w, void* v);
    static void cb_reload(Fl_Widget* w, void* v);
    static void cb_modified(int pos, int inserted, int deleted, int restyled, const char* deleteion, void* arg);


protected:

    void save_file();

    Fl_Text_Editor* display_;

    Fl_Text_Buffer* buffer_;

    Fl_Button* bn_save_;
    Fl_Button* bn_reload_;

    string filename_;
    bool dirty_;

};

