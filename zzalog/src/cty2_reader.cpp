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
		cty_data::ent_entry* entry = new cty_data::ent_entry;
		int dxcc;
		if (load_entity(entry, in, dxcc)) {
			data_->add_entity(dxcc, entry);
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

bool cty2_reader::load_entity(cty_data::ent_entry* entry, istream& in, int& dxcc) {
	string line;
	getline(in, line);
	if (in.good()) {
		vector<string> items;
		split_line(line, items, ',');
		if (items[0][0] == '*') return false;
		entry->nickname = items[0];
		entry->name = items[1];
		dxcc = stoi(items[2]);
		entry->dxcc_id = dxcc;
		entry->continent = items[3];
		entry->cq_zone = stoi(items[4]);
		entry->itu_zone = stoi(items[5]);
		entry->location = { stod(items[6]), stod(items[7]) };
		entry->timezone = stod(items[8]);
		// Now parse patterns
		vector<string> patts;
		split_line(items[9], patts, ' ');
		for (auto it : patts) {
			cty_data::patt_entry* pantry = new cty_data::patt_entry;
			pantry->dxcc_id = dxcc;
			string match;
			load_pattern(it, match, pantry);
			data_->add_pattern(match, dxcc, pantry);
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

void cty2_reader::load_pattern(string patt, string& match, cty_data::patt_entry* entry) {
	size_t pos = 0;
	size_t spos;
	match = "";
	entry->type = 0;
	if (patt[pos] == '=') {
		entry->type |= cty_data::DXCC_EXCEPTION;
		pos++;
	}
	while (pos < patt.length()) {
		switch (patt[pos]) {
		case '(':
			spos = pos + 1;
			break;
		case ')':
			entry->cq_zone = stoi(patt.substr(spos, pos - spos));
			entry->type |= cty_data::CQZ_EXCEPTION;
			break;
		case '[':
			spos = pos + 1;
			break;
		case ']':
			entry->itu_zone = stoi(patt.substr(spos, pos - spos));
			entry->type |= cty_data::ITUZ_EXCEPTION;
			break;
		case ';':
			// ignore 
			break;
		default:
			match += patt[pos];
		}
		pos++;
	}
}
