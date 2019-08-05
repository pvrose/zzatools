#ifndef __ADIF_READER__
#define __ADIF_READER__


#include <istream>
#include <string>
#include <map>
#include <vector>

using namespace std;

namespace zzalog {

	struct spec_dataset;
	class spec_data;

	// Class loads the adif specification database from the .xml files provided by ADIF Forum
	class spec_reader
	{
	public:
		spec_reader();
		~spec_reader();

		// Public methods
	public:
		// Load data from specified file into and add each record to the map
		bool load_data(spec_data* data, string dataset_name, istream& in, string& version);
		// Do the same for enumerated file
		bool load_enumerated_data(spec_data* data, istream& in, string& version);
		// Protected methods
	protected:
		// Display progress
		void display_progress(bool force = false);

		// protected attributes
		// File size - used for progress
		long file_size_;
		// Byte count currently read
		long byte_count_;
		// Byte count at previous progress statement
		long previous_count_;
		// Ignore further ADIF version mismatches
		bool ignore_version_mismatches_;

	};

}
#endif