#include "cty3_reader.h"

#include "cty_data.h"
#include "status.h"

extern status* status_;

cty3_reader::cty3_reader() {}
cty3_reader::~cty3_reader() {}

// Parse the generic parts of the record
cty_element* cty3_reader::load_element(cty_element::type_t type, std::string line, std::string& nickname, std::string& patterns) {
	std::vector<std::string> fields;
	split_line(line, fields, '|');
	cty_element* result = new cty_element;
	result->type_ = type;
	if (fields[FT_DXCC_ID].length()) result->dxcc_id_ = std::stoi(fields[FT_DXCC_ID]);
	result->name_ = fields[FT_NAME];
	if (fields[FT_START].length())	result->time_validity_.start = fields[FT_START] + "0000";
	if (fields[FT_FINISH].length())	result->time_validity_.finish = fields[FT_FINISH] + "2359";
	if (fields[FT_CQZ].length()) result->cq_zone_ = std::stoi(fields[FT_CQZ]);
	if (fields[FT_ITUZ].length()) result->itu_zone_ = std::stoi(fields[FT_ITUZ]);
	if (fields[FT_CONTINENT].length()) result->continent_ = fields[FT_CONTINENT];
	if (fields[FT_LONGITUDE].length()) result->coordinates_.longitude = std::stod(fields[FT_LONGITUDE]) / 180.;
	if (fields[FT_LATITUDE].length()) result->coordinates_.latitude = std::stod(fields[FT_LATITUDE]) / 180.;
	patterns = fields[FT_PFX_MASK];
	nickname = fields[FT_NICKNAME];
	return result;
}

// Convert prefix mask to std::list of prefixes
std::list<std::string> cty3_reader::expand_mask(std::string patterns) {
	std::list<std::string> result;
	// Separate int patterns
	std::vector<std::string> indiv_patts;
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
		std::string expanded = "";
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

		std::list<std::string> interim; // Interim result
		std::list<std::string> temp_start;
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
cty_element* cty3_reader::load_entity(std::string line, bool deleted) {
	std::string nickname = "";
	std::string pattern;
	// Get the basic data
	cty_element* element = load_element(cty_element::CTY_ENTITY, line, nickname, pattern);
	// Generate entity record 
	cty_entity* entity = new cty_entity;
	*(cty_element*)entity = *element;
	entity->deleted_ = deleted;
	entity->nickname_ = nickname;
	data_->add_entity(entity);
	// Generate all prefix records
	std::list<std::string> pfx_pattern = expand_mask(pattern);
	return entity;
}

// Decode geography record
cty_element* cty3_reader::load_geography(std::string line, bool deleted) {
	std::string nickname = "";
	std::string pattern;
	cty_geography* geo = new cty_geography;
	cty_element* element = load_element(cty_element::CTY_GEOGRAPHY, line, nickname, pattern);
	*(cty_element*)geo = *element;
	geo->pattern_ = pattern;
	geo->nickname_ = nickname;
	geo->deleted_ = deleted;
	return geo;
}

// Decode usage record
cty_element* cty3_reader::load_usage(std::string line) {
	std::string nickname = "";
	std::string pattern;
	cty_filter* usage = new cty_filter;
	cty_element* element = load_element(cty_element::CTY_FILTER, line, nickname, pattern);
	*(cty_element*)usage = *element;
	usage->pattern_ = pattern;
	usage->nickname_ = nickname;
	return usage;
}


// Load data from specified file into and add each record to the std::map
bool cty3_reader::load_data(cty_data* data, std::istream& in, std::string& version) {
	data_ = data;
	// calculate the file size and initialise the progress bar
	std::streampos startpos = in.tellg();
	in.seekg(0, std::ios::end);
	std::streampos endpos = in.tellg();
	long file_size = (long)(endpos - startpos);
	int bytes;
	// reposition back to beginning
	in.seekg(0, std::ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "CTY DATA: Started importing data");
	status_->progress(file_size, OT_PREFIX, "Importing country data from DxAtlas", "bytes");

	// Read and discard header lines
	std::string line;
	getline(in, line);
	while (line[0] == '#') getline(in, line);
	bool ok = in.good();

	while (ok) {
		// What sort of record is it?
		size_t pos = line.find('|');
		int depth = 0;
		rec_type_t type;
		std::string stype = line.substr(0, pos);
		switch (pos) {
		case 2:
			// top-level record "<TYPE>"
			depth = 0;
			type = (rec_type_t)std::stoi(stype, 0, 10);
			break;
		case 3:
			// top-level record "<MODE><TYPE>"
			depth = 0;
			type = (rec_type_t)std::stoi(stype.substr(1, 2), 0, 10);
			break;
		case 4:
			// sub-level record "<TYPE><DEPTH>"
			depth = std::stoi(stype.substr(2, 2));
			type = (rec_type_t)std::stoi(stype.substr(0, 2), 0, 10);
			break;
		case 5:
			// sub-level record "<MODE><TYPE><DEPTH>"
			depth = std::stoi(stype.substr(3, 2));
			type = (rec_type_t)std::stoi(stype.substr(1, 2), 0, 10);
			break;
		}
		current_elements_.resize(depth + 1);
		switch (type) {
		case CTY_UNDEFINED:
			data_->out() << "Undefined record " << line << "\n";
			break;
		case CTY_ENTITY:
			// Add the element and remove the filters
			current_elements_[depth] = load_entity(line, false);
			break;
		case CTY_GEOGRAPHY:
			current_elements_[depth] = load_geography(line, false);
			data_->add_filter(current_elements_[depth-1], (cty_filter*)current_elements_[depth]);
			break;
		case CTY_SPECIAL:
			current_elements_[depth] = load_usage(line);
			data_->add_filter(current_elements_[depth - 1], (cty_filter*)current_elements_[depth]);
			break;
		case CTY_OLD_ENTITY:
			current_elements_[depth] = load_entity(line, true);
			break;
		case CTY_OLD_PREFIX:
			data_->out() << "Old prefix " << line << "\n";
			break;
		case CTY_UNRECOGNISED:
			current_elements_[depth] = load_entity(line, false);
			break;
		case CTY_UNASSIGNED:
			current_elements_[depth] = load_entity(line, false);
			break;
		case CTY_OLD_GEOGRAPHY:
			current_elements_[depth] = load_geography(line, true);
			data_->add_filter(current_elements_[depth - 1], (cty_filter*)current_elements_[depth]);
			break;
		case CTY_CITY:
			current_elements_[depth] = load_geography(line, false);
			data_->add_filter(current_elements_[depth - 1], (cty_filter*)current_elements_[depth]);
			break;

		}
		getline(in, line);
		// Report progress 
		ok = in.good();
		if (ok) {
			bytes = (int)in.tellg();
			status_->progress(bytes, OT_PREFIX);
		}
	}
	if (in.eof()) ok = true;
	return ok;
}

