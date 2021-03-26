#include "exc_data.h"
#include "exc_reader.h"
#include "club_handler.h"
#include "status.h"
#include "../zzalib/callback.h"

#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <ctime>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>

using namespace zzalog;
using namespace zzalib;

extern club_handler* club_handler_;
extern status* status_;
extern Fl_Preferences* settings_;

// Constructor - Download new data and build database
exc_data::exc_data() {
	// Get the filename
	string filename = get_filename();
	if (!data_valid(filename)) {
		// Exception list has expired or didn't exist
		status_->misc_status(ST_NOTE, "EXCEPTION: Downloading exception file again as doesn't exist or is > 7 days old");
		club_handler_->download_exception();
		filename.replace(filename.length() - 3, 3, "xml");
	}
	// Load the exception data
	load_data(filename);
}

// Destructor
exc_data::~exc_data() {
	delete_contents();
}

// Remove all the entries in the database
void exc_data::delete_contents() {
	// Delete all the exception entries
	for (auto it1 = entries_.begin(); it1 != entries_.end(); it1++) {
		list<exc_entry*>* list_entries = &(it1->second);
		for (auto it2 = list_entries->begin(); it2 != list_entries->end(); it2++) {
			delete (*it2);
		}
		list_entries->clear();
	}
	entries_.clear();
}

// Return the exc_data entry for the record (call and date) - nullptr if one doesn't exist
exc_entry* exc_data::is_exception(record* record) {
	// Does the call exist in the list
	string call = record->item("CALL");
	time_t timestamp = record->timestamp();
	if (entries_.find(call) == entries_.end()) {
		// The call is not mentioned at all
		return nullptr;
	}
	else {
		list<exc_entry*>* list_entries = &(entries_.at(call));
		// Exceptions are time limited - return the entry if the call was in the appropriate time-period
		for (auto it = list_entries->begin(); it != list_entries->end(); it++) {
			exc_entry* entry = *it;
			if ((entry->end == -1 || entry->end > timestamp) && 
				(entry->start == -1 || timestamp > entry->start)) {
				char message[160];
				snprintf(message, 160, "EXCEPTION: Contact %s at %s %s is in exceptions list in DXCC %d", 
					call.c_str(), record->item("QSO_DATE").c_str(), record->item("TIME_ON").c_str(), entry->adif_id);
				status_->misc_status(ST_NOTE, message);
				return entry;
			}
		}
		// Does not match any of the timeframes
		return nullptr;
	}
}

// Check timeliness of data
bool exc_data::data_valid(string filename) {
#ifdef _WIN32
	int fd = _sopen(filename.c_str(), _O_RDONLY, _SH_DENYNO);
	if (fd == -1) {
		// File doesn't exist
		return false;
	}
	// Get file status
	struct _stat status;
	int result = _fstat(fd, &status);
	time_t now;
	time(&now);
	if (difftime(now, status.st_mtime) > 7 * 24 * 3600) {
		// File is over 7 days old
		return false;
	}
	else {
		return true;
	}
#else
	// TODO: Code Posix version of the above
#endif
}

// Is the call in the list of invalid calls - unauthorised DXpeditions
bool exc_data::is_invalid(record* record) {
	string call = record->item("CALL");
	time_t timestamp = record->timestamp();
	if (invalids_.find(call) == invalids_.end()) {
		// The call is not mentioned at all
		return false;
	}
	else {
		list<invalid*>* list_invalids = &(invalids_.at(call));
		// Exceptions are time limited - return the entry if the call was in the appropriate time-period
		for (auto it = list_invalids->begin(); it != list_invalids->end(); it++) {
			invalid* inval = *it;
			if ((inval->end == -1 || inval->end > timestamp) &&
				(inval->start == -1 || timestamp > inval->start)) {
				char message[160];
				snprintf(message, 160, "EXCEPTION: Contact %s at %s %s is an invalid operation", call.c_str(), record->item("QSO_DATE").c_str(), record->item("TIME_ON").c_str());
				status_->misc_status(ST_NOTE, message);
				return true;
			}
		}
		// Does not match any of the timeframes
		return false;
	}

}

// Load data
bool exc_data::load_data(string filename) {
	ifstream is(filename.c_str(), ios_base::in);
	// File is either XML (a new download) or TSV (local copy to load faster)
	if (filename.substr(filename.length() - 3) == "xml") {
		exc_reader* reader = new exc_reader();
		char message[160];
		snprintf(message, 160, "EXCEPTION: Loading exception file %s", filename.c_str());
		status_->misc_status(ST_NOTE, message);
		if (reader->load_data(this, is, file_created_)) {
			// OK - data loaded into database
			snprintf(message, 160, "EXCEPTION: Loaded exception file %s", filename.c_str());
			status_->misc_status(ST_OK, message);
			is.close();
			// Dump database into local TSV file
			filename.replace(filename.length() - 3, 3, "tsv");
			snprintf(message, 160, "EXCEPTION: Saving exception file %s", filename.c_str());
			status_->misc_status(ST_NOTE, message);
			ofstream os(filename.c_str(), ios_base::out | ios_base::trunc);
			store(os);
			if (os.good()) {
				snprintf(message, 160, "EXCEPTION: Saved exception file %s", filename.c_str());
				status_->misc_status(ST_OK, message);
				os.close();
				return true;
			}
			else {
				snprintf(message, 160, "EXCEPTION: failed to save %s", filename.c_str());
				status_->misc_status(ST_ERROR, message);
				os.close();
				return false;
			}
		}
		else {
			// Failed - delete what may have been loaded - this ensures no exception is reported
			snprintf(message, 160, "EXCEPTION: Failed to load %s", filename.c_str());
			status_->misc_status(ST_ERROR, message);
			delete_contents();
			return false;
		}
	}
	else {
		char message[160];
		snprintf(message, 160, "EXCEPTION: Loading exception file %s", filename.c_str());
		status_->misc_status(ST_NOTE, message);
		load(is);
		if (is.good() || is.eof()) {
			// File has been read in
			snprintf(message, 160, "EXCEPTION: Loaded exception file %s", filename.c_str());
			status_->misc_status(ST_OK, message);
			is.close();
			return true;
		}
		else {
			// Failed - delete what may have been loaded - this ensures no exception is reported
			snprintf(message, 160, "EXCEPTION: Failed to load %s", filename.c_str());
			status_->misc_status(ST_ERROR, message);
			is.close();
			delete_contents();
			return false;
		}
	}
}

// Get the filename {REFERENCE DIR}/cty.tsv
string exc_data::get_filename() {
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
	if (dirname) free(dirname);
	return directory_name + "cty.tsv";
}


// Store the data base to the output stream
ostream& exc_data::store(ostream& out) {
	// First number of exceptions and invalids
	int num_exceptions = 0;
	int num_invalids = 0;
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
		num_exceptions += it->second.size();
	}
	for (auto it = invalids_.begin(); it != invalids_.end(); it++) {
		num_invalids += it->second.size();
	}
	out << num_exceptions << '\t' << num_invalids << endl;
	int i = 0;
	status_->progress(num_exceptions + num_invalids, OT_PREFIX, "Storing Exception data", "records");
	// now the individual exception entries
	for (auto it = entries_.begin(); it != entries_.end() && out.good(); it++) {
		list<exc_entry*>* data_list = &(it->second);
		for (auto it2 = data_list->begin(); it2 != data_list->end(); it2++) {
			(*it2)->store(out);
			i++;
			status_->progress(i, OT_PREFIX);
		}
	}

	// Individual invalid entries
	for (auto it = invalids_.begin(); it != invalids_.end() && out.good(); it++) {
		list<invalid*>* data_list = &(it->second);
		for (auto it2 = data_list->begin(); it2 != data_list->end(); it2++) {
			(*it2)->store(out);
			i++;
			status_->progress(i, OT_PREFIX);
		}
	}
	if (!out.good()) {
		status_->progress("Write failed", OT_PREFIX);
	}
	return out;
}

// Store the individual exception record to the output stream
ostream& exc_entry::store(ostream& out) {
	out << call << '\t';
	out << adif_id << '\t';
	out << cq_zone << '\t';
	// Some records are aeronautical mobile and have no continent - this is needed so the corresponding
	// input does not result in IO error
	if (continent.length()) {
		out << continent << '\t';
	}
	else {
		out << "--\t";
	}
	out << longitude << '\t';
	out << latitude << '\t';
	out << start << '\t';
	out << end << endl;
	return out;
}

// Store the individual invalid record to the output stream
ostream& invalid::store(ostream& out) {
	out << call << '\t';
	out << start << '\t';
	out << end << endl;
	return out;
}

// Read TSV data from the input stream into the database
istream& exc_data::load(istream& in) {
	// Get the number of each records - for progress reporting more than anything else
	int num_exceptions;
	in >> num_exceptions;
	int num_invalids;
	in >> num_invalids;
	// Read to the end of line
	string dummy;
	getline(in, dummy);
	int record_num = 0;
	status_->progress(num_exceptions + num_invalids, OT_PREFIX, "Loading Exception data", "records");
	// Read all the eception records
	for (int i = 0; i < num_exceptions && in.good(); i++) {
		exc_entry* entry = new exc_entry;
		entry->load(in);
		if (entries_.find(entry->call) == entries_.end()) {
			list<exc_entry*>* list_data = new list<exc_entry*>;
			entries_[entry->call] = *list_data;
		}
		entries_.at(entry->call).push_back(entry);
		record_num++;
		status_->progress(record_num, OT_PREFIX);
	}
	// Read all the invalid records
	for (int i = 0; i < num_invalids && in.good(); i++) {
		invalid* entry = new invalid;
		entry->load(in);
		if (invalids_.find(entry->call) == invalids_.end()) {
			list<invalid*>* list_data = new list<invalid*>;
			invalids_[entry->call] = *list_data;
		}
		invalids_.at(entry->call).push_back(entry);
		record_num++;
		status_->progress(record_num, OT_PREFIX);
	}
	// Something bad happened, either an error in reading the file, or the number of records read differs from what was expected
	if (!(in.good() || in.eof()) || record_num != num_invalids + num_exceptions) {
		status_->progress("Read failed!", OT_PREFIX);
	}
	return in;
}

// Read an individual exception record from the input stream
istream& exc_entry::load(istream& in) {
	in >> call;
	in >> adif_id;
	in >> cq_zone;
	in >> continent;
	in >> longitude;
	in >> latitude;
	in >> start;
	in >> end;
	return in;
}

// Read an individual invalid record from the input stream
istream& invalid::load(istream& in) {
	in >> call;
	in >> start;
	in >> end;
	return in;
}

