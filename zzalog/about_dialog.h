#ifndef __ABOUT_DIALOG__
#define __ABOUT_DIALOG__

#include "win_dialog.h"

#include <string>

#include <FL/Fl_Widget.H>

using namespace std;

namespace zzalog {

	// Program strings
	const string COPYRIGHT = "© Philip Rose GM3ZZA 2018. All rights reserved.\n (Prefix data, DX Atlas & OmniRig interfaces © Alex Shovkoplyas, VE3NEA.)";
	const string PROGRAM_ID = "ZZALOG";
	const string PROG_ID = "ZLG";
	const string VERSION = "3.1.15";
#ifdef _DEBUG
	const string PROGRAM_VERSION = VERSION + " (Debug)";
#else
	const string PROGRAM_VERSION = VERSION;
#endif
	const string VENDOR = "GM3ZZA";


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

