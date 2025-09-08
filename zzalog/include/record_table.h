#ifndef __RECORD_TABLE__
#define __RECORD_TABLE__

#include <vector>
#include <string>
#include <FL/Fl_Table_Row.H>

using namespace std;

class record;


	//! This class is the table used in qso_manager to display the field items in the displayed record.
	class record_table : public Fl_Table_Row
	{
		//! The records that will be displayed
		enum display_mode_t {
			NO_RECORD,         //!< No record will be displayed
			LOG_ONLY,          //!< Only a record from the log will be displayed
			QUERY_ONLY,        //!< Only a record from an import query for which no match had been found
			LOG_AND_QUERY,     //!< Both the log record and supposed match from an import or duplicate query
			DUPE_QUERY,        //!< The two dupe checks are displayed
		};
	public:
		//! Constructor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		record_table(int X, int Y, int W, int H, const char* L = nullptr);
		//! Destructor.
		~record_table();

		// public methods
	public:
		//! Inherited from Fl_Table_Row: draws the contents of each cell from the records.
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);
		//! set the records to display.
		
		//! \param log_record QSO record from the log.
		//! \param query_record QSO record being queried against log_record.
		//! \param saved_record A copy of log_record to undo any changes.
		//! Note that log_record and saved_record can be nullptr. 
		void set_records(record* log_record, record* query_record, record* saved_record);
		//! Returns the name of the field in row \p row.
		string field(int row);

		// protected methods
	protected:
		//! Get the fields that will be displayed - those used in records being displayed plus the minimum set of records specified in settings.
		void assess_fields();

		// protected attributes
	protected:
		//! original record from the log
		record * log_record_;
		//! record from the query
		record* query_record_;
		//! current record in log
		record* saved_record_;
		//! The nature of the request
		display_mode_t display_mode_;
		//! the fields to include
		vector<string> display_fields_;

	};
#endif