#ifndef __MENU__
#define __MENU__

#include "book.h"
#include "report_tree.h"
#include "status.h"
#include "extract_data.h"

#include <vector>

#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>

namespace zzalog {

	// This class provides the menu bar and handles all the menu item callbacks
	class menu : public Fl_Menu_Bar
	{
	public:

		menu(int X, int Y, int W, int H, const char* label = 0);
		virtual ~menu();

		// File->New
		static void cb_mi_file_new(Fl_Widget* w, void* v);
		// File->Open
		static void cb_mi_file_open(Fl_Widget* w, void* v);
		// File->Save
		static void cb_mi_file_save(Fl_Widget* w, void* v);
		// File->SaveAs; Export->Save
		static void cb_mi_file_saveas(Fl_Widget* w, void* v);
		// File->Close
		static void cb_mi_file_close(Fl_Widget* w, void* v);
		// File->Auto Save
		static void cb_mi_file_auto(Fl_Widget* w, void* v);
		// File->Print
		static void cb_mi_file_print(Fl_Widget* w, void* v);
		// File->Backup
		static void cb_mi_file_backup(Fl_Widget* w, void* v);
		// Settings->any
		static void cb_mi_settings(Fl_Widget* w, void* v);
		// Navigate->First,Previous,Next,Last callbacks
		static void cb_mi_navigate(Fl_Widget* w, void* v);
		// Navigate->Date
		static void cb_mi_nav_date(Fl_Widget* w, void* v);
		// Navigate->Find->New/Next
		static void cb_mi_nav_find(Fl_Widget* w, void* v);
		// Log->Parse QSO
		static void cb_mi_parse_qso(Fl_Widget* w, void* v);
		// Log->Parse Log
		static void cb_mi_parse_log(Fl_Widget* w, void* v);
		// Log->Validate QSO
		static void cb_mi_valid8_qso(Fl_Widget* w, void* v);
		// Log->Validate Log
		static void cb_mi_valid8_log(Fl_Widget* w, void* v);
		// Log->Mode->Off-air,On-air,Import
		static void cb_mi_log_mode(Fl_Widget* w, void* v);
		// Log->New
		static void cb_mi_log_new(Fl_Widget* w, void* v);
		// Log->Save
		static void cb_mi_log_save(Fl_Widget* w, void* v);
		// Log->Delete/Cancel
		static void cb_mi_log_del(Fl_Widget* w, void* v);
		// Log->Bulk change
		static void cb_mi_log_bulk(Fl_Widget* w, void* v);
		// Log->Check Duplicates
		static void cb_mi_log_dupes(Fl_Widget* w, void* v);
		// Log->Scrathpad
		static void cb_mi_log_spad(Fl_Widget* w, void* v);
		// Log->New View->Main,Record,Scratchpad
		static void cb_mi_log_view(Fl_Widget* w, void* v);
		// Log->Change->Rig/Aerial/QTH
		static void cb_mi_oper_change(Fl_Widget* w, void* v);
		// Log->Set->Rig/Aerial/QTH
		static void cb_mi_oper_set(Fl_Widget* w, void* v);
		// Log->Edit Header
		static void cb_mi_log_edith(Fl_Widget* w, void* v);
		// Extract->Clear
		static void cb_mi_ext_clr(Fl_Widget* w, void* v);
		// Extract->Criteria
		static void cb_mi_ext_crit(Fl_Widget* w, void* v);
		// Extract->redo
		static void cb_mi_ext_redo(Fl_Widget* w, void* v);
		// Extract->display
		static void cb_mi_ext_disp(Fl_Widget* w, void* v);
		// Extract->eQSL/LotW/Card
		static void cb_mi_ext_qsl(Fl_Widget* w, void* v);
		// Extract->Upload
		static void cb_mi_ext_upload(Fl_Widget* w, void* v);
		// Extract->Print
		static void cb_mi_ext_print(Fl_Widget* w, void* v);
		// Import->File
		static void cb_mi_imp_file(Fl_Widget* w, void* v);
		// Import->Download->eQSL/LotW
		static void cb_mi_download(Fl_Widget* w, void* v);
		// Import->Merge
		static void cb_mi_imp_merge(Fl_Widget* w, void* v);
		// Import->Cancel
		static void cb_mi_imp_cancel(Fl_Widget* w, void* v);
		// Reference->Prefix->Clear/All/Extracted/Selected
		static void cb_mi_ref_filter(Fl_Widget* w, void* v);
		// Reference->Prefix->Items
		static void cb_mi_ref_items(Fl_Widget* w, void* v);
		// Reference->Prefix->Add details
		static void cb_mi_ref_details(Fl_Widget* w, void* v);
		// Reference->Reload data
		static void cb_mi_ref_reload(Fl_Widget* w, void* v);
		// Report->Clear/All/Extracted?Selected
		static void cb_mi_rep_filter(Fl_Widget* w, void* v);
		// Report->Levelx
		static void cb_mi_rep_level(Fl_Widget* w, void* v);
		// Information->QRZ.com
		static void cb_mi_info_qrz(Fl_Widget* w, void* v);
		// Information->Google maps
		static void cb_mi_info_map(Fl_Widget* w, void* v);
		// Information->QSO Web-site
		static void cb_mi_info_web(Fl_Widget* w, void* v);
		// Help->About
		static void cb_mi_help_abt(Fl_Widget* w, void* v);
		// Help->View Status
		static void cb_mi_help_view(Fl_Widget* w, void* v);
		// Help->Minimum status level
		static void cb_mi_help_level(Fl_Widget* w, void* v);
		// Help->Show Intl
		static void cb_mi_help_intl(Fl_Widget* w, void * v);

		// Set logging mode
		void logging(logging_mode_t mode);
		// Returns logging mode
		logging_mode_t logging();
		// Enable/disable menu
		void enable(bool active);
		// Set report mode
		void report_mode(vector<report_cat_t> mode, report_filter_t filter);
		// Set ststus level
		void status_level(status_t level);
		// Add the recent files to the menu
		void add_recent_files();
		// Update menu items - activeness
		void update_items();

	protected:
		// Get the browser form settings or if not ask user
		string get_browser();


	protected:
		// Current logging mode
		logging_mode_t logging_mode_;
		// search criteria - remembered
		search_criteria_t* criteria_;
		// Enabled
		bool active_enabled_;
		// Current logging view
		object_t editting_view_;
		// Current extraction
		extract_data::extract_mode_t qsl_type_;

	};

}
#endif