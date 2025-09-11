#pragma once

#include <cstdint>
#include <FL/Fl_Double_Window.H>

enum status_t : char;
enum object_t : char;

// FLTK classes
class Fl_Box;
class Fl_Button;
class Fl_Fill_Dial;
class Fl_Multiline_Output;
class Fl_Output;
class Fl_Text_Display;

//!  A banner display to show progress and report events to the user.
class banner :
    public Fl_Double_Window
{
public:
    //! Constructor - see Fl_Double_Window.

    //! \param W width.
    //! \param H height.
    //! \param L label.
    banner(int W, int H, const char* L = nullptr);
    //! Destructor
    ~banner();
    //! Overload of Fl_Double_Window::handle.

    //! This takes focus events so that clicking and typing F1 opens userguide.
    virtual int handle(int event);
    //! Instantiates all the component widgets.
    void create_form();
    //! Enables and/or configures the compoment widgets in response to a change of conditions.
    void enable_widgets();

    //! Add a message to the banner.
    
    //! \param type the status of the message. This determines which output widget displays the
    //! message and the colour in which to display it.
    //! \param msg the message to display.
    void add_message(status_t type, const char* msg);
    //! Start a progress clock.
    
    //! \param max_value the maximum count of object_t items expected.
    //! \param object identifier of the object_t item.
    //! \param msg message to accompany any display of progress.
    //! \param suffix textual description of the objecy (eg bytes or records).
    void start_progress(uint64_t max_value, object_t object, const char* msg, const char* suffix);
    //! Report on progress - update the clock.
    
    //! \param value the current progress value. The display is only updated if the 
    //! change in progress in greater than one hundredth of the maximum value.
    void add_progress(uint64_t value);
    //! End a progress report normally. However if add_progress indicates 100% complete then
    //! the progress report will indicate complete.
    void end_progress();
    //! Cancel a progress report if an abnormal condition occurred.
    
    //! \param msg message to indicate the reason for the cancellation.
    void cancel_progress(const char* msg);

    //! All the banner to be closed by the system close button.
    
    //! Normally the banner cannot be closed, however if ZZALOG has thrown a fatal or severe
    //! error, the banner will be left visible. In this condition, it can only be closed
    //! by the system close button.
    void allow_close();
 
    //! Callback - system close button.
    
    //! The callback normally ignores the click of this button, but can
    //! be configured to action the normal window close action.
    static void cb_close(Fl_Widget* w, void* v);

    //! Override Fl_Double_Window::draw().
    //! 
    //! When ZZALOG is closing, the word "CLOSING" is displayed across the banner.
    //! This provides an indication to the user that ZZALOG is, in fact, closing as
    //! this can in some cases take a noticeable time.
    virtual void draw();

protected:

    //! Add message to the message history display (with colour)
    void copy_msg_display(status_t type, const char* msg);

    // Widgets
    Fl_Box* bx_icon_;                   //!< holder for the ZZALOG icon.
    Fl_Fill_Dial* fd_progress_;         //!< progress "clock".
    Fl_Multiline_Output* op_msg_low_;   //!< output for low cetegory messages (ST_WARNING and below).
    Fl_Multiline_Output* op_msg_high_;  //!< output for high category messages (ST_ERROR and above).
    Fl_Output* op_prog_title_;          //!< output for progress text message.
    Fl_Box* bx_prog_value_;             //!< display for current progtress value.
    Fl_Text_Display* display_;          //!< display for message history log.
    Fl_Box* bx_closing_;                //!< container for "CLOSING" message.

    //! Progress maximum value
    uint64_t max_value_;
    //! Delta value to trigger update of progress. Set to a fixed fraction of the maximum value.
    uint64_t delta_;
    //! Previous progress value. The progress clock is updated if the new value is greater than
    //! prev_value_ + delta_.
    uint64_t prev_value_;
    //! Progress unit. Eg bytes or records.
    const char* prg_unit_;
    //! Progress message.
    const char* prg_msg_;
    //! Progress object - affects the colour of the progress clock.
    object_t prg_object_;
    //! Flag to allow the closure of banner.
    bool can_close_;

};

