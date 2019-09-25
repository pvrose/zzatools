#ifndef __ABOUT_DIALOG__
#define __ABOUT_DIALOG__

#include "win_dialog.h"

#include <string>

#include <FL/Fl_Widget.H>

using namespace std;

namespace zzalog {


	// Dialog to provide program information
	class about_dialog :
		public win_dialog
	{
	public:
		about_dialog();
		virtual ~about_dialog();

		// callback - OK button
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// callback - cancel button (window close only)
		static void cb_bn_cancel(Fl_Widget* w, void * v);

	protected:

	};

}

#endif

