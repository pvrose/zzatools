#ifndef __RECORD_TABLE__
#define __RECORD_TABLE__

#include "record.h"
#include <vector>
#include <string>
#include <FL/Fl_Table_Row.H>

using namespace std;

namespace zzalog {
	// This class is the table used in record_form to display the field items in the displayed record.
	class record_table : public Fl_Table_Row
	{
		// The records that will be displayed
		enum display_mode_t {
			NO_RECORD,         // No record will be displayed
			LOG_ONLY,          // Only a record from the log will be displayed
			QUERY_ONLY,        // Only a record from an import query for which no match had been found
			LOG_AND_QUERY      // Both the log record and supposed match from an import or duplicate query
		};
	public:
		record_table(int X, int Y, int W, int H, const char* label);
		~record_table();

		// public methods
	public:
		// inherited from Fl_Table_Row
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);
		// set the records to display
		void set_records(record* log_record, record* query_record, record* saved_record);
		// get the name of the field in row n
		string field(int row);

		// protected methods
	protected:
		// assess fields
		void assess_fields();

		// protected attributes
	protected:
		// original record from the log
		record * log_record_;
		// record from the query
		record* query_record_;
		// current record in log
		record* saved_record_;
		// The nature of the request
		display_mode_t display_mode_;
		// the fields to include
		vector<string> fields_;

	};

}
#endif