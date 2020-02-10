#ifndef __EXTRACT_DIALOG__
#define __EXTRACT_DIALOG__

#include "../zzalib/win_dialog.h"
#include "calendar.h"
#include "../zzalib/callback.h"
#include "search.h"

#include <string>

using namespace std;
using namespace zzalib;

namespace zzalog {

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

	protected:
		// Load previous criteria from settings
		void load_values();
		// Save criteria in settings
		void save_values();

		// The current criteria
		search_criteria_t* criteria_;
		// Calendar callback parameters - from date
		cal_cb_data_t from_params_;
		// Calendar callback parameters - to date
		cal_cb_data_t to_params_;

		// Radio button callback parameters - basic condition
		radio_param_t condition_params_[10] =
		{
			{ XC_DXCC, nullptr },
			{ XC_GEO, nullptr },
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
		// Labels for the condition radio buttons 
		const string condition_labels_[10] =
		{ "DXCC", "Geography", "CQ Zone", "ITU Zone", "Continent", "Square (2)", "Square (4)", "Callsign",
			"All", "Field" };
		// Labels for the combination mode radio buttons
		const string combination_labels_[3] = {
			"New", "And", "Or"
		};
		// Fail display widget
		Fl_Widget* fail_box_;
	};

}
#endif
