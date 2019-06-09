#ifndef __SCRATCHPAD__
#define __SCRATCHPAD__

#include "win_dialog.h"
#include "record.h"
#include "book.h"

#include <string>

#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>

using namespace std;

namespace zzalog {

	class scratchpad :
		public win_dialog
	{
	public:
		scratchpad();
		~scratchpad();

		// Callback - close button
		static void cb_close(Fl_Widget* w, void* v);

	protected:
		class editor : public Fl_Text_Editor
		{
		public:
			editor(int X, int Y, int W, int H);
		protected:
			// Handle single left click and right click
			int handle(int event);
		};
		// Create the form
		void create_form();

		// Call back - general button action
		static void cb_action(Fl_Widget* w, void* v);
		// Callback - save button
		static void cb_save(Fl_Widget* w, void* v);
		// Callback - cancel button
		static void cb_cancel(Fl_Widget* w, void* v);
		// Callback - start button
		static void cb_start(Fl_Widget* w, void* v);

		// Enable widgets
		void enable_widgets();

		enum actions {
			WRITE_CALL,
			WRITE_NAME,
			WRITE_QTH,
			WRITE_GRID,
			WRITE_RST_SENT,
			WRITE_RST_RCVD,
			WRITE_FIELD
		};

		// The text items
		Fl_Text_Buffer* buffer_;
		// The editor
		Fl_Text_Editor* editor_;
		// Save button
		Fl_Button* bn_save_;
		// Cancel button
		Fl_Button* bn_cancel_;
		// Start button
		Fl_Button* bn_start_;

		// The record being entered
		record* record_;
		// The fieldname
		string field_;
	};

}
#endif