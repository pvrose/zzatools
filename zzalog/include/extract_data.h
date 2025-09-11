#ifndef __EXTRACT_DATA__
#define __EXTRACT_DATA__

#include "book.h"
#include "search.h"

#include <vector>
#include <list>

using namespace std;


	//! This class is a container of records that have been extracted from the main log. 
	
	//! This inheritance provides
	// additional features required for this. As only pointers to the records are kept in the main log, this log
	// contains pointers to these records and not copies of the records themselves.
	class extract_data :
		public book

	{
	public:
		//! Reason for the extract
		enum extract_mode_t : uchar {
			NONE,        //!< empty extract
			SEARCH,      //!< search results
			EQSL,        //!< upload to eQSL.cc
			LOTW,        //!< upload to arrl.org/LotW
			QRZCOM,      //!< upload to qrz.com
			CARD,        //!< printing cards.
			EMAIL,       //!< Generate PNG file and send as attachment to e-mail
			CLUBLOG,     //!< upload to ClubLog
			NO_NAME,     //!< Special search for empty NAME item
			NO_QTH,      //!< Special search for empty QTH item
			LOCATOR,     //!< Special seatch for non-6 or 8 character gridsquare
			NO_EQSL_CARD,//!< Have eQSL but no card image
			ALL,         //!< Upload to all sites
			DUMMY        //!< end of enumeration
		};

		//! Constructor.
		extract_data();
		//! Destructor.
		virtual ~extract_data();

		//! Add/replace the search criteria
		
		//! \param criteria Search criteria.
		//! \param mode Extract reason defaults to SEARCH.
		//! \return number of records extracted. -1 if search is not valid.
		int criteria(search_criteria_t criteria, extract_mode_t mode = SEARCH);
		//! Reextract data as things may have changed
		void reextract();
		//! Clear the serach criteria, \p redraw = true causes ZZALOG to be redrawn.
		void clear_criteria(bool redraw = true);
		//! Extract for specific criteria defined in \p server - used for QSL uploads.
		void extract_qsl(extract_mode_t server);
		//! Upload data - on previously saved server.
		void upload();
		//! Extract for specific criteria - all records with \p callsign.
		void extract_call(string callsign);
		//! Add record with index \p record_num in full logbook to the extracted book.
		void add_record(qso_num_t record_num);
		//! Sort records by \p field_name (timestamp if empty string), in reverse order if \p reverse is set.
		void sort_records(string field_name, bool reverse);
		//! Special extract (\p for reason = NO_NAME, NO_QTH, LOCATOR).
		void extract_special(extract_mode_t reason);
		//! Special extract for QSO records matched with eQSL.cc but have no image.
		void extract_no_image();
		//! Extract by field only.
		
		//! \param field_name Field to compare value against.
		//! \param value Value to compare.
		//! \param and_search True indicates a further filtering.
		//! \param start Start date for search.
		//! \param endd End date for search.
		void extract_field(string field_name, string value, bool and_search, string start = "", string endd = "");
		//! Sort records in chronological order.
		void correct_record_order();
		//! Returns true if upload in progress, false if not.
		bool upload_in_progress();
		//! Returns extract mode.
		extract_mode_t use_mode();
		//! Set ectract \p mode.
		void use_mode(extract_mode_t mode);
		//! Check and add record: \p record_num is index in full log. 
		void check_add_record(qso_num_t record_num);

		//! Map index \p record_num in full map to the last index in extracted data.
		void map_record(qso_num_t record_num);

		//! Return the index in the full log representing the index in this log
		virtual qso_num_t record_number(item_num_t item);
		//! Return the index in this log representing the index in the full log.
		
		//! If the record in the full log is not in this return -1 unless \p
		//! nearest is true when the nearest record in this log is used.
		virtual item_num_t item_number(qso_num_t record, bool nearest = false);
		//! Change the selected record (& update any necessary controls)
		
		//! Sets the selected index in this log then selects the corresponding index in the full log. 
		virtual item_num_t selection(item_num_t num_record, hint_t hint = HT_SELECTED, view* requester = nullptr, qso_num_t num_other = 0);

	protected:

		//! Structure to support a tree sort.
		struct sort_node {
			qso_num_t qso_num;    //!< index in full log.
			sort_node* parent;    //!< parent to this node
			sort_node* left;      //!< node to the left of this node.
			sort_node* right;     //!< node to the right of this node.
			//! Default constructor.
			sort_node(qso_num_t n, sort_node* p) {
				qso_num = n;
				parent = p;
				left = nullptr;
				right = nullptr;
			}
		};

		//! Add contents of node to this log, return the number of nodes picked
		int pick_node(sort_node* node);

		//! Extract records for the criteria
		void extract_records();
		//! Generate description extract criterion for use in the header comment
		string comment();
		//! Short form comment for status log
		string short_comment();
		//! Swap two records.
		
		//! Check \p qso is within the extraction criteria
		bool meets_criteria(record* qso);

		//! Returns true if \p field in \p lhs QSO is less than \p rhs QSO unless
		//! \p reversed is true.
		
		//! If field is an empty string, then the timestamp is used.
		bool comp_records(record* lhs, record* rhs, string field, bool reversed);

		//! The list of extract criteria
		list<search_criteria_t> extract_criteria_;
		//! The mapping of indices in this log to those in full log.
		vector<qso_num_t> mapping_;
		//! The mapping of indices in the full log to those in this log.
		map<item_num_t, qso_num_t> rev_mapping_;
		//! Current use mode
		extract_mode_t use_mode_;

	};
#endif
