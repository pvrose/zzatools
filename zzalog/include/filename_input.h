#include "button_input.h"

class filename_input :
    public button_input
{

public:
    filename_input(int X, int Y, int W, int H, const char* L = nullptr);
    ~filename_input();

    void info(const char* title, const char* pattern);

protected:

    static void cb_button(Fl_Widget* w, void* v);

    const char* title_;
    const char* pattern_;

};
