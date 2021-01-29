#ifndef __MAIN_WINDOW__
#define __MAIN_WINDOW__

#include <FL/Fl_Single_Window.H>

using namespace std;

namespace zzalog {

	class main_window :
		public Fl_Single_Window
	{
	public:

		main_window(int W, int H, const char* label = nullptr);
		~main_window();

		// Handle FL_HIDE and FL_SHOW
		virtual int handle(int event);
	};


}
#endif