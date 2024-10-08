#ifndef __PAGE_DIALOG__
#define __PAGE_DIALOG__

#include <FL/Fl_Group.H>


	// Standard settings dialog ahs OK, Save and Cancel buttons - each settings tab needs to implement them.
	enum cfg_action_t {
		CA_OK,                  // OK button pressed
		CA_SAVE,                // Save button pressed
		CA_CANCEL               // Cancel button pressed
	};

	// This class is a base class for the individual tabs within the settings dialog
	class page_dialog : public Fl_Group

	{
	public:
		page_dialog(int X, int Y, int W, int H, const char* label = nullptr);
		virtual ~page_dialog();

		// Update - implementation specific action, default do nothing.
		virtual void update() {};

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
		// Used to enable/disable specific widget - any widgets enabled must be attributes
		virtual void enable_widgets() = 0;

		// standard creation
		void do_creation(int X, int Y);
	};
#endif