#pragma once

#include <string>

#include <FL/Fl_Window.H>

using namespace std;
class Fl_Text_Display;
class Fl_Text_Buffer;

class file_viewer :
    public Fl_Window
{
public:
    file_viewer(int W, int H, const char* L = nullptr);
    ~file_viewer();

    void create();

    void load_file(string name);

    static void cb_close(Fl_Widget* w, void* v);

protected:

    Fl_Text_Display* display_;

    Fl_Text_Buffer* buffer_;

    string filename_;

};

