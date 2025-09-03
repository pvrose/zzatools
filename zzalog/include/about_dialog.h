#ifndef __ABOUT_DIALOG__
#define __ABOUT_DIALOG__

#include "win_dialog.h"

#include <FL/Fl_Widget.H>

using namespace std;

	//! Dialog to provide program information.
	
	//! This supplies details of the compilation status of ZZALOG. 
	//! This acknowledges all third-party libraries and data used in ZZALOG.
	class about_dialog :
		public win_dialog
	{
	public:
		//! Constructor.
		about_dialog();
		//! Destructor.
		virtual ~about_dialog();

		//! Callback when \p OK button is clicked.
		//! \param w the clicked button
		//! \param v not used.
		static void cb_bn_ok(Fl_Widget* w, void* v);
		//! Callback when \p CANCEL button clicked.
		//! \param w the clicked button
		//! \param v not used.
		static void cb_bn_cancel(Fl_Widget* w, void * v);

	protected:

	};

#endif

