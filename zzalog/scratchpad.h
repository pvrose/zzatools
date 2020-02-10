#ifndef __SCRATCHPAD__
#define __SCRATCHPAD__

#include "../zzalib/win_dialog.h"
#include "record.h"
#include "book.h"
#include "intl_widgets.h"

#include <string>

#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>

using namespace std;

namespace zzalog {

	// This class provides a standalone dialog in which user can type real-time data from the QSO and set fields in the record
	class scratchpad :
		public win_dialog
	{
	public:
		scratchpad();
		~scratchpad();

		// Callback - close button
		static void cb_close(Fl_Widget* w, void* v);

		// Called when rig is read
		void rig_update(string frequency, string mode, string power);
		// Called when rig is closed
		void update();

	protected:
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
		// Callback - frequency input
		static void cb_ip_freq(Fl_Widget* w, void* v);
		// Callback - band choice
		static void cb_ch_mode(Fl_Widget* w, void* v);
		// Callback - power input
		static void cb_ip_power(Fl_Widget* w, void* v);

		// Enable widgets
		void enable_widgets();

		// Actions attached to the various buttons
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
		intl_editor* editor_;
		// Save button
		Fl_Button* bn_save_;
		// Cancel button
		Fl_Button* bn_cancel_;
		// Start button
		Fl_Button* bn_start_;
		// Frequency input
		Fl_Input* ip_freq_;
		// Mode choice
		Fl_Choice* ch_mode_;
		// Power input
		Fl_Input* ip_power_;

		// The record being entered
		record* record_;
		// The fieldname
		string field_;
	};

}
#endif