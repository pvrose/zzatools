#include "pfx_reader.h"
#include "pfx_data.h"
#include "prefix.h"
#include "../zzalib/utils.h"
#include "status.h"
#include "files.h"

#include <fstream>
#include <istream>

#include <FL/Fl.H>
#include <FL/fl_ask.H>

using namespace zzalog;

extern status* status_;
extern bool closing_;

using namespace std;

// Constructor
pfx_reader::pfx_reader()
{	
	file_size_ = 0;
	byte_count_ = 0;
	previous_count_ = 0;

}

// Destructor
pfx_reader::~pfx_reader()
{
}

// Load data from the input stream into the specifc prefix record
istream& pfx_reader::load_record(prefix* record, istream& in, load_result_t& result) {
	// Read line - totting up bytes read
	vector<string> fields;
	char line[1024];
	in.getline(line, 1024);
	byte_count_ += (long)in.gcount();
	// There should be 14 fields separated by '|'
	split_line(string(line), fields, '|');

	// Test EOF etc.
	if (in.eof()) { result = LR_EOF; } 
	else if (!in.good()) { result = LR_BAD; }
	else result = LR_GOOD;

	// If read hasn't failed and the line has 14 fields or more
	if (result != LR_BAD && fields.size() >= 14) {
		int field_num = 0;
		// Field 0 - record type and depth 
		// 1 character (optional) - mode - ignored
		// 2 characters - record type
		// 2 characters (optional) - denotes depth of child record
		try {
			// Catch any record that has invalid integer or double data
			switch (fields[field_num].size()) {
			case 2:
				// top-level record "<TYPE>"
				record->depth_ = 0;
				record->type_ = (prefix_t)stoi(fields[field_num], 0, 10);
				break;
			case 3:
				// top-level record "<MODE><TYPE>"
				record->depth_ = 0;
				record->type_ = (prefix_t)stoi(fields[field_num].substr(1, 2), 0, 10);
				break;
			case 4:
				// sub-level record "<TYPE><DEPTH>"
				record->depth_ = stoi(fields[field_num].substr(2, 2));
				record->type_ = (prefix_t)stoi(fields[field_num].substr(0, 2), 0, 10);
				break;
			case 5:
				// sub-level record "<MODE><TYPE><DEPTH>"
				record->depth_ = stoi(fields[field_num].substr(3, 2));
				record->type_ = (prefix_t)stoi(fields[field_num].substr(1, 2), 0, 10);
				break;
			}
			field_num++;

			// Field 1 - longitude (units of 20") - If not provided NAN
			if (fields[field_num] == "") {
				record->longitude_ = nan("");
			}
			else {
				// Convert to degrees
				record->longitude_ = stod(fields[field_num], 0) / 180.0;
			}
			field_num++;

			// Field 2 - latitude (units of 20") - If not provided NAN
			if (fields[field_num] == "") {
				record->latitude_ = nan("");
			}
			else {
				// Convert to degrees
				record->latitude_ = stod(fields[field_num], 0) / 180.0;
			}
			field_num++;

			// Field 3 - name - plain text
			record->name_ = fields[field_num];
			field_num++;

			// Field 4 - nickname - plain text
			record->nickname_ = fields[field_num];
			field_num++;

			// Field 5 - CQ Zones - list of numbers with - and ; separators
			string_to_ints(fields[field_num], record->cq_zones_);
			field_num++;

			// Field 6 - ITU Zones - list of numbers with - and ; separators
			string_to_ints(fields[field_num], record->itu_zones_);
			field_num++;

			// Field 7 - Continents - list of 2-character codes with ; separators
			split_line(fields[field_num], record->continents_, ';');
			field_num++;

			// Field 8 - Timezones - list of TZs but treated as single text string for now
			record->timezone_ = fields[field_num];
			field_num++;

			// Field 9 - DXCC code - numeric
			if (fields[field_num] == "") {
				record->dxcc_code_ = 0;
			}
			else {
				record->dxcc_code_ = stoi(fields[field_num], nullptr, 10);
			}
			field_num++;

			// Field 10 - province - text
			record->state_ = fields[field_num];
			field_num++;

			// Field 11 - valid from date as "YYYYMMDD"
			if (fields[field_num] == "") {
				record->valid_from_ = "19000101";
			}
			else {
				record->valid_from_ = fields[field_num];
			}
			field_num++;

			// Field 12 - valid to date as "YYYYMMDD"
			if (fields[field_num] == "") {
				record->valid_to_ = "99991231";
			}
			else {
				record->valid_to_ = fields[field_num];
			}
			field_num++;

			// Field 13 - match pattern
			split_line(fields[field_num], record->patterns_, ',');

		}
		catch (invalid_argument&) {
			char * message = new char[fields[field_num].length() + 100];
			sprintf(message, "LOAD PFX DATA: Invalid data reading prefix record: field %d = %s", field_num, fields[field_num].c_str());
			status_->misc_status(ST_WARNING, message);
			delete[] message;
		}
	}
	// Tidy up
	fields.clear();

	return in;
}

// Load data from specified file into and add each record to the map
bool pfx_reader::load_data(pfx_data& prefixes, string filename) {
	bool fail = false;
	// these are used to remember the most recent prefix processed at each depth level, so that 
	// the next level down can be associated with the correct parent
	prefix* hang_points[4] = { 0, 0, 0, 0 };
	// Open File
	ifstream file;
	file.open(filename.c_str(), fstream::in);
	load_result_t result = LR_GOOD;
	// calculate the file size and initialise progress bar
	streampos startpos = file.tellg();
	file.seekg(0, ios::end);
	streampos endpos = file.tellg();
	file_size_ = (long)(endpos - startpos);
	// reposition back to beginning
	file.seekg(0, ios::beg);
	status_->misc_status(ST_NOTE, "LOAD PFX DATA: Started");
	status_->progress(file_size_, OT_PREFIX, "bytes");

	string line;
	byte_count_ = 0;
	previous_count_ = 0;
	while (file.good() && !closing_) {
		// Read every line - ignoring comment lines (starting #)
		if (file.peek() != '#') {
			prefix* record = new prefix;
			load_record(record, file, result);
			display_progress();

			// Ignored if depth 0 and the DXCC already has a prefix
			if (result == LR_GOOD && ( record->depth_ != 0 || prefixes.find(record->dxcc_code_) == prefixes.end())) {
				// Now store the record
				if (record->depth_ == 0) {
					// It's a DXCC entity - so map it against the DXCC code
					prefixes[record->dxcc_code_] = record;
					record->parent_ = NULL;
				}
				else {
					// it needs to be hung as a child of its parent 
					// the parent got remembered when it was processed in hang_points for its depth
					hang_points[record->depth_ - 1]->children_.push_back(record);
					record->parent_ = hang_points[record->depth_ - 1];
				}
				// TODO: temporary fix - old prefixes and removed entities may have nicknames that
				// have been reused
				if (record->type_ != PX_OLD_PREFIX && record->type_ != PX_REMOVED_ENTITY) {
					prefixes.add_pfx_to_nickname(record);
				}
				// Now remeber this prefix as a hanger for any possible children it may have
				hang_points[record->depth_] = record;

				// In the case where a field is empty it inherits the value from its parent
				if (record->parent_ != NULL) {
					if (record->continents_.size() == 0 || record->continents_[0] == "") record->continents_ = record->parent_->continents_;
					if (record->cq_zones_.size() == 0) record->cq_zones_ = record->parent_->cq_zones_;
					if (record->itu_zones_.size() == 0) record->itu_zones_ = record->parent_->itu_zones_;
					if (isnan(record->latitude_)) record->latitude_ = record->parent_->latitude_;
					if (isnan(record->longitude_)) record->longitude_ = record->parent_->longitude_;
					if (record->nickname_ == "") record->nickname_ = record->parent_->nickname_;
					if (record->timezone_ == "") record->timezone_ = record->parent_->timezone_;
				}
			}
			else {
				// tidy up if it's not being used
				delete record;
			}
		}
		else {
			// Comment - ignore the record except to adjust progress
			char dummy[1024];
			file.getline(dummy, 1024);
			byte_count_ += (long)file.gcount();
			display_progress();
		}
	}
	if (!file.eof()) {
		result = LR_BAD;
	}
	// Tidy up
	file.close();

	// Return success
	if (result == LR_BAD) {
		status_->misc_status(ST_ERROR, "LOAD PFX DATA: Failed");
		status_->progress("Load failed", OT_PREFIX);
		return false;
	}
	else {
		byte_count_ = file_size_;
		display_progress();
		status_->misc_status(ST_OK, "LOAD PFX DATA: Done!");
		return true;
	}
}

// Display the progress 
void pfx_reader::display_progress() {
	// update progress every 10k bytes
	status_->progress(byte_count_, OT_PREFIX);
}
