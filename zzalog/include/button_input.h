#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>

class Fl_Button;

class button_input :
	public Fl_Group
{
protected:

	Fl_Input* ip_;
	Fl_Button* bn_;

public:
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
	~button_input() {};

	// Intercept setting callback to apply it to the Fl_Input
	virtual void callback(Fl_Callback* cb, void* v) { ip_->callback(cb, v); }
	virtual void callback(Fl_Callback* cb) { ip_->callback(cb); }
	virtual Fl_Callback_p callback() { return ip_->callback(); }

	// Interecpt setting when for ditto
	virtual void when(uchar i) { ip_->when(i); }
	virtual Fl_When when() { return ip_->when(); }

	// Ditto for user data
	virtual void user_data(void* v) { ip_->user_data(v); }
	virtual void* user_data() { return ip_->user_data(); }

	// Setting the input
	void value(const char* d) { ip_->value(d); }
	const char* value() { return ip_->value(); }

	// Access to the internal widgets
	Fl_Input* input() { return ip_; };
	Fl_Button* button() { return bn_; };


};

