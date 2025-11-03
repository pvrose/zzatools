#ifndef __CHANGE_DIALOG__
#define __CHANGE_DIALOG__

#include "win_dialog.h"
#include "callback.h"

class Fl_Widget;

	//! Values for attaching to the action radio buttons.
	enum change_action_t {
		RENAME_FIELD = 0,   //!< Rename the field.
		DELETE_FIELD = 1,   //!< Delete the field.
		ADD_FIELD = 2,      //!< Add a field without changing any already there.
		CHANGE_FIELD = 3    //!< Add, and change existing fields with this name.
	};

	//! \brief This class implements a dialog that asks a user how to make similar
	//! changes to all records in the log.
	class change_dialog :
		public win_dialog
	{
	public:
		//! Constructor.
		
		//! \param label std::set the window label.
		change_dialog(const char* label = 0);
		//! Destructor.
		virtual ~change_dialog();

		//! Overload of win_dialog::handler.
		
		//! Intercepts click events to allow keyboard F1
		//! to open userguide for this dialog.
		virtual int handle(int event);

		//! Instantiate all the component widgets.
		void create_form();
		//! Get the changes specified by the dialog.
		void get_data(change_action_t& action, std::string& old_field_name, std::string& new_field_name, std::string& new_text);

		//! Callback for OK button: sets the OK response for the dialog.
		static void cb_bn_ok(Fl_Widget* w, void* v);
		//! Callback for "Cancel" button: sets the CANCEL response for the dialog.
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		//! Callback for the "Action" button: sets the SPARE response for the dialog.
		static void cb_bn_action(Fl_Widget* w, void* v);
		//! Callback for the radio buttons: updates the dialog appropriately. 
		static void cb_radio_action(Fl_Widget* w, void* v);
		//! Callback for the "Old field" choice: saves the value and updates the dialog appropriately.
		static void cb_ch_old_field(Fl_Widget* w, void* v);

	protected:
		//! Update the dialog as a result of changes.
		void enable_widgets();
		//! Populate the choice menu for the enumeration selector of possible field values.
		void populate_enum(std::string name);
		//! Original field name when changing field name or the field name when changing values.
		std::string old_field_name_;
		//! Action selected by radio buttons.
		change_action_t action_;
		//! New field name when changing field name.
		std::string new_field_name_;
		//! New value for the field being changed.
		std::string new_text_;
		//! Radio button callback data: sets the action for each radio button.
		const radio_param_t radio_params_[4] =
		{
			{ RENAME_FIELD, (int*)&action_ },
			{ DELETE_FIELD, (int*)&action_ },
			{ ADD_FIELD, (int*)&action_ },
			{ CHANGE_FIELD, (int*)&action_ }
		};
		// Widgets that need to be accessed
		Fl_Widget* w_field_name_;    //!< Choice for field name to rename old field.
		Fl_Widget* w_text_;          //!< Input for new text for changed field.
		Fl_Widget* w_enum_;          //!< Choicefor enumerated value for changed field.

	};

#endif
