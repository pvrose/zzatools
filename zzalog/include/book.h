#ifndef __BOOK__
#define __BOOK__

#include "band.h"
#include "drawing.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>

using namespace std;

	class view;
	class adi_reader;
	class adx_reader;
	class adi_writer;
	class adx_writer;
	class record;
	class band_set;
	struct search_criteria_t;
	typedef vector<string> field_list;

	// extern spec_data* spec_data_;



	// File type
	enum adif_format_t {
		FT_NONE,             // no file loaded
		FT_ADI,              // .adi
		FT_ADX,              // .adx
		FT_MIXED             // both .adx and .adi
	};

	// Navigation type
	enum navigate_t : uchar {
		NV_FIRST,        // Go to the first record
		NV_LAST,         // Go to the last record
		NV_NEXT,         // Go to the next record
		NV_PREV          // Go to the previous record
	};


	// Update hint
	enum hint_t : uchar {
		HT_ALL,                   // invalidate all records in view - select record if >= 0
		HT_CHANGED,               // Invalidate only specified record
		HT_MINOR_CHANGE,          // Invalidate the record, but not location, band or mode
		HT_INSERTED,              // Record has been added - invalidate it and all after
		HT_DELETED,               // Record has been deleted - invalidate it and all after
		HT_SELECTED,              // Selection has been changed to this record - remove selection from existing and select this.
		HT_DUPE_DELETED,          // Record may have changed as the one after deleted- invalidate it and after
		HT_EXTRACTION,            // Extract conditions have changed - invalidate OT_EXTRACT and views displaying extracted data
		HT_IMPORT_QUERY,          // Import record cannot be processed without user intervention - 
		HT_IMPORT_QUERYNEW,       // Query whether mismatch is a new record
		HT_IMPORT_QUERYSWL,       // Query whether SWL report is valid
		HT_DUPE_QUERY,            // Query whether records are duplicates
		HT_FORMAT,                // Formats or Columns have changed (LOGVIEW and EXTRACTVIEW)
		HT_STARTING,              // Record is being created as HT_INSERTED but don't include it
		HT_NEW_DATA,              // New data has been loaded - action as HT_ALL but clears modified
		HT_NO_DATA,               // Log has been cleared
		HT_LOCATION,              // The home locations have changed
		HT_RESET_ORDER,           // Reset order as first to last
		HT_MEMORIES,              // Read mmemories from rig
		HT_START_CHANGED,         // The start date or time has changed
		HT_IGNORE,                // Ignore the change
		HT_INSERTED_NODXA,         // As HT_INSERTED but do not update DxAtlas
	};

	// Worked before categories
	enum worked_t : uchar {
		WK_ANY,                   // Any QSO
		WK_DXCC,                  // DXCC
		WK_GRID4,                 // 4-character gridsquares
		WK_CQZ,                   // CQ Zone
		WK_ITUZ,                  // ITU Zone
		WK_CONT,                  // Continent
		WK_PAS,                   // Prmary Admin: Subdiv: (eg US State)
	};

	// The records are kept in a container with size_t as index
	typedef size_t item_num_t;    // Position of item within this book
	typedef size_t qso_num_t;     // Position of item within book_ instance

	// This class is the container for the ADIF records. These are held in chronological order.
	// As well as standing alone it is used as a base class for extract_data and import_data
	class book : public vector<record*>
	{
	// Constructors and destructors
	public:
		// default constructor
		book(object_t type = OT_MAIN);
		// default destructor
		virtual ~book();

		// Public methods
	public:
		// Load data
		bool load_data(string filename);
		// Store data - only if modified unless "force"d. Can specify limited fields
		bool store_data(string filename = "", bool force = false, field_list* fields = nullptr);
		// Get the current selected record
		record* get_record();
		// Set the numbered record and optionally select it
		record* get_record(item_num_t item_num, bool set_selected);
		// Get the most recent record
		record* get_latest();
		// Delete the current record
		void delete_record(bool force);
		// Change the selected record (& update any necessary controls) - returns the new record number if the book gets reordered
		virtual item_num_t selection(item_num_t num_item, hint_t hint = HT_SELECTED, view* requester = nullptr, item_num_t num_other = 0);
		// Get the current selection
		item_num_t selection();
		// Insert a record in its chronological position 
		item_num_t insert_record(record* record);
		// Append a record at the end of the book
		item_num_t append_record(record* record);
		// add a header record
		void header(record* header);
		// return the header
		record* header();
		// Delete all records and tidy up
		void delete_contents(bool new_book);
		// Return count of records - less header
		item_num_t get_count();
		// Navigate the log 
		void navigate(navigate_t target);
		// Go to a date
		void go_date(string date);
		// get the position at which to chronologically insert a record
		item_num_t get_insert_point(record* record);
		// insert the record at specific position
		void insert_record_at(item_num_t pos_record, record* record);
		// get filename
		string filename(bool full = true);
		// match record
		bool match_record(record* record);
		// basic match - field comparison matches
		bool basic_match(record* record);
		// refine match - by date band and mode
		bool refine_match(record* record);
		// item matching - string
		bool match_string(string test, int comparator, string value);
		// item matching - integer
		bool match_int(string test, int comparator, string value);
		bool match_int(int test, int comparator, int value);
		// get book type
		object_t book_type();
		// set book type
		void book_type(object_t);
		// get used bands - worked type WK_ANY - all QSOs
		band_set* used_bands();
		band_set* used_bands(worked_t category, int32_t entity, string call);
		band_set* used_bands(worked_t category, string entity, string call);
		// get used modes 
		set<string>* used_modes();
		set<string>* used_modes(worked_t category, int32_t entity, string call);
		set<string>* used_modes(worked_t category, string entity, string call);
		// get used submodes
		set<string>* used_submodes();
		set<string>* used_submodes(worked_t category, int32_t entity, string call);
		set<string>* used_submodes(worked_t category, string entity, string call);
		// New record
		bool new_record();
		// Set new record
		void new_record(bool value);
		// Record is not dupe
		void accept_dupe();
		// Record is an exact dupe: use_dupe = true, use the dupe record else keep the original entry
		void reject_dupe(bool use_dupe);
		// Record is a dupe but we want to merge the two
		void merge_dupe();
		// Check duplicates
		void check_dupes(bool restart);
		// get the match query question
		string match_question();
		// Open editor for header comment
		void edit_header();
		// Remember record
		void remember_record();
		// Book has been modified
		bool been_modified();
		// Call back for closing edit window
		static void cb_close_edith(Fl_Widget* w, void* v);
		// Callback to close edit window without updating header
		static void cb_cancel_edith(Fl_Widget* w, void* v);
		// Call back to add default header
		static void cb_default_edith(Fl_Widget* w, void* v);
		// Callback to restore original header text
		static void cb_restore_edith(Fl_Widget* w, void* v);

		// methods to be overridden --
		// Add/replace the search/find criteria
		item_num_t search(search_criteria_t* criteria, bool reset_search);

		// return the real record number - base class returns it unchanged, 
		inline virtual qso_num_t record_number(item_num_t item_num) {
			return item_num;
		}
		// return the virtual record number - base class returns it unchanged
		inline virtual item_num_t item_number(qso_num_t record_num, bool nearest = false) {
			return record_num;
		}
		// Inhibit QSO u-pload
		void allow_upload(bool enable);

		// Protected methods
		item_num_t correct_record_position(item_num_t current_pos);
		// And all data
		void add_use_data(record* record);
		// Enable/Disable auto-save
		void enable_save(bool enable, const char* reason);
		// Save enabled
		bool enable_save();
		// Delete enabled
		bool delete_enabled();
		// Upload single QSO
		bool upload_qso(qso_num_t record);
		// enterring record
		bool enterring_record();

		// Find session start
		void set_session_start();
		// Get percentage comple of load or save
		double get_complete();
		// Get loading
		bool loading();
		bool storing();

		// Remove old macros
		void deprecate_macros(record* use_record);
		// Set the filename
		void set_filename(string filename);
		//// Get macro field
		//set<string> get_macro_fields(string name);
		// Is this record in the book
		bool has_record(record* qso);
		// Mark this record dirty
		void add_dirty_record(record* qso);
		// Mark this record clean
		void delete_dirty_record(record* qso);
		// Record is dirty
		bool is_dirty_record(record* qso);
		// Book is dirty
		bool is_dirty();

		// Protected attributes
	protected:
		// current selected item number
		item_num_t current_item_;
		// Test duplicate item
		item_num_t test_item_;
		// Possible duplicate item
		item_num_t dupe_item_;
		// Newest record number
		item_num_t newest_item_;
		// Book type
		object_t book_type_;
		// Save in progress
		bool save_in_progress_;
		// current filename 
		string filename_;
		// current input filestream
		ifstream input_;
		// file format
		adif_format_t format_;
		// inhibit views being updated when selection changes
		bool inhibit_view_update_;
		// New record being created
		bool new_record_;
		// header record
		record* header_;
		// Current find/extract criteria
		search_criteria_t* criteria_;
		// Most recent search result
		item_num_t last_search_result_;
		// used bands
		band_set used_bands_;
		// used modes
		set<string> used_modes_;
		// used submodes
		set<string> used_submodes_;
		// Rig options used
		set<string> used_rigs_;
		// Antenna options used
		set<string> used_antennas_;
		// Callsigns used
		set<string> used_callsigns_;
		// Ignore null macros
		bool ignore_null_;
		// Bands worked per DXCC
		map < string, map < worked_t, map < string, band_set > > > bands_;
		// mOdes worked per DXCC
		map < string, map < worked_t, map < string, set<string> > > > modes_;
		// submOdes worked per DXCC
		map < string, map < worked_t, map < string, set<string> > > > submodes_;
		// match query question
		string match_question_;
		// INhibit automatic save -
		bool inhibit_auto_save_;
		// Delete in progress
		bool delete_in_progress_;
		// Unmodified record
		record* old_record_;
		// Number duplicates removed
		int number_dupes_removed_;
		// Number of duplicates accepted
		int number_dupes_kept_;
		// Book has been modified
		bool been_modified_;
		// File loading
		bool main_loading_;
		// Ignore GRIDSQUARE shortening
		bool ignore_gridsquare_;
		// Save enabled level (enable downs it, disable ups it
		int save_level_;
		// Readers and writer
		adi_reader* adi_reader_;
		adx_reader* adx_reader_;
		adi_writer* adi_writer_;
		adx_writer* adx_writer_;
		// Upload allowed
		bool upload_allowed_;
		// Set of dirty QSOs
		set<record*> dirty_qsos_;
		// Need flag to indicate a record has been deleted
		bool deleted_record_;
	};

#endif
