#ifndef __WINDOW__
#define __WINDOW__

#include "..//zzalib/serial.h"

#include <FL/Fl_Window.H>

namespace zzaportm {

	class window :
		public Fl_Window
	{
	public:
		window(int W, int H, const char* label);
		~window();

	protected:
		// The method to create the forms
		void create_form();

		//attributes
		serial* port_;


	};

}

#endif