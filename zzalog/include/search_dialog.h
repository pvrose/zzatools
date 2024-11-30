#ifndef __SEARCH_DIALOG__
#define __SEARCH_DIALOG__

#include "win_dialog.h"
#include "callback.h"
#include "search.h"

#include <string>

using namespace std;

	// Labels for the comparison mode labels
	static const string comparator_labels_[7] = {
		"~", "/=", "<", "<=", "=", ">=", ">"
	};

	// This class provides a dialog to generate a search or extract condition
	class search_dialog :
		public win_dialog
	{
	public:
		search_dialog();
		virtual ~search_dialog();

		// get the extract criteria
		search_criteria_t* criteria();
		// set extract fail message
		void fail(string message);

		// callbacks 
		// OK button clicked
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// Cancel button clicked
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		// One of the condition buttons
		static void cb_bn_condx(Fl_Widget* w, void* v);
		// The field name choice
		static void cb_ch_field(Fl_Widget* w, void* v);


	protected:
		// Load previous criteria from settings
		void load_values();
		// Save criteria in settings
		void save_values();
		// Enable widgets
		void enable_widgets();

		// The current criteria
		search_criteria_t* criteria_;

		// Radio button callback parameters - basic condition
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
		// Radio button callback parameters - combination mode
		radio_param_t combination_params_[3] =
		{
			{ XM_NEW, nullptr },
			{ XM_AND, nullptr },
			{ XM_OR, nullptr }
		};
		// Radio button callback parameters - comparison mode
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
		// Labels for the condition radio buttons 
		const string condition_labels_[XC_MAXIMUM] =
		{ "DXCC", "CQ Zone", "ITU Zone", "Continent", "Square (2)", "Square (4)", "Callsign",
			"All", "Field" };
		// Labels for the combination mode radio buttons
		const string combination_labels_[3] = {
			"New", "And", "Or"
		};
		// Fail display widget
		Fl_Widget* fail_box_;
		// Field definitions widget
		Fl_Widget* field_name_;
		Fl_Widget* search_text_;
	};
#endif
