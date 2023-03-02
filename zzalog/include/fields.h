/* Types and constants associated with field ordering in views and other classes
*/

#ifndef __FIELDS__
#define __FIELDS__

#include <string>

using namespace std;



	// Column info 
	struct field_info_t {
		string field;			// Field name
		string header;			// Text used for column headers in log table views
		unsigned int width;		// Column width in log table views

		field_info_t::field_info_t() :
			field(""),
			header(""),
			width(0)
		{}
		field_info_t::field_info_t(string f, string h, unsigned int w) :
			field(f),
			header(h),
			width(w)
		{}
	};


	// ADIF field ordering applications - order used for radio button and 
	enum field_ordering_t {
		FO_MAINLOG,          // Main log
		FO_EXTRACTLOG,       // Extract log
		FO_DXATLAS,          // DxAtlas records
		FO_QSOVIEW,          // QSO View
		FO_IMPORTLOG,        // Records for import
		FO_EXPORTTSV,        // Export tab-separated-file
		FO_CHOICE,           // Only present in field choice drop-down lists
		FO_LAST              // Keep at end to supply extent of enum
	};

	// Default columns to use in log_form and record_form
	const field_info_t DEFAULT_FIELDS[] = {
		{ "QSO_DATE", "Date", 62 },
		{ "TIME_ON", "Start", 50 },
		{ "TIME_OFF", "End", 50 },
		{ "FREQ", "QRG", 65 },
		{ "BAND", "Band", 45 },
		{ "MODE", "Mode", 50 },
		{ "CALL", "Callsign", 100 },
		{ "RST_SENT", "Sent", 40 },
		{ "RST_RCVD", "Rcvd", 50 },
		{ "TX_PWR", "Pwr", 35 },
		{ "NAME", "Name", 65 },
		{ "QTH", "QTH", 150 },
		{ "GRIDSQUARE", "Loc", 62 },
		{ "", "", 0 }
	};
#endif