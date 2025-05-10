#include "qsl_reader.h"
#include "qsl_data.h"
#include "status.h"

#include <regex>

using namespace std;

extern status* status_;
extern string PROGRAM_VERSION;

// Constructor
qsl_reader::qsl_reader() :
    xml_wreader()
    , data_(nullptr)
    , callsign_("")
    , type_(qsl_data::LABEL)
	, current_(nullptr)
	, item_(nullptr)
{
    elements_.clear();
	xml_wreader::method_map_ = method_map_;
	xml_wreader::element_map_ = element_map_;
	data_ = new map<qsl_data::qsl_type, map<string, qsl_data*>* >;
}

qsl_reader::~qsl_reader() {
    elements_.clear();
}

bool qsl_reader::load_data(map<qsl_data::qsl_type, map<string, qsl_data*>* >* data, istream& in) {
    data_ = data;
  	// calculate the file size and initialise the progress bar
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	long file_size = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "QSL DATA: Started");
	status_->progress(file_size, OT_QSLS, "Converting XML into QSL Design database", "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "QSL DATA: Done!");
		status_->progress(file_size, OT_QSLS);
		return true;
	} else {
		// Read failed - report failure
		status_->misc_status(ST_FATAL, "QSL DATA: Read failed");
		status_->progress("Load failed", OT_QSLS);
		return false;
	}
}

// <qsls version="....">
bool qsl_reader::start_qsls(xml_wreader* that, map<string, string>* attributes) {
	if (that->elements_.size()) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected QSLS element");
		return false;
	} 
	// else
	that->elements_.push_back(QSL_QSLS);
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "VERSION") {
			if (!((qsl_reader*)that)->check_version(it.second)) {
				return false;
			} else {
				return true;
			}
			// else 
			char msg[128];
			snprintf(msg, sizeof(msg), "QSL DATA: Unexpected attribute %=% in QSLS element",
				it.first.c_str(), it.second.c_str());
			return false;
		}
	}
	return true;
}

// <qsl call="GM3ZZA" type="label">
bool qsl_reader::start_qsl(xml_wreader* that, map<string, string>* attributes) {
	char msg[128];
	// Only expect in QSLS
	if (that->elements_.back() != QSL_QSLS) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected QSL element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_QSL);
	// get the name
	string call = "";
	qsl_data::qsl_type type = qsl_data::MAX_TYPE;
	string stype;
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "CALL") call = it.second;
		else if (name == "TYPE") {
			stype = it.second;
			if (it.second == "label") type = qsl_data::LABEL;
			if (it.second == "file") type = qsl_data::FILE;
		}
		else {
			snprintf(msg, sizeof(msg), "QSL DATA: Unexpected attribute %=% in RIG element",
			it.first.c_str(), it.second.c_str());
			return false;
		}
	}
	if (call == "" || type == qsl_data::MAX_TYPE) {
		snprintf(msg, sizeof(msg), "QSL DATA: Either call= or type= is messing from QSL element");
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
	// else
	((qsl_reader*)that)->callsign_ = call;
	((qsl_reader*)that)->type_ = type;
	if (((qsl_reader*)that)->data_->find(type) == ((qsl_reader*)that)->data_->end()) {
		// New outer level map so new inner is also required
		qsl_data* qd = new qsl_data();
		map<string, qsl_data*>* call_data = new map<string, qsl_data*>;
		(*call_data)[call] = qd;
		(*((qsl_reader*)that)->data_)[type] = call_data;
		((qsl_reader*)that)->current_ = qd;
		// Used to indicate that data not loaded from XML
		qd->filename_valid = false;
		return true;
	}
	// else
	map<string, qsl_data*>* call_data = ((qsl_reader*)that)->data_->at(type);
	if (call_data->find(call) == call_data->end()) {
		qsl_data* qd = new qsl_data();
		(*call_data)[call] = qd;
		((qsl_reader*)that)->current_ = qd;
		return true;
	}
	// else
	snprintf(msg, sizeof(msg), "QSL DATA: Duplicate design call=%s, type=%s", call.c_str(), stype.c_str());
	status_->misc_status(ST_ERROR, msg);
	return false;
}

// <size unit="inch|mm|point" width="nn" height="nn" />
bool qsl_reader::start_size(xml_wreader* that, map<string, string>* attributes) {
	// Only expect in QSL
	if (that->elements_.back() != QSL_QSL) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected SIZE element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_SIZE);
	qsl_data* data = ((qsl_reader*)that)->current_;
	bool ok = true;
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "UNIT") {
			if (it.second == "inch") data->unit = qsl_data::INCH;
			else if (it.second == "mm") data->unit = qsl_data::MILLIMETER;
			else if (it.second == "point") data->unit = qsl_data::POINT;
			else ok = false;
			break;
		} else if (name == "WIDTH") data->width = stod(it.second);
		else if (name == "HEIGHT") data->height = stod(it.second);
		else {
			ok = false;
			break;
		}
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in SIZE element");
	}
	return ok;
}

// <array rows="nn" columns="nn" x_offset=.. y_offset=.. x_delta= y_delta= />
bool qsl_reader::start_array(xml_wreader* that, map<string, string>* attributes) {
	// Only expect in QSL
	if (that->elements_.back() != QSL_QSL) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected ARRAY element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_ARRAY);
	qsl_data* data = ((qsl_reader*)that)->current_;
	bool ok = true;
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "ROWS") data->rows = stoi(it.second);
		else if (name == "COLUMNS") data->columns = stoi(it.second);
		else if (name == "X_OFFSET") data->col_left = stod(it.second);
		else if (name == "Y_OFFSET") data->row_top = stod(it.second);
		else if (name == "X_DELTA") data->col_width = stod(it.second);
		else if (name == "Y_DELTA") data->row_height = stod(it.second);
		else {
			ok = false;
			break;
		}
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in ARRAY element");
	}
	return ok;
}

// <formats date="..." time="...." max_qsos="nn" />
bool qsl_reader::start_formats(xml_wreader* that, map<string, string>* attributes) {
	char msg[128];
	// Only expect in QSL
	if (that->elements_.back() != QSL_QSL) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected FORMATS element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_FORMATS);
	qsl_data* data = ((qsl_reader*)that)->current_;
	bool ok = true;
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "DATE") data->f_date = parse_date(it.second);
		else if (name == "TIME") data->f_time = parse_time(it.second);
		else if (name == "MAX_QSOS") data->max_qsos = stoi(it.second);
		else {
			ok = false;
			break;
		}
		if (data->f_date == qsl_data::FMT_INVALID_DATA) {
			snprintf(msg, sizeof(msg), "QSL DATA: Unable to parse data %s", it.second.c_str());
			status_->misc_status(ST_ERROR, msg);
			ok = false;
			break;
		}
		if (data->f_time == qsl_data::FMT_INVALID_TIME) {
			snprintf(msg, sizeof(msg), "QSL DATA: Unable to parse time %s", it.second.c_str());
			status_->misc_status(ST_ERROR, msg);
			ok = false;
			break;
		}
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in FORMATS element");
	}
	return ok;

}

// <DESIGN>
bool qsl_reader::start_design(xml_wreader* that, map<string, string>* attributes) {
	// Only expect in QSL
	if (that->elements_.back() != QSL_QSL) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected QSL element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_DESIGN);
	bool ok = true;
	if (attributes->size() != 0) ok = false;
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in DESIGN element");
	}
	return ok;
}

// <TEXT>
bool qsl_reader::start_text(xml_wreader* that, map<string, string>* attributes) {
	// Only expect in QSL
	if (that->elements_.back() != QSL_DESIGN) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected TEXT element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_TEXT);
	
	// Create a new item as a text item
	((qsl_reader*)that)->item_ = new qsl_data::item_def;
	((qsl_reader*)that)->item_->type = qsl_data::TEXT;
	((qsl_reader*)that)->current_->items.push_back(((qsl_reader*)that)->item_);
	return true;
}

// <POSITION X="nn" Y="nn" />
bool qsl_reader::start_position(xml_wreader* that, map<string, string>* attributes) {
	switch(that->elements_.back()) {
	case QSL_TEXT:
	case QSL_FIELD:
	case QSL_IMAGE:
		break;
	default:
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected POSITION element");
		return false;
	}

	int* dx;
	int* dy;
	bool ok = true;
	qsl_data::item_def* item = ((qsl_reader*)that)->item_;
	switch (item->type) {
	case qsl_data::FIELD:
		if (that->elements_.back() != QSL_FIELD) {
			ok = false;
			break;
		}
		dx = &item->field.dx;
		dy = &item->field.dy;
		break;
	case qsl_data::TEXT:
		if (that->elements_.back() != QSL_TEXT) {
			ok = false;
			break;
		}
		dx = &item->text.dx;
		dy = &item->text.dy;
		break;
	case qsl_data::IMAGE:
		if (that->elements_.back() != QSL_IMAGE) {
			ok = false;
			break;
		}
		dx = &item->image.dx;
		dy = &item->image.dy;
		break;
	default:
		ok = false;
		break;
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected POSITION element");
		return false;
	}
	//else
	that->elements_.push_back(QSL_POSITION);

	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "X") *dx = stoi(it.second);
		else if (name == "Y") *dy = stoi(it.second);
		else {
			ok = false;
			break;
		}
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in POSITION element");
	}
	return ok;
}

// <LABEL SIZE="nn" FONT="nn" COLOUR="nn">[text]</LABEL>
bool qsl_reader::start_label(xml_wreader* that, map<string, string>* attributes) {
	qsl_data::style_def* style;
	bool ok = true;
	qsl_data::item_def* item = ((qsl_reader*)that)->item_;
	switch (item->type) {
	case qsl_data::FIELD:
		if (that->elements_.back() != QSL_FIELD) {
			ok = false;
			break;
		}
		style = &item->field.l_style;
		break;
	default:
		ok = false;
		break;
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected POSITION element");
		return false;
	}
	that->elements_.push_back(QSL_LABEL);
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "FONT") style->font = parse_font(it.second);
		else if (name == "SIZE") style->size = stoi(it.second);
		else if (name == "COLOUR") style->colour = stoi(it.second);
		else {
			ok = false;
			break;
		}
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in POSITION element");
	}
	return ok;
}

// <FIELD>
bool qsl_reader::start_field(xml_wreader* that, map<string, string>* attributes) {
	// Only expect in QSL
	if (that->elements_.back() != QSL_DESIGN) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected FIELD element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_FIELD);
	
	// Create a new item as a text item
	((qsl_reader*)that)->item_ = new qsl_data::item_def;
	((qsl_reader*)that)->item_->type = qsl_data::FIELD;
	((qsl_reader*)that)->current_->items.push_back(((qsl_reader*)that)->item_);
	return true;
}

// <DATA FONT= SIZE= COLOUR=>[field|text]</DATA?
bool qsl_reader::start_data(xml_wreader* that, map<string, string>* attributes) {
	qsl_data::style_def* style;
	bool ok = true;
	qsl_data::item_def* item = ((qsl_reader*)that)->item_;
	switch (item->type) {
	case qsl_data::FIELD:
		if (that->elements_.back() != QSL_FIELD) {
			ok = false;
			break;
		}
		style = &item->field.t_style;
		break;
	case qsl_data::TEXT:
		if (that->elements_.back() != QSL_TEXT) {
			ok = false;
			break;
		}
		style = &item->text.t_style;
		break;
	default:
		ok = false;
		break;
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected POSITION element");
		return false;
	}
	that->elements_.push_back(QSL_DATA);

	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "FONT") style->font = parse_font(it.second);
		else if (name == "SIZE") style->size = stoi(it.second);
		else if (name == "COLOUR") style->colour = stoi(it.second);
		else {
			ok = false;
			break;
		}
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in POSITION element");
	}
	return ok;
}

// <OPTIONS VERTICAL="Y|N" BOXED="Y|N" MULTI_QSO="Y|N" ALWAYS="Y|N"/>
bool qsl_reader::start_options(xml_wreader* that, map<string, string>* attributes) {
	bool* vertical = nullptr;
	bool* boxed = nullptr;
	bool* multi_qso = nullptr;
	bool* always = nullptr;
	bool ok = true;
	qsl_data::item_def* item = ((qsl_reader*)that)->item_;
	switch (item->type) {
	case qsl_data::FIELD:
		if (that->elements_.back() != QSL_FIELD) {
			ok = false;
			break;
		}
		vertical = &item->field.vertical;
		boxed = &item->field.box;
		multi_qso = &item->field.multi_qso;
		always = &item->field.display_empty;
		break;
	case qsl_data::TEXT:
		if (that->elements_.back() != QSL_TEXT) {
			ok = false;
			break;
		}
		vertical = &item->text.vertical;
		break;
	default:
		ok = false;
		break;
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected OPTIONS element");
		return false;
	}
	that->elements_.push_back(QSL_OPTIONS);
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "VERTICAL" && vertical) *vertical = parse_bool(it.second);
		else if (name == "BOXED" && boxed) *boxed = parse_bool(it.second);
		else if (name == "MULTI_QSO" && multi_qso) *multi_qso = parse_bool(it.second);
		else if (name == "ALWAYS" && always) *always = parse_bool(it.second);
		else {
			ok = false;
			break;
		}
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "QSL DATA: Invalid attributes in OPTIONS element");
	}
	return ok;
}

// <IMAGE>
bool qsl_reader::start_image(xml_wreader* that, map<string, string>* attributes) {
	// Only expect in QSL
	if (that->elements_.back() != QSL_DESIGN) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected IMAGE element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_IMAGE);
	// Create a new item as a text item
	((qsl_reader*)that)->item_ = new qsl_data::item_def;
	((qsl_reader*)that)->item_->type = qsl_data::IMAGE;
	((qsl_reader*)that)->current_->items.push_back(((qsl_reader*)that)->item_);
	return true;
}

// <FILE>[filename]</FILE>
bool qsl_reader::start_file(xml_wreader* that, map<string, string>* attributes) {
	// Only expect in QSL
	if (that->elements_.back() != QSL_IMAGE) {
		status_->misc_status(ST_ERROR, "QSL DATA: Unexpected FILE element");
		return false;
	}
	// else
	that->elements_.push_back(QSL_FILE);
	return true;
}

// </QSLS>
bool qsl_reader::end_qsls(xml_wreader* w) {
	qsl_reader* that = (qsl_reader*)w;
	if (that->elements_.size()) {
		status_->misc_status(ST_ERROR, "QSL DATA: Closing /RIGS element encountered unexpectedly");
		return false;
	} 
	return true;
}

// </QSL>
bool qsl_reader::end_qsl(xml_wreader* w) {
	qsl_reader* that = (qsl_reader*)w;
	that->callsign_ = "";
	that->type_ = qsl_data::MAX_TYPE;
	that->current_ = nullptr;
	that->item_ = nullptr;
	return true;
}
// <FILE>[filename]</FILE>
bool qsl_reader::chars_file(xml_wreader* w, string content) {
	qsl_reader* that = (qsl_reader*)w;
	if (that->item_->type == qsl_data::IMAGE) {
		that->item_->image.filename = content;
		return true;
	} else {
		status_->misc_status(ST_ERROR, "QSL DATA: Processing image filename when not processing an image");
		return false;
	}
}

// <LABEL FONT=.....>[Text to use]</LABEL
bool qsl_reader::chars_label(xml_wreader* w, string content) {
	qsl_reader* that = (qsl_reader*)w;
	switch(that->item_->type) {
	case qsl_data::FIELD:
		that->item_->field.label = content;
		return true;
	default:
		status_->misc_status(ST_ERROR, "QSL DATA: Processing label when not processing a field item");
		return false;
	}
}

bool qsl_reader::chars_data(xml_wreader* w, string content) {
	qsl_reader* that = (qsl_reader*)w;
	switch(that->item_->type) {
	case qsl_data::FIELD:
		that->item_->field.field = content;
		return true;
	case qsl_data::TEXT:
		that->item_->text.text = content;
		return true;
	default:
		status_->misc_status(ST_ERROR, "QSL DATA: Processing data when not processing a field or text item");
		return false;
	}
}

// Valid date - YYYYMMDD from 1930
const basic_regex<char> REGEX_Y4MDA("(19[0-9]{2}|[2-9][0-9]{3})(0[1-9]|1[0-2])(0[1-9]|[1-2][0-9]|3[01])");
// Valid date - YYYY/MM/DD from 1930
const basic_regex<char> REGEX_Y4MD("(19[0-9]{2}|[2-9][0-9]{3})/(0[1-9]|1[0-2])/(0[1-9]|[1-2][0-9]|3[01])");
// Valid date - YY/MM/DD from 2001
const basic_regex<char> REGEX_Y2MD("([0-9]{2})/(0[1-9]|1[0-2])/(0[1-9]|[1-2][0-9]|3[01])");
// Valid Time - HHMMSS
const basic_regex<char> REGEX_HMSA("([01][0-9]|2[0-3])[0-5][0-9][0-5][0-9]");
// Valid Time - HHMM
const basic_regex<char> REGEX_HMA("([01][0-9]|2[0-3])[0-5][0-9]");
// Valid Time - HH:MM:SS
const basic_regex<char> REGEX_HMS("([01][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]");
// Valid Time - HH:MM
const basic_regex<char> REGEX_HM("([01][0-9]|2[0-3]):[0-5][0-9]");

// Parse date and time formats
qsl_data::date_format qsl_reader::parse_date(string s) {
	if (regex_match(s.c_str(), REGEX_Y4MDA)) return qsl_data::FMT_Y4MD_ADIF;
	if (regex_match(s.c_str(), REGEX_Y4MD)) return qsl_data::FMT_Y4MD;
	if (regex_match(s.c_str(), REGEX_Y2MD)) return qsl_data::FMT_Y2MD;
	return qsl_data::FMT_INVALID_DATA;
}

qsl_data::time_format qsl_reader::parse_time(string s) {
	if (regex_match(s.c_str(), REGEX_HMSA)) return qsl_data::FMT_HMS_ADIF;
	if (regex_match(s.c_str(), REGEX_HMA)) return qsl_data::FMT_HM_ADIF;
	if (regex_match(s.c_str(), REGEX_HMS)) return qsl_data::FMT_HMS;
	if (regex_match(s.c_str(), REGEX_HM)) return qsl_data::FMT_HM;
	return qsl_data::FMT_INVALID_TIME;

}

Fl_Font qsl_reader::parse_font(string s) {
	Fl_Font f;
	string src = to_upper(s);
	if (regex_search(src, regex("COURIER"))) f = FL_COURIER;
	else if(regex_search(src, regex("HELVETICA"))) f = FL_HELVETICA;
	else if (regex_search(src, regex("TIMES"))) f = FL_TIMES;
	if (regex_search(src, regex("BOLD"))) f |= FL_BOLD;
	if (regex_search(src, regex("ITALIC"))) f |= FL_ITALIC;
	return f;	
}

bool qsl_reader::parse_bool(string s) {
	if (to_upper(s) == "YES") return true;
	else return false;
}

// Check app version is >= version in settings file
bool qsl_reader::check_version(string v) {
	vector<string> file_words;
	split_line(v, file_words, '.');
	vector<string> prog_words;
	split_line(PROGRAM_VERSION, prog_words, '.');
	if (stoi(prog_words[0]) > stoi(file_words[0])) {
		return true;
	} else if (prog_words[0] == file_words[0]) {
		if (stoi(prog_words[1]) > stoi(prog_words[1])) {
			return true;
		}
		else if (prog_words[1] == file_words[1]) {
			if (stoi(prog_words[2]) >= stoi(file_words[2])) {
				return true;
			}
		}
	}
	return false;
}    

