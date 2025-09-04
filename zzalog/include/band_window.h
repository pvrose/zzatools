#pragma once
#include <FL/Fl_Double_Window.H>

class band_widget;

//! A separate window that displays the full bandplan view.
class band_window :
    public Fl_Double_Window
{
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    band_window(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor
    ~band_window();

    //! Overload of Fl_Double_Window::draw().
    
    //! Passes on the selection colours to band_widget. 
    virtual void draw();
    
    //! Set the frequency
    
    //! \param tx transmit frequency.
    //! \param rx receive frequency.
    void set_frequency(double tx, double rx);

    //! Callback from band_widget.
    
    //! Callback called when band_widget is clicked. The frequency
    //! under the mouse click is read and passed to the current rig
    //! to switch frequency to that clicked.
    static void cb_widget(Fl_Widget* w, void* v);

protected:
    //! The band_widget instance.
    band_widget* bw_;

};

