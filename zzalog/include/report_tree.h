#ifndef __REPORT_TREE__
#define __REPORT_TREE__

#include "view.h"
#include "fields.h"

#include <string>
#include <vector>
#include <list>
#include <map>

#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>

using namespace std;



	// Report View filter
	enum report_filter_t {
		RF_NONE,                  // No display
		RF_ALL,                   // Display all items
		RF_ALL_CURRENT,           // Display all items for current call (Report View only)
		RF_EXTRACTED,             // Display items for extracted records
		RF_SELECTED,              // Display item for selected record
		RF_LAST                   // enum upper bound
	};

	// Report view items
	enum report_item_t {
		RI_CODE,                  // DXCC integer code
		RI_NICK,                  // DXCC prefix nickname
		RI_NAME,                  // DXCC name
		RI_CQ_ZONE,               // CQ Zone
		RI_ITU_ZONE,              // ITU Zone
		RI_ADIF,                  // ADIF Specification data
		RI_EXCEPTION = 8,         // Exception records
		RI_INVALID = 16,          // Invalid records
		RI_ZONE_EXC = 32,         // Zone exception records
		RI_LAST                   // enum upper bound
	};

	// Report category items - maximum 8-bit number
	enum report_cat_t {
		RC_DXCC = 1,                  // DXCC
		RC_PAS = 3,                   // DXCC plus primary adminstrative subdivision (State)
		RC_BAND = 4,                  // Band
		RC_MODE = 8,                  // Mode
		RC_CALL = 16,                 // Callsign
		RC_CUSTOM = 32,               // Custom - select ADIF field
		RC_EMPTY = 0                  // Level not used
	};

	// This class displays the log analysis report - it is a tree report
	class report_tree :
		public view, public Fl_Tree
	{
	public:
		// Constructor initialises Fl_Tree and view
		report_tree(int X, int Y, int W, int H, const char* label, field_ordering_t app);
		~report_tree();

		// inherited from view
		virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0);
		// Delete all items
		void delete_all();
		// Delete the tree
		void delete_tree();
		// Basic record list - list of record numbers
		typedef list<qso_num_t> record_list_t;
		// Basic entry for a map
		struct report_map_entry_t {
			// Depth of entry
			int entry_type;
			// what sort of entry
			report_cat_t entry_cat;
			// list of records in this entry
			record_list_t* record_list;
			// map of entries to display under this entry
			map<string, report_map_entry_t*>* next_entry;
			// Default constructor
			report_map_entry_t() {
				entry_type = 0;
				entry_cat = RC_DXCC;
				record_list = nullptr;
				next_entry = nullptr;
			}
		};
		// The map of entries
		typedef map<string, report_map_entry_t*> report_map_t;
		// methods
		// Add record details to a specific map entry
		void add_record(qso_num_t iRecord, report_map_entry_t* entry);
		// Copy the map to the tree control
		void copy_map_to_tree(report_map_t* pMap, Fl_Tree_Item* item, int& num_records, int& num_eqsl, int& num_lotw, int& num_card, int& num_dxcc, int &num_any);
		// Copy the list of records at a map entry to the tree control
		void copy_records_to_tree(record_list_t* pRecords, Fl_Tree_Item* item, int& num_records, int& num_eqsl, int& num_lotw, int& num_card, int& num_dxcc, int& num_any);
		// Create the map top-down
		void create_map();
		// Delete the map in a specific map entry
		void delete_map(report_map_entry_t* entry);
		// redraw the tree control
		void populate_tree(bool activate);
		// Update the status
		void update_status();
		// Select records
		void add_filter(report_filter_t filter);
		// Add Type
		void add_category(int level, report_cat_t category, string custom_field);
		// Change font
		void set_font(Fl_Font font, Fl_Fontsize size);

		// Callback - select item
		static void cb_tree_report(Fl_Widget* w, void* v);

		// attributes
	protected:
		// Top-level entry containing a map of all first-level entries
		report_map_entry_t map_;
		// map order - e.g. "DXCC","Band","Mode"
		vector<report_cat_t> map_order_;
		// Adjusted map order (including state)
		vector<report_cat_t> adj_order_;
		// Report type
		report_filter_t filter_;
		// number of current selected record
		qso_num_t selection_;
		// Add statesin break down of DXCC
		bool add_states_;
		// Font and size
		Fl_Font font_;
		Fl_Fontsize fontsize_;
		// Country tallies
		int entities_;
		int entities_eqsl_;
		int entities_lotw_;
		int entities_card_;
		int entities_dxcc_;
		int entities_any_;
		// Custom field name
		string custom_field_;

	};
#endif
