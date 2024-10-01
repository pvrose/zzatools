#include "qsl_widget.h"
#include "qsl_display.h"

qsl_widget::qsl_widget(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L)
{
	box(FL_BORDER_BOX);
	display_ = new qsl_display(x(), y(), w(), h());
}

qsl_widget::~qsl_widget() {}

// Draw will draw the QSL design, image or text after resizing
void qsl_widget::draw() {
	display_->resize(w(), h());
	display_->draw();
}

// Handle intercepts mouse button click to invoke the callback
int qsl_widget::handle(int event) {
	switch (event) {
	case FL_RELEASE: {
		switch (Fl::event_button()) {
		case FL_LEFT_MOUSE: {
			if ((when() & FL_WHEN_RELEASE) == FL_WHEN_RELEASE) do_callback();
			return true;
		}
		}
		break;
	}
	}
	return Fl_Widget::handle(event);
}

// Return a pointer to the qsl_display instance
qsl_display* qsl_widget::display() {
	return display_;
}
