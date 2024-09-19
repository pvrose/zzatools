#include "button_input.h"

class filename_input :
    public button_input
{

public:

    enum type_t : uchar {
        FILE,
        DIRECTORY
    };
    filename_input(int X, int Y, int W, int H, const char* L = nullptr);
    ~filename_input();

    // Set title on file chooser dialog
    void title(const char* value);
    // Set filter pattern
    void pattern(const char* value);
    // Set chooser type - only
    void type(type_t value);


protected:

    static void cb_button(Fl_Widget* w, void* v);

    const char* title_;
    const char* pattern_;

    type_t type_;
};
