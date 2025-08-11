#include "cty3_reader.h"

#include "cty_data.h"
#include "status.h"

extern status* status_;

cty3_reader::cty3_reader() {}
cty3_reader::~cty3_reader() {}

// Parse the generic parts of the record
cty_element* cty3_reader::load_element(cty_element::type_t type, string line, string& nickname, string& patterns) {
	vector<string> fields;
	split_line(line, fields, '|');
	cty_element* result = new cty_element;
	result->type_ = type;
	if (fields[FT_DXCC_ID].length()) result->dxcc_id_ = stoi(fields[FT_DXCC_ID]);
	result->name_ = fields[FT_NAME];
	if (fields[FT_START].length())	result->time_validity_.start = fields[FT_START] + "0000";
	if (fields[FT_FINISH].length())	result->time_validity_.finish = fields[FT_FINISH] + "2359";
	if (fields[FT_CQZ].length()) result->cq_zone_ = stoi(fields[FT_CQZ]);
	if (fields[FT_ITUZ].length()) result->itu_zone_ = stoi(fields[FT_ITUZ]);
	if (fields[FT_CONTINENT].length()) result->continent_ = fields[FT_CONTINENT];
	if (fields[FT_LONGITUDE].length()) result->coordinates_.longitude = stod(fields[FT_LONGITUDE]) / 180.;
	if (fields[FT_LATITUDE].length()) result->coordinates_.latitude = stod(fields[FT_LATITUDE]) / 180.;
	patterns = fields[FT_PFX_MASK];
	nickname = fields[FT_NICKNAME];
	return result;
}

// Convert prefix mask to list of prefixes
list<string> cty3_reader::expand_mask(string patterns) {
	char msg[128];
	list<string> result;
	// Separate int patterns
	vector<string> indiv_patts;
	split_line(patterns, indiv_patts, ',');
	for (auto ptn : indiv_patts) {
		int num_multis = 0;
		for (auto c : ptn) {
			switch (c) {
			case '#':
			case '@':
			case '?':
				num_multis++;
				break;
			default:
				break;
			}
		}
		if (num_multis > 1) {
			data_->out() << "Pattern " << ptn << " ignored - too many individual prefixes\n";
			continue;
		}
		// Preprocess removing #,@,? and -
		string expanded = "";
		expanded.reserve(500);
		bool braced = false;
		bool slash_seen = false;
		bool sequence = false;
		bool ignore = false;
		char last_c = '\0';
		for (auto c : ptn) {
			switch (c) {
			case '[':
				braced = true;
				expanded += c;
				break;
			case ']':
				braced = false;
				slash_seen = false;
				expanded += c;
				break;
			case '-':
				sequence = true;
				break;
			case '#':
				if (!slash_seen) expanded += "[0123456789]";
				break;
			case '@':
				if (!slash_seen) expanded += "[ABCDEFGHIJKLMNOPQRSTUVWXYZ]";
				break;
			case '?':
				if (!slash_seen) expanded += "[0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ]";
				break;
			case '/':
				if (braced) slash_seen = true;
				else ignore = true;;
				break;
			default:
				if (braced && sequence) {
					for (char ca = last_c + 1; ca <= c; ca++) {
						expanded += ca;
					}
					sequence = false;
				}
				else {
					expanded += c;
					last_c = c;
				}
			}
		}

		if (ignore) continue;

		list<string> interim; // Interim result
		list<string> temp_start;
		braced = false;
		for (auto c : expanded) {
			switch (c) {
			case '[':
				braced = true;
				temp_start = interim;
				interim.clear();
				break;
			case ']':
				// If no interim - implies [] seen, so restore value before [
				if (interim.size() == 0) interim = temp_start;
				braced = false;
				break;
			case ',':
				result.insert(result.end(), interim.begin(), interim.end());
				interim.clear();
				break;
			case '/':
				// TODO: just ignore for now.
				break;
			default:
				if (braced) {
					if (temp_start.size()) {
						for (auto it : temp_start) {
							interim.push_back(it + c);
						}
					}
					else {
						interim.push_back({ c });
					}
				}
				else {
					if (interim.size()) {
						temp_start = interim;
						interim.clear();
						for (auto it : temp_start) {
							interim.push_back(it + c);
						}
					}
					else {
						interim.push_back({ c });
					}
				}
			}
		}
		result.insert(result.end(), interim.begin(), interim.end());
	}

	return result;
}

// Decode entity record
void cty3_reader::load_entity(string line, bool deleted) {
	string nickname = "";
	string pattern;
	// Get the basic data
	cty_element* element = load_element(cty_element::CTY_ENTITY, line, nickname, pattern);
	// Generate entity record 
	cty_entity* entity = new cty_entity;
	*(cty_element*)entity = *element;
	entity->deleted_ = deleted;
	entity->nickname_ = nickname;
	data_->add_entity(entity);
	// Generate all prefix records
	list<string> pfx_pattern = expand_mask(pattern);
	current_prefixes_.clear();
	element->type_ = cty_element::CTY_PREFIX;
	for (auto it : pfx_pattern) {
		cty_prefix* pfx = new cty_prefix;
		*(cty_element*)pfx = *element;
		current_prefixes_.push_back(pfx);
		data_->add_prefix(it, pfx);
	}
}

// Decode geography record
void cty3_reader::load_geography(string line, list<cty_prefix*> parents, bool deleted) {
	string nickname = "";
	string pattern;
	cty_geography* geo = new cty_geography;
	cty_element* element = load_element(cty_element::CTY_GEOGRAPHY, line, nickname, pattern);
	*(cty_element*)geo = *element;
	geo->pattern_ = pattern;
	geo->nickname_ = nickname;
	geo->deleted_ = deleted;
	data_->add_filter(geo);
	for (auto it : parents) {
		it->filters_.push_back(geo);
	}
}

// Decode usage record
void cty3_reader::load_usage(string line, list<cty_prefix*> parents) {
	string nickname = "";
	string pattern;
	cty_filter* usage = new cty_filter;
	cty_element* element = load_element(cty_element::CTY_FILTER, line, nickname, pattern);
	*(cty_element*)usage = *element;
	usage->pattern_ = pattern;
	usage->nickname_ = nickname;
	data_->add_filter(usage);
	for (auto it : parents) {
		it->filters_.push_back(usage);
	}
}


// Load data from specified file into and add each record to the map
bool cty3_reader::load_data(cty_data* data, istream& in, string& version) {
	data_ = data;
	// calculate the file size and initialise the progress bar
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	long file_size = (long)(endpos - startpos);
	int bytes;
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "CTY DATA: Started importing data");
	status_->progress(file_size, OT_PREFIX, "Importing country data from DxAtlas", "bytes");

	// Read and discard header lines
	string line;
	getline(in, line);
	while (line[0] == '#') getline(in, line);
	bool ok = in.good();

	while (ok) {
		// What sort of record is it?
		size_t pos = line.find('|');
		int depth = 0;
		rec_type_t type;
		string stype = line.substr(0, pos);
		switch (pos) {
		case 2:
			// top-level record "<TYPE>"
			depth = 0;
			type = (rec_type_t)stoi(stype, 0, 10);
			break;
		case 3:
			// top-level record "<MODE><TYPE>"
			depth = 0;
			type = (rec_type_t)stoi(stype.substr(1, 2), 0, 10);
			break;
		case 4:
			// sub-level record "<TYPE><DEPTH>"
			depth = stoi(stype.substr(2, 2));
			type = (rec_type_t)stoi(stype.substr(0, 2), 0, 10);
			break;
		case 5:
			// sub-level record "<MODE><TYPE><DEPTH>"
			depth = stoi(stype.substr(3, 2));
			type = (rec_type_t)stoi(stype.substr(1, 2), 0, 10);
			break;
		}
		char msg[128];
		switch (type) {
		case CTY_UNDEFINED:
			data_->out() << "Undefined record " << line << "\n";
			break;
		case CTY_ENTITY:
			load_entity(line, false);
			break;
		case CTY_GEOGRAPHY:
			load_geography(line, current_prefixes_, false);
			break;
		case CTY_SPECIAL:
			load_usage(line, current_prefixes_);
			break;
		case CTY_OLD_ENTITY:
			load_entity(line, true);
			break;
		case CTY_OLD_PREFIX:
			data_->out() << "Old prefix " << line << "\n";
		case CTY_UNRECOGNISED:
			load_entity(line, false);
			break;
		case CTY_UNASSIGNED:
			load_entity(line, false);
			break;
		case CTY_OLD_GEOGRAPHY:
			load_geography(line, current_prefixes_, true);
			break;
		case CTY_CITY:
			load_geography(line, current_prefixes_, false);
			break;

		}
		getline(in, line);
		// Report progress 
		bytes = (int)in.tellg();
		status_->progress(bytes, OT_PREFIX);
		ok = in.good();
	}
	bytes = (int)in.tellg();
	status_->progress(bytes, OT_PREFIX);
	if (in.eof()) ok = true;
	return ok;
}

