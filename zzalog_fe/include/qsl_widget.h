#pragma once
#include <FL/Fl_Widget.H>

class qsl_display;

//! Class to provide a widget to encapsulate the QSL design image.

//! It allows the drawing to be scaled to fit the size of this widget.
class qsl_widget :
    public Fl_Widget
{
public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qsl_widget(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~qsl_widget();

    //! Display widget.
    qsl_display* display();
    //! Override of Fl_Widget::draw().
    
    //! Scales the qsl_display object and draws an enclosing box.
    virtual void draw();

    //! Override of Fl_Widget::handle().
    
    //! Invokes a callback when the left mouse button is released. This 
    //! is usually used to open a window with a full-size version of
    //! the qsl_display.
    virtual int handle(int event);

protected:
    
    //! The QSL image container.
    qsl_display* display_;

};

