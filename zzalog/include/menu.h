#ifndef __MENU__
#define __MENU__

#include "report_tree.h"

#include <vector>

#include <FL/Fl_Menu_Bar.H>

class book;
class status;
class extract_data;
struct search_criteria_t;


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
		// File->Print
		static void cb_mi_file_print(Fl_Widget* w, void* v);
		// File->Backup
		static void cb_mi_file_backup(Fl_Widget* w, void* v);
		// File->Restore
		static void cb_mi_file_restore(Fl_Widget* w, void* v);
		// Settings->any
		static void cb_mi_settings(Fl_Widget* w, void* v);
		// Windows->all
		static void cb_mi_windows_all(Fl_Widget* w, void* v);
		// Windows->others
		static void cb_mi_windows(Fl_Widget* w, void* v);
		// Navigate->First,Previous,Next,Last callbacks
		static void cb_mi_navigate(Fl_Widget* w, void* v);
		// Navigate->Date
		static void cb_mi_nav_date(Fl_Widget* w, void* v);
		// Navigate->Find->New/Next
		static void cb_mi_nav_find(Fl_Widget* w, void* v);
		// Navigate->Record
		static void cb_mi_nav_recnum(Fl_Widget* w, void* v);
		// Log->Parse QSO
		static void cb_mi_parse_qso(Fl_Widget* w, void* v);
		// Log->Unparse QSO
		static void cb_mi_unparse_qso(Fl_Widget* w, void* v);
		// Log->Reparse QSO
		static void cb_mi_reparse_qso(Fl_Widget* w, void* v);
		// Log->Parse Log
		static void cb_mi_parse_log(Fl_Widget* w, void* v);
		// Log->Validate QSO
		static void cb_mi_valid8_qso(Fl_Widget* w, void* v);
		// Log->Validate Log
		static void cb_mi_valid8_log(Fl_Widget* w, void* v);
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
		// Log->Retime QSO
		static void cb_mi_log_retime(Fl_Widget* w, void* v);
		// Log->Start/Stop Session
		static void cb_mi_log_start(Fl_Widget* w, void* v);
		// Log->Edit Header
		static void cb_mi_log_edith(Fl_Widget* w, void* v);
		// Log->Suspend save
		static void cb_mi_log_ssave(Fl_Widget* w, void* v);
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
		// Extract->e-Mail
		static void cb_mi_ext_email(Fl_Widget* w, void* v);
		// Extract->Mark sent
		static void cb_mi_ext_mark(Fl_Widget* w, void* v);
		// Extract->Special->
		static void cb_mi_ext_special(Fl_Widget* w, void* v);
		// Extract->Special->No card image
		static void cb_mi_ext_no_image(Fl_Widget* w, void* v);
		// Extract->Download Images
		static void cb_mi_ext_dl_images(Fl_Widget* w, void* v);
		// Import->File
		static void cb_mi_imp_file(Fl_Widget* w, void* v);
		// Import->Download->eQSL/LotW
		static void cb_mi_download(Fl_Widget* w, void* v);
		// Import->Merge
		static void cb_mi_imp_merge(Fl_Widget* w, void* v);
		// Import->Cancel
		static void cb_mi_imp_cancel(Fl_Widget* w, void* v);
		// Import->WSJTX UDP
		static void cb_mi_imp_wsjtx(Fl_Widget* w, void* v);
		// Import->Clipboard
		static void cb_mi_imp_clipb(Fl_Widget* w, void* v);
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
		// Help->USer Guide
		static void cb_mi_help_ug(Fl_Widget* w, void* v);

		// Enable/disable menu
		void enable(bool active);
		// Set report mode
		void report_mode(vector<report_cat_t> mode, report_filter_t filter);
		// Set append status
		void append_file(bool append);
		// Add the recent files to the menu
		void add_recent_files();
		// Update menu items - activeness
		void update_items();
		// Get the browser form settings or if not ask user
		string get_browser();
		// Update windows menu items
		void update_windows_items();
		// Add userdate to windows items
		void add_windows_items();
		// Update QSL related items
		void update_qsl_items();

	protected:
		// search criteria - remembered
		search_criteria_t* criteria_;
		// Enabled
		bool active_enabled_;

	};
#endif
