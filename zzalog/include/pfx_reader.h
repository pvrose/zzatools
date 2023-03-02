#ifndef __PFX_READER__
#define __PFX_READER__

#include "prefix.h"
#include "pfx_data.h"
#include "files.h"

#include <map>
#include <string>
#include <istream>

using namespace std;


	// This class handles the reading of the prefix reference data and loads into pfx_data
	class pfx_reader
	{

	public:
		pfx_reader();
		~pfx_reader();

		// Public methods
	public:
		// Load data from specified file into and add each record to the map
		bool load_data(pfx_data& prefixes, string filename);

		// Protected methods
	protected:
		// load a single prefix record
		istream & load_record(prefix* record, istream& in, load_result_t& result);
		// Display progress
		void display_progress();

		// size of the file in bytes
		long file_size_;
		// Number of bytes read
		long byte_count_;
		// Number of bytes read previously reported
		long previous_count_;
	};
#endif

