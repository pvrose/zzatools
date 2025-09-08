#ifndef __REPORT_TREE__
#define __REPORT_TREE__

#include "view.h"
#include "fields.h"

#include <string>
#include <vector>
#include <list>
#include <map>

#include <FL/Fl_Tree.H>
#include <FL/Enumerations.H>

using namespace std;

template <class T> class band_map;



	//! Report view filter
	enum report_filter_t {
		RF_NONE,                  //!< No display
		RF_ALL,                   //!< Display all items
		RF_ALL_CURRENT,           //!< Display all items for current call (Report View only)
		RF_EXTRACTED,             //!< Display items for extracted records
		RF_SELECTED,              //!< Display item for selected record
		RF_LAST                   
	};

	//! Report view items
	enum report_item_t {
		RI_CODE,                  //!< DXCC integer code
		RI_NICK,                  //!< DXCC prefix nickname
		RI_NAME,                  //!< DXCC name
		RI_CQ_ZONE,               //!< CQ Zone
		RI_ITU_ZONE,              //!< ITU Zone
		RI_ADIF,                  //!< ADIF Specification data
		RI_EXCEPTION = 8,         //!< Exception records
		RI_INVALID = 16,          //!< Invalid records
		RI_ZONE_EXC = 32,         //!< Zone exception records
		RI_LAST                   
	};

	//! Report category items - maximum 8-bit number
	enum report_cat_t : uchar {
		RC_DXCC = 1,                  //!< DXCC
		RC_PAS = 3,                   //!< DXCC plus primary adminstrative subdivision (State)
		RC_BAND = 4,                  //!< Band
		RC_MODE = 8,                  //!< Mode
		RC_CALL = 16,                 //!< Callsign
		RC_CUSTOM = 32,               //!< Custom - select ADIF field
		RC_EMPTY = 0                  //!< Level not used
	};

	//! This class displays the log analysis report - it is a tree report
	class report_tree :
		public view, public Fl_Tree
	{
	public:
		//! Constructor initialises Fl_Tree and view

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		//! \param app Used to configure view.
		report_tree(int X, int Y, int W, int H, const char* L, field_app_t app);
		//! Descriptor.
		~report_tree();

		//! Inherited from Fl_Tree::handle to allow keyboard F1 to open userguide.
		virtual int handle(int event);

		//! Inherited from view
		
		//! \param hint Indication of whatthe change is.
		//! \param record_num_1 Index of QSO record changed.
		//! \param record_num_2 Index of asociated QSO record.
		virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0);
		//! Delete all items
		void delete_all();
		//! Delete the tree
		void delete_tree();
		//! Basic record list - list of record numbers
		typedef list<qso_num_t> record_list_t;
		//! Basic entry for a map
		struct report_map_entry_t {
			//! Depth of entry
			int entry_type;
			//! list of records in this entry
			record_list_t* record_list;
			//! This will either be as above, or: base_map<report_map_entry_t*>* depending on the context.
			void* next_entry;
			//! Default constructor
			report_map_entry_t() {
				entry_type = 0;
				// entry_cat = RC_DXCC;
				record_list = nullptr;
				next_entry = nullptr;
			}
		};
		//! The map of entries
		typedef map<string, report_map_entry_t*> report_map_t;
		//! The map of entries if first level is a band.
		typedef band_map<report_map_entry_t*> report_band_map_t;
		// methods
		//! Add record details to a specific map entry
		
		//! \param iRecord index of QSO record.
		//! \param entry Entry to add record to.
		void add_record(qso_num_t iRecord, report_map_entry_t* entry);
		//! Copy the map to the tree control: \p type indicates level, \p item indicates item to hang, remainder of parameters receive record counts.
		void copy_map_to_tree(int type, void* pMap, Fl_Tree_Item* item, int& num_records, int& num_eqsl, int& num_lotw, int& num_card, int& num_qrz, int& num_dxcc, int &num_any);
		//! Copy the list of records at a map entry to the tree control
		void copy_records_to_tree(record_list_t* pRecords, Fl_Tree_Item* item, int& num_records, int& num_eqsl, int& num_lotw, int& num_card, int& num_qrz, int& num_dxcc, int& num_any);
		//! Create the map top-down
		void create_map();
		//! Delete the map in a specific map entry
		void delete_map(report_map_entry_t* entry);
		//! redraw the tree control
		void populate_tree(bool activate);
		//! Update the status
		void update_status();
		//! Select records
		void add_filter(report_filter_t filter);
		//! Add Type
		void add_category(int level, report_cat_t category, string custom_field);
		//! Change font
		void set_font(Fl_Font font, Fl_Fontsize size);

		//! Callback from clicking tree
		static void cb_tree_report(Fl_Widget* w, void* v);

		// attributes
	protected:
		//! Top-level entry containing a map of all first-level entries
		report_map_entry_t map_;
		//! map order - e.g. "DXCC","Band","Mode"
		vector<report_cat_t> map_order_;
		//! Adjusted map order (including state)
		vector<report_cat_t> adj_order_;
		//! Report type
		report_filter_t filter_;
		//! number of current selected record
		qso_num_t selection_;
		//! Add states in break down of DXCC
		bool add_states_;
		//! Font for drawing  tree
		Fl_Font font_;
		//! Font size for drawing tree.
		Fl_Fontsize fontsize_;
		// Country tallies
		int entities_;          //!< Number of DXCC entities
		int entities_eqsl_;     //!< Number of entities confirmed on eQSL
		int entities_lotw_;     //!< Number of entities confirmed on lotW
		int entities_card_;     //!< Number of entities confirmed by card
		int entities_dxcc_;     //!< Number of entities confirmed on LotW pr card
		int entities_qrz_;     //!< Number of entities confirmed on QRZ.com
		int entities_any_;     //!< Number of entities confirmed on any
		//! Custom field name
		string custom_field_;
		//! Station callsign used
		string station_call_;

	};
#endif
