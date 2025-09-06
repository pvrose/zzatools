#pragma once

#include <string>

#include <FL/Fl_Window.H>

using namespace std;
class Fl_Text_Editor;
class Fl_Text_Buffer;
class Fl_Button;

//! Rudimentary file editor
class file_viewer :
    public Fl_Window
{
public:
    //! Constructor.

    //! \param W width 
    //! \param H height
    //! \param L label
    file_viewer(int W, int H, const char* L = nullptr);
    //! Destructor.
    ~file_viewer();

    //! Instantiate component widgets.
    void create();
    //! Configure widgets after data changes.
    void enable_widgets();

    //! Load  specified file \p name into the editor.
    void load_file(string name);

    //! Returns name of displayed file
    string file();

    //! File has been changed and not saved.
    bool is_dirty();

    //! Callback from window close button.
    static void cb_close(Fl_Widget* w, void* v);
    //! Callback from "Save" button, saves the file.
    static void cb_save(Fl_Widget* w, void* v);
    //! Callback from "Reload" button, clears buffer and loads file again without saving any edits.
    static void cb_reload(Fl_Widget* w, void* v);

    //! Callback when text buffer in the editor has been modified.
    //! 
    //! Only parameters \p inserted and \p deleted are used and indicate that a change 
    //! has been made, so marks the data "dirty".
    static void cb_modified(int pos, int inserted, int deleted, int restyled, const char* deletion, void* arg);


protected:

    //! Save edited data back to the file.
    void save_file();

    Fl_Text_Editor* display_;    //!< Text editor widget.

    Fl_Text_Buffer* buffer_;     //!< Data buffer used in editor.

    Fl_Button* bn_save_;         //!< "Save" button.
    Fl_Button* bn_reload_;       //!< "Reload" button.

    string filename_;            //!< Name of file being edited.
    bool dirty_;                 //!< Data is "dirty", ie it has been changed and 
                                 //!< not written to store.

};

