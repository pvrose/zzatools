/* Types and constants used by file releated classes
*/
#ifndef __FILES__
#define __FILES__

#include <string>

using namespace std;

namespace zzalog {

	// Results from loading a file
	enum load_result_t {
		LR_GOOD,     // loaded OK.
		LR_BAD,      // failed to complete loading
		LR_EOF       // EOF read
	};

	const string PREFIX_FILE = "\\Prefix.lst";

	// List of filenames containing the Specifications
	const string ADIF_FILE = "\\all.xml";
	const struct { string dataset_name; string file_name; } ADIF_FILES[] = {
		{ "Data Types", "datatypes.tsv" },
		{ "Fields", "fields.tsv" },
		{ "", "" }
	};

}
#endif