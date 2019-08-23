#include "spec_reader.h"
#include "spec_data.h"
#include "../zzalib/utils.h"
#include "status.h"
#include "book.h"
#include "files.h"
#include <fstream>
#include <istream>

#include <FL/Fl_Progress.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>

using namespace zzalog;

extern status* status_;
extern bool closing_;

// Default constructor
spec_reader::spec_reader()
	: ignore_version_mismatches_(false)
	, file_size_(0)
	, byte_count_(0)
	, previous_count_(0)
{
}

// Default destructor
spec_reader::~spec_reader()
{
}

// Load data from specified file into and add each record to the map
bool spec_reader::load_data(spec_data* data, string dataset_name, istream& in, string& version) {
	fl_cursor(FL_CURSOR_WAIT);
	// save the dataset pointer
	if (in.fail()) return false;
	// calculate the file size
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	file_size_ = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialise progress
	status_->misc_status(ST_NOTE, ("SPEC: Loading " + dataset_name).c_str());
	status_->progress(file_size_, OT_ADIF, "bytes");
	byte_count_ = 0;

	// Read first line - column names
	string line;
	getline(in, line);
	byte_count_ += line.length() + 1;
	display_progress();
	// remove any non-ASCII characters
	while (line[0] < 0) line = line.substr(1);
	// get column names
	if (in.good()) {
		// Get the dataset
		spec_dataset* dataset = data->dataset(dataset_name);
		// If it does not exist create it and add it - this allows us to append several files to the same dataset
		if (dataset == nullptr) {
			dataset = new spec_dataset;
			(*data)[dataset_name] = dataset;
		}
		// Write the column names to the dataset
		split_line(line, dataset->column_names, '\t');
		// Now read all the records
		getline(in, line);
		byte_count_ += line.length() + 1;
		display_progress();
		while (in.good() && !closing_) {
			// Get the various data fields
			vector<string> data_values;
			map<string, string>* data_items = new map<string, string>;
			split_line(line, data_values, '\t');
			// Copy each data field into data mapped by column name for that field - column 0 is used for dataset key
			for (unsigned int i = 1; i < dataset->column_names.size() && i < data_values.size(); i++) {
				(*data_items)[dataset->column_names[i]] = data_values[i];
			}
			// Set dataset entry 
			dataset->data[data_values[0]] = data_items;
			// Get the ADIF version field
			auto it_version = data_items->find("ADIF Version");
			if (it_version != data_items->end()) {
				// One exists
				if (it_version->second.length() > 0) {
					if (version.length() == 0) {
						// application's ADIF version not already set so use this value
						version = it_version->second;
					}
					else if (!ignore_version_mismatches_ && version != it_version->second) {
						// Version not the same - ask if OK or ignore
						// TODO: Option to abort?
						if (fl_choice("ADIF file (dataset %s) has inconsistent ADIF version %s v. %s",
							fl_ok, "Ignore further", nullptr,
							dataset_name,
							it_version->second,
							version) == 1) {
							// User replied ignore further
							ignore_version_mismatches_ = true;
						}
					}
				}
			}
			data_values.clear();
			// Read into next line
			getline(in, line);
			// Increment counts and see if we update progress
			byte_count_ += line.length() + 1;
			display_progress();
		}
	}
	bool result = in.eof() ? true : false;
	if (result) {
		status_->misc_status(ST_OK, "SPEC: Loading done!");
	}
	else {
		status_->misc_status(ST_ERROR, "SPEC: Loading failed");
	}
	fl_cursor(FL_CURSOR_DEFAULT);
	return result;
}

// Do the same for enumerated file - all the enumerations are in the same file
bool spec_reader::load_enumerated_data(spec_data* data, istream& in, string& version) {
	fl_cursor(FL_CURSOR_WAIT);
	// Open File
	if (in.fail()) return false;
	// calculate the file size
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	file_size_ = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialise progress
	status_->misc_status(ST_NOTE, "SPEC: Loading Enumerations");
	status_->progress(file_size_, OT_ADIF, "bytes");
	byte_count_ = 0;

	string line;
	// Read first line - column names
	getline(in, line);
	byte_count_ += line.length() + 1;
	display_progress();

	while (line[0] < 0) line = line.substr(1);
	bool first = true;
	spec_dataset* dataset = nullptr;
	vector<string> prev_data_values;
	int dxcc_column_num = -1;
	string curr_dxcc_value = "";
	while (in.good() && !closing_) {
		vector<string>* curr_data_values = new vector<string>;
		// Now read all the records
		split_line(line, *curr_data_values, '\t');
		if ((*curr_data_values)[0] == "Enumeration Name" ||
			(((*curr_data_values)[0] == "Primary_Administrative_Subdivision" ||
			(*curr_data_values)[0] == "Secondary_Administrative_Subdivision") &&
				dxcc_column_num != -1 &&
				(*curr_data_values)[dxcc_column_num] != curr_dxcc_value)) {
			// Enumeration name differs - save the current dataset and start a new one
			if (dataset != nullptr && prev_data_values.size() > 0) {
				// If it was Administrative subdivision save as ...._[DXCC]
				if (prev_data_values[0] == "Primary_Administrative_Subdivision" ||
					prev_data_values[0] == "Secondary_Administrative_Subdivision") {
					char enumeration_name[128];
					sprintf(enumeration_name, "%s_%s", prev_data_values[0].c_str(), curr_dxcc_value.c_str());
					(*data)[string(enumeration_name)] = dataset;
					data->set_has_states(curr_dxcc_value);
				}
				else {
					// else just save using enumeration name
					(*data)[prev_data_values[0]] = dataset;
				}
			}
			if ((*curr_data_values)[0] == "Enumeration Name") {
				// Create new enumeration - from scratch
				dataset = new spec_dataset;
				split_line(line, dataset->column_names, '\t');
				getline(in, line);
				byte_count_ += line.length() + 1;
				display_progress();
				first = true;
			}
			else {
				// Create new subdivision enumeration - use the previous ones column names
				vector<string>* column_names = &dataset->column_names;
				dataset = new spec_dataset;
				dataset->column_names.assign(column_names->begin(), column_names->end());
			}
			// Initialise subdivision 
			dxcc_column_num = -1;
			curr_dxcc_value = "";
			// Look in the columns for a DXCC Entity Code to use when we save the dataset
			for (unsigned int i = 0; i < dataset->column_names.size(); i++) {
				if (dataset->column_names[i] == "DXCC Entity Code") dxcc_column_num = i;
			}
		}
		if (in.good()) {
			// Copy data items into enumeration data set
			map<string, string>* data_items = new map<string, string>;
			prev_data_values.clear();
			split_line(line, prev_data_values, '\t');
			// Display the name of the enumeration
			if (first) {
				char* message = new char[prev_data_values[0].length() + 20];
				sprintf(message, "SPEC: Loading %s", prev_data_values[0].c_str());
				status_->misc_status(ST_NOTE, message);
				first = false;
			}
			// Copy the fields into the dataset entry - ignore column 0 (enumeration name), column 1 (its value)
			for (unsigned int i = 2; i < dataset->column_names.size() && i < prev_data_values.size(); i++) {
				(*data_items)[dataset->column_names[i]] = prev_data_values[i];
			}
			// Capitalise the enumeration value
			prev_data_values[1] = to_upper(prev_data_values[1]);
			// TODO: Temporarily rename any deleted subdivisions
			if ((prev_data_values[0] == "Primary_Administrative_Subdivision" ||
				prev_data_values[0] == "Secondary_Administrative_Subdivision" ) &&
				data_items->find("Deleted") != data_items->end() && 
				(*data_items)["Deleted"] == "Deleted") {
				prev_data_values[1] += " Deleted";
			}
			// Save the entry at the value key
			dataset->data[prev_data_values[1]] = data_items;
			// Get DXCC value if one exists
			if (dxcc_column_num != -1) curr_dxcc_value = prev_data_values[dxcc_column_num];
			// Check version
			auto it_version = data_items->find("ADIF Version");
			if (it_version != data_items->end()) {
				if (it_version->second.length() > 0) {
					// If it exists and is not empty
					if (version.length() == 0) {
						// It's the first version (unlike) - use it as app's ADIF version
						version = it_version->second;
					}
					else if (!ignore_version_mismatches_ && version != it_version->second) {
						// Differs from app's version so query user
						if (fl_choice("ADIF file (Enumerations) has inconsistent ADIF version %s v. %s",
							fl_ok, "Ignore further", nullptr,
							it_version->second,
							version) == 1) {
							// user says ignore further mismatches
							ignore_version_mismatches_ = true;
						}
					}
				}
			}
			// Read next line
			getline(in, line);
			// Update count and report progress
			byte_count_ += line.length() + 1;
			display_progress();
			// EOF - save last enumeration data-set
			if (in.eof()) {
				// If it was Administrative subdivision save as ...._[DXCC]
				if (prev_data_values[0] == "Primary_Administrative_Subdivision" ||
					prev_data_values[0] == "Secondary_Administrative_Subdivision") {
					char enumeration_name[128];
					sprintf(enumeration_name, "%s_%s", prev_data_values[0].c_str(), curr_dxcc_value.c_str());
					(*data)[string(enumeration_name)] = dataset;
				}
				else {
					(*data)[prev_data_values[0]] = dataset;
				}
			}
		}
		curr_data_values->clear();
		delete curr_data_values;
	}
	display_progress(true);
	prev_data_values.clear();
	bool result = in.eof() ? true : false;
	if (result) {
		status_->misc_status(ST_OK, "SPEC: Loading done!");
	}
	else {
		status_->misc_status(ST_ERROR, "SPEC: Loading failed");
	}
	fl_cursor(FL_CURSOR_DEFAULT);
	return result;
}


// Display progress
void spec_reader::display_progress(bool force /*=false*/) {
	// update progress - every 10 k characters
	status_->progress(byte_count_);
};
