#include "cty2_reader.h"

#include "cty_data.h"
#include "status.h"

#include "drawing.h"

#include <string>
#include <vector>

using namespace std;

extern status* status_;

cty2_reader::cty2_reader() {
	data_ = nullptr;
}

cty2_reader::~cty2_reader() {
}

// Load data from specified file into and add each record to the map
bool cty2_reader::load_data(cty_data* data, istream& in, string& version) {
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
	status_->progress(file_size, OT_PREFIX, "Importing Exception data (bigcty.csv)", "bytes");

	while (in.good()) {
		cty_entity* entry = new cty_entity;
		int dxcc;
		if (load_entity(entry, in, dxcc)) {
			data_->add_entity(entry);
		}
		else {
			delete entry;
		}
	}
	if (in.eof()) {
		return true;
	}
	else {
		return false;
	}
}

bool cty2_reader::load_entity(cty_entity* entry, istream& in, int& dxcc) {
	string line;
	getline(in, line);
	if (in.good()) {
		vector<string> items;
		split_line(line, items, ',');
		if (items[0][0] == '*') return false;
		entry->nickname_ = items[0];
		entry->name_ = items[1];
		dxcc = stoi(items[2]);
		entry->dxcc_id_ = dxcc;
		entry->continent_ = items[3];
		entry->cq_zone_ = stoi(items[4]);
		entry->itu_zone_ = stoi(items[5]);
		entry->coordinates_ = { stod(items[6]), stod(items[7]) };
		//entry->timezone = stod(items[8]);
		// Now parse patterns
		vector<string> patts;
		split_line(items[9], patts, ' ');
		for (auto it : patts) {
			string match;
			bool exception;
			cty_element* entry = load_pattern(it, match, exception);
			entry->dxcc_id_ = dxcc;
			if (exception) {
				data_->add_exception(match, (cty_exception*)entry);
			}
			else {
				data_->add_prefix(match, (cty_prefix*)entry);
			}
		}
		// Report progress 
		int bytes = (int)in.tellg();
		status_->progress(bytes, OT_PREFIX);
		return true;
	}
	else {
		if (in.eof()) return true;
		else return false;
	}
}

cty_element* cty2_reader::load_pattern(string patt, string& match, bool& exception) {
	size_t pos = 0;
	size_t spos;
	match = "";
	cty_element* result;
	if (patt[pos] == '=') {
		result = new cty_exception;
		exception = true;
		pos++;
	}
	else {
		result = new cty_prefix;
		exception = false;
	}
	while (pos < patt.length()) {
		switch (patt[pos]) {
		case '(':
			spos = pos + 1;
			break;
		case ')':
			result->cq_zone_ = stoi(patt.substr(spos, pos - spos));
			break;
		case '[':
			spos = pos + 1;
			break;
		case ']':
			result->itu_zone_ = stoi(patt.substr(spos, pos - spos));
			break;
		case ';':
			// ignore 
			break;
		default:
			match += patt[pos];
		}
		pos++;
	}
	return result;
}
