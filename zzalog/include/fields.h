/*! \file */
//! Types and constants associated with field ordering in views and other classes.


#ifndef __FIELDS__
#define __FIELDS__

#include <string>
#include <map>
#include <set>
#include <vector>
#include <list>

using namespace std;

class Fl_Preferences;

//! Field data element. 
struct field_info_t {
	string field;			//!< Field name
	string header;			//!< Text used for column headers in log table views
	unsigned int width;		//!< Column width in log table views
	//! Default constructor.
	field_info_t() :
		field(""),
		header(""),
		width(0)
	{}
	//! Declaration constructor.
	field_info_t(string f, string h, unsigned int w) :
		field(f),
		header(h),
		width(w)
	{}
};

//! ADIF field ordering applications - order 
enum field_app_t : char {
	FO_NONE,             //!< No application
	FO_MAINLOG,          //!< Main log
	FO_EXTRACTLOG,       //!< Extract log
	FO_QSOVIEW,          //!< QSO Manager views
	FO_IMPORTLOG,        //!< Records for import
	FO_LAST              //!< Keep at end to supply extent of enum
};

//! Default columns when no saved configuration and used for restore defaults.

//! \todo Check that all usages have a restore default feature.
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

//! Convert usage type to text.
const map<field_app_t, string> APPLICATION_LABELS = {
	{ FO_NONE, "None" },
	{ FO_MAINLOG, "Main Log" },
	{ FO_EXTRACTLOG, "Extracted" },
	{ FO_QSOVIEW, "QSO Manager" },
	{ FO_IMPORTLOG, "Imported data" }
};

//! Field collection: a set of field descriptions.
typedef vector<field_info_t> collection_t;
//! List of fields.
typedef vector<string> field_list;

//! Container class for the field descriptions.
class fields {

public:

	//! Constructor.
	fields(); 
	//! Destructor.
	~fields();

	//! Returns the collection for the usage \p app.
	collection_t* collection(field_app_t app);
	//! Returns the collection named amd if necessary create it by copying from \p source.
	collection_t* collection(string name, string source = "Default", bool* copied = nullptr);
	//! Returns the collection named and if necessary pre-populate it from the supplied \p values.
	collection_t* collection(string name, field_list values);
	//! Returns the list of field names in the collection in the collection \p name.
	field_list field_names(string name);

	//! Returns the set of collection names.
	set<string> coll_names();
	//! Returns the collection name for the usage \p app.
	string coll_name(field_app_t app);

	//! Link the usage \p app to collection \p coll.
	void link_app(field_app_t app, string coll);

	//! Delete collection named \p coll, returns false if failed.
	bool delete_coll(string coll);

	//! Save any update made in the usage.
	void save_update();

protected:
	//! Load database from fields.tsv.
	void load_data();
	//! Store databade to fields.tsv.
	void store_data();
	//! Load collections - called by load_data().
	bool load_collections();

	//! Mapping usage to collection name.
	map<field_app_t, string> app_map_;
	//! The database, mapped by collection name.
	map<string, collection_t*> coll_map_;
	//! Field settings filename.
	string filename_;

};

#endif
