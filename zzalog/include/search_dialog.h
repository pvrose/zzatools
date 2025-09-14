#ifndef __SEARCH_DIALOG__
#define __SEARCH_DIALOG__

#include "win_dialog.h"
#include "callback.h"
#include "search.h"

#include <string>



	//! Labels for the comparison mode labels : order matches enumeration search_comp_t.
	static const std::string comparator_labels_[7] = {
		"~", "/=", "<", "<=", "=", ">=", ">"
	};

	//! This class provides a dialog to generate a search or extract condition
	class search_dialog :
		public win_dialog
	{
	public:
		//! Constrcutor.
		search_dialog();
		//! Destructor.
		virtual ~search_dialog();

		//! Inherited from win_dialog to accept keyboard F1 to open userguide.
		virtual int handle(int event);

		//! Returns the extract criteria
		search_criteria_t* criteria();
		//! std::set extract fail message
		void fail(std::string message);

		// callbacks 
		//! Callback from "OK" button.
		static void cb_bn_ok(Fl_Widget* w, void* v);
		//! Callback from "Cancel" button
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		//! Callback from a condition button: \p v is search_comp_t indicating which button.
		static void cb_bn_condx(Fl_Widget* w, void* v);
		//! Callback from field name choice button.
		static void cb_ch_field(Fl_Widget* w, void* v);


	protected:
		//! Load previous criteria from settings
		void load_values();
		//! Save criteria in settings
		void save_values();
		//! Configure component widgets after data chane.
		void enable_widgets();

		//! The current criteria
		search_criteria_t* criteria_;

		//! Radio button callback parameters - basic condition
		radio_param_t condition_params_[XC_MAXIMUM] =
		{
			{ XC_DXCC, nullptr },
			{ XC_CQZ, nullptr },
			{ XC_ITUZ, nullptr },
			{ XC_CONT, nullptr },
			{ XC_SQ2, nullptr },
			{ XC_SQ4, nullptr },
			{ XC_CALL, nullptr },
			{ XC_UNFILTERED, nullptr },
			{ XC_FIELD, nullptr }
		};
		//! Radio button callback parameters - combination mode
		radio_param_t combination_params_[3] =
		{
			{ XM_NEW, nullptr },
			{ XM_AND, nullptr },
			{ XM_OR, nullptr }
		};
		//! Radio button callback parameters - comparison mode
		radio_param_t comparator_params_[7] =
		{
			{XP_REGEX, nullptr},
			{XP_NE, nullptr},
			{XP_LT, nullptr},
			{XP_LE, nullptr},
			{XP_EQ, nullptr},
			{XP_GE, nullptr},
			{XP_GT, nullptr},
		};
		//! Labels for the condition radio buttons 
		const std::string condition_labels_[XC_MAXIMUM] =
		{ "DXCC", "CQ Zone", "ITU Zone", "Continent", "Square (2)", "Square (4)", "Callsign",
			"All", "Field" };
		//! Labels for the combination mode radio buttons
		const std::string combination_labels_[3] = {
			"New", "And", "Or"
		};
		//! Output: Displays error message.
		Fl_Widget* fail_box_;
		//! Choice: Field name menu for "Field" condition.
		Fl_Widget* field_name_;
		//! Input: Field value - can be a menu of enumeration values or input for direct entry.
		Fl_Widget* search_text_;
	};
#endif
