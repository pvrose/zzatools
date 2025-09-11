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

	//! ADIF File format.
	enum adif_format_t {
		FT_NONE,             //!< no file loaded
		FT_ADI,              //!< file loaded from a file in .adi format
		FT_ADX,              //!< file loaded from a file in .adx format
		FT_MIXED             //!< file loaded from files in both .adx and .adi formats.
	};

	//! Type used in navigation commands.
	enum navigate_t : uchar {
		NV_FIRST,        //!< Go to the first record.
		NV_LAST,         //!< Go to the last record.
		NV_NEXT,         //!< Go to the next record.
		NV_PREV          //!< Go to the previous record.
	};

	//! When a record has been updated, this provides the raeson to allow any
	//! display widget to decide how to redraw itself.
	enum hint_t : uchar {
		HT_ALL,                   //!< Invalidate all records in view - select record if >= 0.
		HT_CHANGED,               //!< Invalidate only specified record.
		HT_MINOR_CHANGE,          //!< Invalidate the record, but not location, band or mode.
		HT_INSERTED,              //!< Record has been added - invalidate it and all after.
		HT_DELETED,               //!< Record has been deleted - invalidate it and all after.
		HT_SELECTED,              //!< Selection has been changed to this record - remove selection from existing and select this.
		HT_DUPE_DELETED,          //!< Record may have changed as the one after deleted- invalidate it and after.
		HT_EXTRACTION,            //!< Extract conditions have changed - invalidate OT_EXTRACT and views displaying extracted data.
		HT_IMPORT_QUERY,          //!< Import record cannot be processed without user intervention.
		HT_IMPORT_QUERYNEW,       //!< Query whether mismatch is a new record.
		HT_IMPORT_QUERYSWL,       //!< Query whether SWL report is valid.
		HT_DUPE_QUERY,            //!< Query whether records are duplicates.
		HT_FORMAT,                //!< Formats or Columns have changed (LOGVIEW and EXTRACTVIEW).
		HT_STARTING,              //!< Record is being created as HT_INSERTED but don't include it.
		HT_NEW_DATA,              //!< New data has been loaded - action as HT_ALL but clears modified.
		HT_NO_DATA,               //!< Log has been cleared.
		HT_LOCATION,              //!< The home locations have changed.
		HT_RESET_ORDER,           //!< Reset order as first to last.
		HT_MEMORIES,              //!< Read mmemories from rig.
		HT_START_CHANGED,         //!< The start date or time has changed.
		HT_IGNORE,                //!< Ignore the change.
		HT_INSERTED_NODXA,        //!< As HT_INSERTED but do not update DxAtlas.
	};

	//! Worked before categories: used when checking any previous contacts.
	enum worked_t : uchar {
		WK_ANY,                   //!< Any QSO.
		WK_DXCC,                  //!< DXCC.
		WK_GRID4,                 //!< 4-character gridsquares.
		WK_CQZ,                   //!< CQ Zone.
		WK_ITUZ,                  //!< ITU Zone.
		WK_CONT,                  //!< Continent.
		WK_PAS,                   //!< Prmary Admin: Subdiv: (eg US State).
	};

	// The records are kept in a container with size_t as index
	typedef size_t item_num_t;    //!< Position of item within this book.
	typedef size_t qso_num_t;     //!< Position of item within book_ instance.

	//! This class is the container for the ADIF records. 

	//! These are held in chronological order.
	//! As well as standing alone it is used as a base class for extract_data and import_data
	class book : public vector<record*>
	{
	// Constructors and destructors
	public:
		//! default constructor.
		
		//! \param type used to indicate whether this is the main logbook, an extraction
		//! or records waiting to be imported.
		book(object_t type = OT_MAIN);
		//! default destructor
		virtual ~book();

		// Public methods
	public:
		//! Load data from file \p filename.
		
		//! \param filename location of data to load with full path.
		bool load_data(string filename);
		//! Store data.
		
		//! \param filename location of data to be stored. Defaults to file from which the data was loaded.
		//! \param force store data even if it is unchanged. Defaults to false.
		//! \param fields store only the specified fields. Defalts to all fields.
		bool store_data(string filename = "", bool force = false, field_list* fields = nullptr);
		//! Get the current selected record
		record* get_record();
		//! Get the numbered record and optionally select it.
		
		//! \param item_num the index within this array of records.
		//! \param set_selected select the requested record.
		//! \return the indexed record. 
		record* get_record(item_num_t item_num, bool set_selected);
		//! Get the most recent record
		
		//! \return the most recent record.
		record* get_latest();
		//! Delete the current record.
		
		//! When deleting the record, reconfigures all the linked data items.
		//! \param force byasses checks that significant data will be lost.
		void delete_record(bool force);
		//! Change the selected record.
		
		//! Notifies all components of ZZALOG.
		//! \param num_item the index within this set of records.
		//! \param hint indicates how all components should treat the update.
		//! \param requester indicates which component changed the record - to avoid indefinite loops.
		//! \param num_other index of any other relevant record - say in comparison checks.
		//! \return the index of the record as may have been changed.
		virtual item_num_t selection(item_num_t num_item, hint_t hint = HT_SELECTED, view* requester = nullptr, item_num_t num_other = 0);
		//! Get the index of the current selected record.
		
		//! \return index within this set of records of the current selection.
		item_num_t selection();
		//! Insert a record in its chronological position.
		
		//! \param record the record to be inserted.
		//! \return the index of the record after insertion.
		item_num_t insert_record(record* record);
		//! Append a record at the end of the book.
		
		//! \param record the record to be appended.
		//! \return the index of the record after being added.
		item_num_t append_record(record* record);
		//! Add a header record.
		
		//! \param header the header record.
		void header(record* header);
		//! Get the header
		
		//! \return the header record.
		record* header();
		//! Delete all records and tidy up.
		
		//! Delete all the records and resets all the link data.
		//! \param new_book re-initialises the link data.
		void delete_contents(bool new_book);
		//! Return count of QSO records.
		
		//! \return the number of QSO records, excluding the header.
		item_num_t get_count();
		//! Navigate the log.
		
		//! \param target select the record according to navigate_t.
		void navigate(navigate_t target);
		//! Go to a date.
		
		//! \param date Select the first record with QSO_DATE equal date. Format YYYYMMDD.
		void go_date(string date);
		//! Get the position at which to insert a record.
		
		//! Records are inserted where they would fit in time order.
		//! \param record the QSO record to insert
		//! \return the index where the record would be inserted.
		item_num_t get_insert_point(record* record);
		//! Insert the record at specific position.
		
		//! \param pos_record the index at which to insert the record.
		//! \param record the QSO record to be inserted.
		void insert_record_at(item_num_t pos_record, record* record);
		//! Get filename
	
		//! \param full returns the filename including the full path (as stored),
		//! otherwise returns only the filename within the directory.
		//! \return filename as requested by /p full.
		string filename(bool full = true);
		//! Match record.
		
		//! Returns whether a record matches search criteria.
		//! \param record QSO record to match.
		//! \return true if record matches the criteria, false if not.
		bool match_record(record* record);
		//! Basic match.
		
		//! Returns whether the record matches the field comparison criteria.
		//! \param record QSO record to match.
		//! \return true if the field comparison check agrees, false if not.
		bool basic_match(record* record);
		//! Refine match.
		
		//! Returns whether the record matches the additional refinement criteria:
		//! - Band matches.
		//! - Mode matches.
		//! - Date within sleected range.
		//! - Station callsign matches.
		//! - QSL checks.
		//! \param record QSO record to match.
		//! \return true if the addition criteria match, false if not.
		bool refine_match(record* record);
		//! item matching - string value.
		
		//! Returns whether the \p value matches the \p test according to the \p comparator.
		//! \param test the value against which the field is being compared.
		//! \param value the value from the QSO record.
		//! \param comparator search_comp_t operator for the comparison.
		//! \return true if the values match, false if not.
		bool match_string(string test, int comparator, string value);
		//! Item matching - integer value of string items.
		
		//! \see match_string.
		bool match_int(string test, int comparator, string value);
		//! Item matching - integer values.
		
		//! \see match_string.
		bool match_int(int test, int comparator, int value);
		//! Get book type.
		
		//! \return the object_t type of the book.
		object_t book_type();
		//! Set book type.
		
		//! \param type sets the object_t type of the book.
		void book_type(object_t type);
		//! Get bands used in logbook - worked_t = WK_ANY - all QSOs
		band_set* used_bands();
		//! Get bands used in logbook.
		
		//! Returns bands worked that match all the below condiitons.
		//! \param category get bands worked in worked_t category.
		//! \param entity get bands worked for DXCC identification number.
		//! \param call get bands worked using specific station callsign.
		//! \return the set of bands worked in the logbook subject to these conditions.
		band_set* used_bands(worked_t category, int32_t entity, string call);
		//! Get bands used in logbook.

		//! Returns bands worked that match all the below condiitons.
		//! \param category get bands worked in worked_t category.
		//! \param entity get bands worked for DXCC nickname.
		//! \param call get bands worked using specific station callsign.
		//! \return the set of bands worked in the logbook subject to these conditions.
		band_set* used_bands(worked_t category, string entity, string call);
		//! Get modes used in logbook - worked_t = WK_ANY - all QSOs
		set<string>* used_modes();
		//! Get modes used in logbook.

		//! Returns modes worked that match all the below condiitons.
		//! \param category get modes worked in worked_t category.
		//! \param entity get modes worked for DXCC identification number.
		//! \param call get modes worked using specific station callsign.
		//! \return the set of modes worked in the logbook subject to these conditions.
		set<string>* used_modes(worked_t category, int32_t entity, string call);
		//! Get modes used in logbook.

		//! Returns modes worked that match all the below condiitons.
		//! \param category get modes worked in worked_t category.
		//! \param entity get modes worked for DXCC nickname.
		//! \param call get modes worked using specific station callsign.
		//! \return the set of modes worked in the logbook subject to these conditions.
		set<string>* used_modes(worked_t category, string entity, string call);
		//! Get submodes used in logbook - worked_t = WK_ANY - all QSOs
		set<string>* used_submodes();
		//! Get submodes used in logbook.

		//! Returns submodes worked that match all the below condiitons.
		//! \param category get submodes worked in worked_t category.
		//! \param entity get submodes worked for DXCC identification number.
		//! \param call get submodes worked using specific station callsign.
		//! \return the set of submodes worked in the logbook subject to these conditions.
		set<string>* used_submodes(worked_t category, int32_t entity, string call);
		//! Get submodes used in logbook.

		//! Returns submodes worked that match all the below condiitons.
		//! \param category get submodes worked in worked_t category.
		//! \param entity get submodes worked for DXCC nickname.
		//! \param call get submodes worked using specific station callsign.
		//! \return the set of submodes worked in the logbook subject to these conditions.
		set<string>* used_submodes(worked_t category, string entity, string call);
		//! New record.
		
		//! \return a new record is being added to the book.
		bool new_record();
		//! Set new record.
		
		//! \param value set whether the current record is a new record or not.
		void new_record(bool value);
		//! Mark that the record is not a dupe. Used after a duplicate check.
		void accept_dupe();
		//! Record is an exact duplicate.
		
		//! \param use_dupe keep the record in the logbook, otherwise delete it.
		void reject_dupe(bool use_dupe);
		//! Record is a duplicate. Merge data between the two records.
		void merge_dupe();
		//! Check duplicates.
		
		//! \param restart resume the current duplicate check otherwise start a new one.
		void check_dupes(bool restart);
		//! Get the match query question.
		
		//! \return the question text to display to the user in a match or duplicate check.
		string match_question();
		//! Open a text editor for header comment;
		void edit_header();
		//! Take a copy of the current record.
		
		//! This is done prior to any match or duplicate check to enable any changes to be regressed.
		void remember_record();
		//! Is the book modified?
		
		//! \return true if any record has been meodified and not written back to filestore - is "dirty".
		bool been_modified();
		//! Callback for closing header comment edit window: saves the edit.
		static void cb_close_edith(Fl_Widget* w, void* v);
		//! Callback from closing header comment edit window using "Cancel" button.
		static void cb_cancel_edith(Fl_Widget* w, void* v);
		//! Callback from "Default" button on header comment edit window: 
		
		//! uses default value for header comment.
		static void cb_default_edith(Fl_Widget* w, void* v);
		//! Callback frim "Restore" button on header comment edit window:
		
		//! restoress original comment.
		static void cb_restore_edith(Fl_Widget* w, void* v);

		// methods to be overridden --
		//! Get record that matches search criteria.

		//! /param criteria the search criteria.
		//! /param reset_search restart search at the beginning, otherwise resume the search.
		//! /return index of matching record within this set of records.
		item_num_t search(search_criteria_t* criteria, bool reset_search);

		//! Convert the index in this set to the index in the logbook.
		
		//! \note in the logbook instance these indices are the same number.
		//! \param item_num index of record in this set of records.
		//! \return index of record in the logbbok.
		inline virtual qso_num_t record_number(item_num_t item_num) {
			return item_num;
		}
		//! Convert the index in the logbook to the index in this set of records.
		
		//! \note in the logbook instance these indices are the same number.
		//! \param record_num index in the logbook.
		//! \param nearest return a value that's closest if not in this set of records.
		//! \return index of record in this set of numbers. returns -1 if no match.
		inline virtual item_num_t item_number(qso_num_t record_num, bool nearest = false) {
			return record_num;
		}
		//! Inhibit QSO upload.
		
		//! \param enable if true allows QSOs to be uploaded to the QSL server sites supported by ZZALOG.
		//! If false disables it.
		void allow_upload(bool enable);

		//! Correct position of record within the logbook.
		
		//! If a record has its start time changed then the logbook will no longer
		//! be ordered chronologically.
		//! \param current_pos current index of the QSO record within this set of records.
		item_num_t correct_record_position(item_num_t current_pos);
		//! Add the usage information from this record
		
		//! Process the QSO record and update the usage data for band, mode, submode,
		//! rig, antenna and station callsign.
		//! \param record QSO record to be processed.
		void add_use_data(record* record);
		//! Control auto-save
		
		//! The normal behaviour of ZZALOG is to save the logbook when the user has finished
		//! a QSO or finished editing it. This enables the software to override this to 
		//! prevent excessive saving during intensive editing or when the user disables 
		//! the feature. 
		//! \note this is implemented as a stack of enables. only when the number
		//! of enables equals the number of disables is the feature truly enabled.
		//! \param enable if true allows auto-save behaviour, if false disable it.
		//! \param reason adds this text to the message recorded in banner.
		void enable_save(bool enable, const char* reason);
		//! Is auto-save enable?
		
		//! \return true if auto-save is enabled, false if it is not.
		bool enable_save();
		//! Is delete record enabled?
		
		//! \return true if a record can be deleted, false if it cannot. A record cannot
		//! be deleted if another record is already being deleted.
		bool delete_enabled();
		//! Upload the current QSO.
		
		//! \param record upload the QSO record to the supported QSL sites. This feature
		//! can be disabled globally or on a per-site basis which is checked within this
		//! method.
		//! \return true if all the sites have been uploaded to, false if one or more
		//! have not.
		bool upload_qso(qso_num_t record);
		//! Is a record being enteredor edited?
		
		//! \return true if a new record is being entered or the current selected
		//! record is dirty (differs from the record in filestore). False if neither
		//! condition is met.
		bool enterring_record();

		//! Find session start
		
		//! Finds the position in the logbook where there is a gap in operating greater than
		//! a preset period. Sets the global session_start_ to the timestamp of this QSO record.
		void set_session_start();
		//! Get fraction comple of load or save.
		
		//! \return the fraction of bytes read (for load) or records written (for save).
		double get_complete();
		//! Get loading.
		
		//! \return true if the logbook is being loaded from filestore, false if it is not.
		bool loading();
		//! Get storing.
		
		//! \return true if the logbook is being stored to filestore, false if it is not.
		bool storing();

		//! Remove old macros.
		
		//! Remove no longer supported ZZALOG application specific ADIF fields, used
		//! as macros by ZZALOG.
		void deprecate_macros(record* use_record);
		//! Set the filename.
		
		//! \param filename if the logbook is empty set this as the new filename and
		//! propagate to other components of ZZALOG.
		void set_filename(string filename);
		//! Is this record in the book?
		
		//! \param qso QSO record to check in the log.
		//! \return true if the QSO is in the logbook.
		bool has_record(record* qso);
		//! Mark this record dirty.
		
		//! A dirty record is one which has been modified and not yet written to filestore.
		//! \param qso QSO record that has been modified. Add this to the list of "dirty"
		//! records.
		//! \param reason Add reason for mrking ot dirty (debug feature).
		void add_dirty_record(record* qso, string reason);
		//! Mark this record clean.
		
		//! A clean record is one that has the same contents as its version in filestore.
		//! \param qso QSO record has been written to store. Remove this from the list od
		//! "dirty" records.
		void delete_dirty_record(record* qso);
		//! Is the record dirty?
		
		//! \param qso QSO record to check in list of "dirty" records.
		//! \return true if the record is in the list , false if it is not.
		bool is_dirty_record(record* qso);
		//! Is the book dirty?
		
		//! \return true if the "dirty" list is not empty, false if it is.
		bool is_dirty();

		// Protected attributes
	protected:
		//! Index of the current selected QSO in this set of QSO records.
		item_num_t current_item_;
		//! Index of a QSO record to be tested whether any duplicates exist.
		item_num_t test_item_;
		//! Index of a QSO record that mey be a duplicate of test_item_.
		item_num_t dupe_item_;
		//! Book usage type: one of OT_MAIN, OT_EXTRACT or OT_IMPORT.
		object_t book_type_;
		//! Save in progress: used to inhibit further saves.
		bool save_in_progress_;
		//! Current filename.
		string filename_;
		//! Current input filestream.
		ifstream input_;
		//! File format.
		adif_format_t format_;
		//! inhibit views being updated when selection changes. This is set to prevent
		//! unnecessary redrawing during an atomic sequence of updates.
		bool inhibit_view_update_;
		//! New record being created.
		bool new_record_;
		//! The header record.
		record* header_;
		//! Current find/extract criteria.
		search_criteria_t* criteria_;
		//! The index of the most recent search result.
		item_num_t last_search_result_;
		//! The set of bands logged in records within this set of QSO records.
		band_set used_bands_;
		//! The set of modes logged in records within this set of QSO records.
		set<string> used_modes_;
		//! The set of submodes logged in records within this set of QSO records.
		set<string> used_submodes_;
		//! The set of rigs logged in records within this set of QSO records.
		set<string> used_rigs_;
		//! The set of antennas logged in records within this set of QSO records.
		set<string> used_antennas_;
		//! The set of station callsigns logged in records within this set of QSO records.
		set<string> used_callsigns_;
		//! Multi-dimensional map indicating bands used:
		
		//! - Outer map: Mapped to DXCC entity.
		//! - Middle map: Mapped to worked_t type.
		//! - Inner map: Mapped to station callsign used.
		map < string, map < worked_t, map < string, band_set > > > bands_;
		//! Multi-dimensional map indicating modes used:

		//! - Outer map: Mapped to DXCC entity.
		//! - Middle map: Mapped to worked_t type.
		//! - Inner map: Mapped to station callsign used.
		map < string, map < worked_t, map < string, set<string> > > > modes_;
		//! Multi-dimensional map indicating submodes used:

		//! - Outer map: Mapped to DXCC entity.
		//! - Middle map: Mapped to worked_t type.
		//! - Inner map: Mapped to station callsign used.
		map < string, map < worked_t, map < string, set<string> > > > submodes_;
		//! match query question.
		string match_question_;
		//! Global inhibit to auto-save feature.
		bool inhibit_auto_save_;
		//! A QSO record delete is in progress.
		bool delete_in_progress_;
		//! A copy of the record currently being edited in a query or duplicate check.
		record* old_record_;
		//! Number of records removed during a duplicate check.
		int number_dupes_removed_;
		//! Number of possible duplicate records not deleted.
		int number_dupes_kept_;
		//! Flag that indicates that the book has been modified
		
		//! \todo Check whether this can be replaced using a function of dirty_qsos_?
		bool been_modified_;
		//! File loading
		
		//! \todo Is this still required. It is used, but for what.
		bool main_loading_;
		//! Save enabled level.
		
		//! This is incremented every time enable_save(false) is called and
		//! decremented every time enable_save(true) is called.
		int save_level_;
		// Readers and writer
		adi_reader* adi_reader_;  //!< Component used to read a .adi format file.
		adx_reader* adx_reader_;  //!< Component used to read a .adx format file.
		adi_writer* adi_writer_;  //!< Component used to write a .adi format file.
		adx_writer* adx_writer_;  //!< Component used to write a .adx format file.
		//! Flag set when uploads to QSL sites is allowed.
		bool upload_allowed_;
		//! The set of QSO records that have been declared "dirty". Their contents
		//! differ from the equivalent records in filestore.
		set<record*> dirty_qsos_;
		//! Flag set to indicate that the book is dirty after a record has been deleted.
		bool deleted_record_;
	};

#endif
