#include "button_input.h"

//! This class implements a combination of input widget and a button which opens a file browser.
class filename_input :
    public button_input
{

public:

    //! Defines browser type to open
    enum type_t : uchar {
        FILE,            //!< Opens a file browser
        DIRECTORY        //!< Opens a directory browser
    };
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    filename_input(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~filename_input();

    //! Set title to \p value on file chooser dialog
    void title(const char* value);
    //! Set filename filter pattern to \p value.
    void pattern(const char* value);
    //! Set chooser type to \p value
    void type(type_t value);


protected:

    //! \brief Callback from clicking button: opens file or directory browser, 
    //! and selected object transferred to input widget.
    static void cb_button(Fl_Widget* w, void* v);

    //! Browser label.
    const char* title_;
    //! Pattern to filter filenames
    const char* pattern_;

    //! Browser type (FILE or DIRECTORY)
    type_t type_;
};
