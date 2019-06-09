#ifndef __PAGE_DIALOG__
#define __PAGE_DIALOG__

#include "callback.h"
#include "drawing.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

namespace zzalog {
	enum cfg_action_t {
		CA_OK,                  // OK button pressed
		CA_SAVE,                // Save button pressed
		CA_CANCEL               // Cancel button pressed
	};

	enum cfg_dialog_t {
		DLG_FORMAT,
		DLG_RIG,
		DLG_FILES,
		DLG_WEB,
		DLG_STATION,
		DLG_COLUMN,
		DLG_ADIF,
		DLG_ALL
	};

	// config window default sizes
	const int WCONFIG = 600;
	const int HCONFIG = 500;

	// This class is a base class for use with settings dialogs
	class page_dialog : public Fl_Group

	{
	public:
		page_dialog(int X, int Y, int W, int H, const char* label = nullptr);
		virtual ~page_dialog();

	protected:
		// CB from external source - usually the controlling tabbed view OK/Cancel
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// CB that also calls enable_widgets
		static void cb_ch_enable(Fl_Widget* w, void* v);

		// Standard methods - need to be written for each class that inherits from this
		// Load values from settings_
		virtual void load_values() = 0;
		// Used to create the form
		virtual void create_form(int X, int Y) = 0;
		// Used to write settings back
		virtual void save_values() = 0;
		// Used to enable/disable specific widget - any widgets enabled musr be attributes
		virtual void enable_widgets() = 0;

		// standard creation
		void do_creation(int X, int Y);
	};

}
#endif