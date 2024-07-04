/* Types and constants associated with field ordering in views and other classes
*/

#ifndef __FIELDS__
#define __FIELDS__

#include <string>
#include <map>
#include <set>
#include <vector>

using namespace std;

// Column info 
struct field_info_t {
	string field;			// Field name
	string header;			// Text used for column headers in log table views
	unsigned int width;		// Column width in log table views

	field_info_t() :
		field(""),
		header(""),
		width(0)
	{}
	field_info_t(string f, string h, unsigned int w) :
		field(f),
		header(h),
		width(w)
	{}
};


// ADIF field ordering applications - order used for radio button and 
enum field_app_t : char {
	FO_NONE,             // No application
	FO_MAINLOG,          // Main log
	FO_EXTRACTLOG,       // Extract log
	FO_QSOVIEW,          // QSO View
	FO_IMPORTLOG,        // Records for import
	FO_LAST              // Keep at end to supply extent of enum
};

// Default columns to use in log_form 
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

// Anglicise collection names
const map<field_app_t, string> APPLICATION_LABELS = {
	{ FO_NONE, "None" },
	{ FO_MAINLOG, "Main Log" },
	{ FO_EXTRACTLOG, "Extracted" },
	{ FO_QSOVIEW, "Record browser" },
	{ FO_IMPORTLOG, "Imported data" }
};

// Field collection
typedef vector<field_info_t> collection_t;

class fields {

public:

	// Constructor
	fields(); 
	// Destructor
	~fields();

	// Get the collection for the application
	collection_t* collection(field_app_t app);
	// Get the collection named.. and if necessary copy the collection
	collection_t* collection(string name, string source = "Default");
	// Get the collection named .. and if necessary pre-populate it
	collection_t* collection(string name, set<string> values);
	// Get the field names in the collection
	set<string> field_names(string name);

	// Get the list of collection names
	set<string> coll_names();
	// Get the collection name for the application
	string coll_name(field_app_t app);

	// Link app to collection
	void link_app(field_app_t app, string coll);

	// Delete colelction - returns false if failed
	bool delete_coll(string coll);

protected:
	// Read settings
	void load_data();
	// Store settings
	void store_data();

	// The data - mapping app to collection name
	map<field_app_t, string> app_map_;
	// The data - mapping collection by name
	map<string, collection_t*> coll_map_;

};

#endif
