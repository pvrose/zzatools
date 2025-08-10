#include "cty3_reader.h"

#include "cty_data.h"
#include "status.h"

extern status* status_;

cty3_reader::cty3_reader() {}
cty3_reader::~cty3_reader() {}

// Load data from specified file into and add each record to the map
bool cty3_reader::load_data(cty_data* data, istream& in, string& version) {
	data_ = data;
	// calculate the file size and initialise the progress bar
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	long file_size = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "CTY DATA: Started importing data");
	status_->progress(file_size, OT_PREFIX, "Importing Prefix data (prefix.csv)", "bytes");

	cty_entity* current_entity = nullptr;
	vector<raw_prefix_t*> current_prefix;
	int current_depth = 0;

	while (in.good()) {
		string line;
		getline(in, line);

		// Results from parsing line
		cty_entity* this_entity = nullptr;
		raw_prefix_t* this_prefix = nullptr;
		int depth;
		rec_type_t type;

	}
	return false;
}

