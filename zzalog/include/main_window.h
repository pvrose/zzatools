#ifndef __MAIN_WINDOW__
#define __MAIN_WINDOW__

#include <FL/Fl_Double_Window.H>





	//! This calss inherits from Fl_Single_Window and is the main application window.
	
	//! It allows custom handling FL_SHOW and FL_HIDE events
	class main_window :
		public Fl_Double_Window
	{
	public:
		//! Constructor.

		//! \param W width 
		//! \param H height
		//! \param label label
		main_window(int W, int H, const char* label = nullptr);
		//! Destructor.
		~main_window();

		//! Override Fl_Double_Window::handle.
		//! 
		//! On HIDE and SHOW let menu know visibility.
		//! Accept paste events to import QSO records from the clipboard.
		virtual int handle(int event);
	};

#endif