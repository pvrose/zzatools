#ifndef __CORR_DIALOG__
#define __CORR_DIALOG__

#include "win_dialog.h"

#include <string>
#include <set>



class record;
class Fl_Widget;


	//! This class implements a dialog to allow user correction in response to valdation errors
	class corr_dialog : public win_dialog
	{
	public:
		//! Constructor.
		
		//! \param record QSO record being validated.
		//! \param field The field of the QSO record - rewriteable.
		//! \param message The reason for the correcton request.
		corr_dialog(record* record, const std::string& field, const std::string& message);
		//! Destructor.
		virtual ~corr_dialog();

		//! Returns the correction message
		std::string correction_message();

	protected:
		//! Callback on clicking the OK button.
		
		//! Implementthe changes requested in the dialog.
		static void cb_bn_ok(Fl_Widget* w, void* v);
		//! Callback on clicking the "Cancel" button.
		
		//! Cancels this request, but continue validation.
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		//! Callback on clicking "Quit" button.
		
		//! Cancels this request and quits the validation process.
		static void cb_bn_quit(Fl_Widget* w, void* v);

		// attributes

		//! Change the value.
		bool change_value_;
		//! Change the field name.
		bool change_field_;
		//! Add a field.
		bool add_field_;
		//! Name to change field to.
		std::string change_field_name_;
		//! Name of field to add.
		std::string add_field_name_;
		//! New data for field.
		std::string change_value_data_;
		//! Data for an added field.
		std::string add_value_data_;

		//! The QSO record to be updated
		record* record_;
		//! The name if the field being queried
		std::string query_field_;
		//! Waiting for OK or CANCEL to be clicked.
		bool pending_button_;
		//! Button clicked.
		button_t button_;
		//! Message describing validation query.
		std::string correction_message_;

		//! widgets that need to be cleared.
		std::set<Fl_Widget*> field_choices_;

	};
#endif