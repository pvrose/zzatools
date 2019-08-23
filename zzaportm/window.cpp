#include "window.h"
#include "..//zzalib/drawing.h"
#include "../zzalib/serial.h"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Button.H>

using namespace zzaportm;

window::window(int W, int H, const char* label) :
	Fl_Window(W, H, label)
{
	create_form();
	show();
}

void window::create_form() {
	begin();
	int curr_x = EDGE;
	int curr_y = EDGE + HTEXT;

	Fl_Choice*
}
