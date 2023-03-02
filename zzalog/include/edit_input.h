#ifndef __EDIT_DIALOG__
#define __EDIT_DIALOG__

#include "intl_widgets.h"

using namespace std;



	// Create a custom version of intl_input so that we can handle keyboard events
	class edit_input :
		public intl_input
	{
	public:

		enum edit_exit_t {
			SAVE,           // Save and close
			SAVE_NEXT,      // Save data and open next item
			SAVE_PREV,      // Save data and open previous
			SAVE_UP,        // Save data and open item in above record
			SAVE_DOWN       // Save data and open item in below record
		};
		edit_input(int X, int Y, int W, int H);
		virtual int handle(int event);

	};
#endif