#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>

class Fl_Button;

//! This class provides an input plus a button combination that are connected.

//! The button will affect the input is some way, either opening up some browser 
//! or affecting its format.
class button_input :
	public Fl_Group
{
protected:

	//! The Fl_Input widget
	Fl_Input* ip_;
	//! The Fl_Button widget
	Fl_Button* bn_;

public:
	//! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
	button_input(int X, int Y, int W, int H, const char* L = nullptr) :
		Fl_Group(X, Y, W, H, L)
	{
		box(FL_FLAT_BOX);
		// Set the button width to H or 0.2 W whichever is less
		int bw = H < W / 5 ? H : W / 5;

		ip_ = new Fl_Input(X, Y, W - bw, H);
		bn_ = new Fl_Button(X + W - bw, Y, bw, H);

		end();

	};
	//! Destructor
	~button_input() {};

	//! Inherited from Fl_Group - to be overridden by inheritee.
	
	//! The default is to std::set the callback of the Fl_Input to the specified callback.
	virtual void callback(Fl_Callback* cb, void* v) { ip_->callback(cb, v); }
	//! \see callback
	virtual void callback(Fl_Callback* cb) { ip_->callback(cb); }
	//! \see callback
	virtual Fl_Callback_p callback() { return ip_->callback(); }

	//! Inherited from Fl_Group - to be overridden by inheritee
	
	//! The default sets the when condition onto the Fl_Input component.
	virtual void when(uchar i) { ip_->when(i); }
	//! \see when
	virtual Fl_When when() { return ip_->when(); }

	//! Inherited from Fl_Group - to be overridden by inheritee

	//! The default forwards the user data to the Fl_Input component.
	virtual void user_data(void* v) { ip_->user_data(v); }
	//! \see user_data
	virtual void* user_data() { return ip_->user_data(); }

	//! Inherited from Fl_Group - to be overridden by inheritee

	//! The default forwards the value to the Fl_Input component.
	void value(const char* d) { ip_->value(d); }
	//! \see value
	const char* value() { return ip_->value(); }

	//! Returns the internal Fl_Input widget
	Fl_Input* input() { return ip_; };
	//! Returns the internal Fl_Button widget.
	Fl_Button* button() { return bn_; };


};

