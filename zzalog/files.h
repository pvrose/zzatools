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

	// Default prefix database
	const string PREFIX_FILE = "\\Prefix.lst";

	// Default specification database
	const string ADIF_FILE = "\\all.xml";

}
#endif