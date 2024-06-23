#include "band_data.h"
#include "status.h"

#include <fstream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>

extern Fl_Preferences* settings_;
extern status* status_;

// Constructor - calls the Window constructor 
band_data::band_data()
{
	load_data();
}

// Destructor
band_data::~band_data()
{
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
		delete* it;
	}
	entries_.clear();
}

// Read the band plan data
bool band_data::load_data() {
	string filename = get_path() + "band_plan.tsv";
	ifstream file;
	file.open(filename.c_str(), fstream::in);
	// calculate the file size
	streampos startpos = file.tellg();
	file.seekg(0, ios::end);
	streampos currpos = file.tellg();
	int length = (int)(currpos - startpos);
	// reposition back to beginning
	file.seekg(0, ios::beg);
	// Initialise progress
	status_->misc_status(ST_NOTE, ("BAND: Loading band-plan data"));
	status_->progress((int)(currpos - startpos), OT_BAND, "Reading band-plan file", "bytes");
	// Read and ignore first line
	string line;
	getline(file, line);
	currpos = file.tellg();
	status_->progress((int)(currpos - startpos), OT_BAND);
	while (file.good()) {
		// Read the line and parse it
		getline(file, line);
		currpos = file.tellg();
		if (file.good()) {
			// Update progress bar
			status_->progress((int)(currpos - startpos), OT_BAND);
			// Read and decode the entry and add to the end
			band_entry_t* entry = get_entry(line);
			entries_.push_back(entry);
		}
	}
	// Return success or fail
	if (file.eof()) {
		status_->misc_status(ST_OK, "BAND: Loaded band-plan data");
		return true;
	}
	else {
		status_->misc_status(ST_ERROR, "BAND: Load band-plan data failed");
		status_->progress("Load failed!", OT_BAND);
		return false;
	}
}

// Decode an entry
band_data::band_entry_t* band_data::get_entry(string line) {
	vector<string> words;
	split_line(line, words, '\t');
	// Create the entry
	band_entry_t* result = new band_entry_t;
	// Lower->Upper->Bandwidth->Mode->Notes
	// First entry lower bound or spot entry
	result->lower = stod(words[0]);
	// Second entry upper bound - if no entry then it's a spot frequency entry
	try {
		result->upper = stod(words[1]);
	}
	catch (exception&) {
		result->upper = result->lower;
	}
	// Third entry is bandwidth
	try {
		result->bandwidth = stod(words[2]);
	}
	catch (exception&) {
		result->bandwidth = 0.0;
	}
	// Fourth is mode
	result->mode = words[3];
	// Fifth is summary
	result->summary = words[4];
	// Sixth is notes
	result->notes = words[4];
	return result;
}

// Get the directory of the reference files
string band_data::get_path() {
	// get the datapath settings group.
	Fl_Preferences datapath(settings_, "Datapath");
	char* dirname = nullptr;
	string directory_name;
	// get the value from settings or force new browse
	if (!datapath.get("Reference", dirname, "")) {
		// We do not have one - so open chooser to get one
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select reference file directory");
		chooser->preset_file(dirname);
		while (chooser->show()) {}
		directory_name = chooser->filename();
		delete chooser;
	}
	else {
		directory_name = dirname;
	}
	// Append a foreslash if one is not present
	if (directory_name.back() != '/') {
		directory_name.append(1, '/');
	}
	// Free any memory used
	if (dirname) free(dirname);
	datapath.set("Reference", directory_name.c_str());
	return directory_name;

}

// Get the band plan data entry for the specified frequency
band_data::band_entry_t* band_data::get_entry(double frequency) {
	for (unsigned int ix = 0; ix < entries_.size(); ix++) {
		if (entries_[ix]->lower <= frequency && entries_[ix]->upper >= frequency) {
			return entries_[ix];
		}
	}
	return nullptr;
}

// Is the supplied frequency in a band
bool band_data::in_band(double frequency) {
	if (get_entry(frequency)) {
		return true;
	}
	else {
		return false;
	}
}
