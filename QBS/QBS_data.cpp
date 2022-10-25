#include "QBS_data.h"
#include "QBS_import.h"
#include "../zzalib/utils.h"
#include "../zzalib/callback.h"

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>

using namespace std;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern const char* DATE_FORMAT;

// Constructor - read data
QBS_data::QBS_data()
{
	boxes_.clear();
	actions_.clear();
	tail_ = -1;
	head_ = -1;
	mode_ = INITIAL;
	reading_mode_ = IMPORT;
	action_read_ = NONE;
	window_ = nullptr;
}

// Destructor - write data, tidy up
QBS_data::~QBS_data() {
	file_.close();
}

bool QBS_data::close_qbs() {
	file_.close();
	return true;
}

// Inherit cards to current box
int QBS_data::discard_inherits(
	string date,                // date received
	string call,                // callsign
	int value                   // num of cards
) {
	tail_ = 0;
	head_ = 0;
	log_action(INHERIT, 0, date, call, value, 0);
	// add cards to current box
	(*(*boxes_[0]).counts)[call] += value;
	return value;
}

// Receive cards to either inbox or current box
int QBS_data::receive_cards(
	int box_num,                // box number
	string date,                // date received
	string call,                // callsign
	int value                   // num of cards
) {
	received_box_[call] += value;
	// Check box number valid
	if (box_num != IN_BOX && box_num != boxes_.size() - 1) {
		char message[100];
		snprintf(message, 100, "C: %s box number %d neither current (%d) nor in_box",
			call.c_str(), box_num, boxes_.size() - 1);
		cerr << message << endl;
		return 0;
	}
	else {
		if (box_num == IN_BOX) {
			action_read_ = RECEIVE_CARD;
			// Add cards to in-box
			in_box_[call] += value;
		}
		else {
			action_read_ = SORT_CARDS;
			// Initialisation of diposal queue
			if (box_num == 1) {
				tail_ = 1;
				head_ = 0;
			}
			// Move in-box to current
			(*(*boxes_[box_num]).counts)[call] += in_box_[call];
			in_box_[call] = 0;
			// Add kept cards to current
			(*(*boxes_[box_num]).counts)[call] += keep_box_[call];
			keep_box_[call] = 0;
			// add cards to current box and totals
			(*(*boxes_[box_num]).counts)[call] += value;
			(*boxes_[box_num]).recycle_info->sum_received += value;
			if (value > 0) {
				(*boxes_[box_num]).recycle_info->count_received += 1;
			}
		}
		log_action(CARDS, box_num, date, call, value, 0);
		return value;
	}
}

// Create a new box for batch being received
int QBS_data::new_batch(
	int box_num,                // box number
	string date,                // date created
	string batch                // batch "name" - e.g. "2022 Q4"
) {
	mode_ = ACTIVE;
	action_read_ = NEW_BATCH;
	// Check if box number is valid
	if (box_num != boxes_.size()) {
		char message[100];
		snprintf(message, 100, "B: %s box number %d is not valid - should be %d",
			batch.c_str(), box_num, boxes_.size());
		cerr << message << endl;
		return -1;
	}
	else {
		// Create the new box and associare it with the batch name
		box_data* box = new box_data;
		box->id = batch;
		box->date_received = date;
		int num = boxes_.size();
		boxes_.push_back(box);
		log_action(BATCH, box_num, date, batch, 0, 0);
		return num;
	}
}

// Receive or delete envelopes
int QBS_data::receive_sases(
	string date,                // date actioned
	string call,                // callsign
	int value                   // num of envelopes
) {
	if (value < 0) {
		action_read_ = DISPOSE_SASE;
		// Deleting envelopes
		int avail = sases_[call];
		if (avail < value) {
			char message[100];
			snprintf(message, 100, "E: %s trying to delete more envelopes (%d) than have (%d)",
				call.c_str(), value, avail);
			cerr << message << endl;
			log_action(SASES, -1, date, call, -avail, 0);
			sases_[call] = 0;
			return avail;
		}
		else {
			sases_[call] += value;
			log_action(SASES, -1, date, call, value, 0);
			return -value;
		}
	}
	else {
		action_read_ = RECEIVE_SASE;
		// Adding envelopes
		sases_[call] += value;
		log_action(SASES, -1, date, call, value, 0);
		return value;
	}
}

// Send cards in envelopes
int QBS_data::stuff_cards(
	int box_num,                // box number
	string date,                // date actioned
	string call,                // callsign
	int value                   // num of cards
) {
	// Process valid boxes
	int sent = 0;
	sent_box_[call] += value;
	if (box_num == KEEP_BOX) {
		if (mode_ != IMPORT && keep_box_[call] < value) {
			// Send all cards available
			sent = keep_box_[call];
			char message[100];
			snprintf(message, 100, "O: %s insufficient cards in keep box - %d v %d",
				call.c_str(), sent, value);
			cerr << message << endl;
		}
		else {
			// Send specified value
			sent = value;
		}
		keep_box_[call] -= sent;
		out_box_[call] += sent;
	}
	else if (box_num == IN_BOX) {
		if (mode_ != IMPORT && in_box_[call] < value) {
			// Send all cards available
			sent = in_box_[call];
			char message[100];
			snprintf(message, 100, "O: %s insufficient cards in IN box - %d v %d",
				call.c_str(), sent, value);
			cerr << message << endl;
		}
		else {
			// Send specified value
			sent = value;
		}
		in_box_[call] -= sent;
		out_box_[call] += sent;
	}
	else if (box_num < (signed)boxes_.size() && box_num >= head_) {
		action_read_ = STUFF_CARDS;
		// Sending box number 
		box_data& box = *boxes_[box_num];
		sent = (*box.counts)[call];
		if (mode_ != IMPORT && sent < value) {
			// send all cards available
			char message[100];
			snprintf(message, 100, "O: %s insufficient cards in box %d - %d v %d",
				call.c_str(), box_num, sent, value);
			cerr << message << endl;

		}
		else {
			// Send specified amount
			sent = value;
		}
		(*box.counts)[call] -= sent;
		out_box_[call] += sent;
	}
	else {
		// Invalid box number
		char message[100];
		snprintf(message, 100, "O: %s invalid box number %d", call.c_str(), box_num);
		cerr << message << endl;
	}
	log_action(OUTPUT, box_num, date, call, sent, 0);
	return sent;
}

// Use envelopes
int QBS_data::use_sases(
	string date,                // date actioned
	string call,                // callsign
	int value                   // Number of envelopes
) {
	// Check SASEs
	int sase_used = 0;
	if (mode_ != IMPORT && sases_[call] < value) {
		char message[100];
		snprintf(message, 100, "U: %s insufficient envelopes (%d v %d)",
			call.c_str(), sases_[call], value);
		sase_used = sases_[call];
		sases_[call] = 0;
		cerr << message << endl;
	}
	else {
		sase_used = value;
		sases_[call] -= value;
	}
	log_action(USE, -1, date, call, sase_used, 0);
	return sase_used;
}

// Keep box - transfer cards from current box to keep box
int QBS_data::keep_cards(
	int box_num,                // box number
	string date,                // date actioned
	string call,                // callsign
	int value                   // num of cards
) {
	if (box_num != boxes_.size() - 1) {
		char message[100];
		snprintf(message, 100, "K: %s box number %d is not current (%d)",
			call.c_str(), box_num, boxes_.size() - 1);
		cerr << message << endl;
		return 0;
	}
	else {
		// Assume keep box is empty as receive_cards will have emptied it
		box_data& box = *boxes_[box_num];
		int rcvd = (*box.counts)[call];
		if (mode_ != IMPORT && rcvd == 0) {
			// Received box will not have been updated with inbox or keep_box
			// Move in box and keep box to received box
			(*box.counts)[call] += (in_box_[call] + keep_box_[call]);
			in_box_[call] = 0;
			keep_box_[call] = 0;
			rcvd = (*box.counts)[call];
		}
		if (mode_ != IMPORT && rcvd < value) {
			char message[100];
			snprintf(message, 100, "K: %s insufficient cards in box %d - %d v %d",
				call.c_str(), box_num, rcvd, value);
			cerr << message << endl;
			return 0;
		}
		// Move cards from current box to keep box
		keep_box_[call] = value;
		(*box.counts)[call] -= value;
		log_action(KEEP, box_num, date, call, value, 0);
		return value;
	}
}

// Post box - transfer cards and envelopes from out-tray to pillar box
int QBS_data::post_cards(
	string date                 // date actioned
) {
	mode_ = DORMANT;
	action_read_ = POST_CARDS;
	int box = boxes_.size() - 1;
	int cards = 0;
	for (auto it = out_box_.begin(); it != out_box_.end(); it++) {
		cards += (*it).second;
	}
	out_box_.clear();
	log_action(POST, box, date, "", cards, 0);
	return cards;
}

// Dispose box - convert current box into a disposal box, add it to the queue
int QBS_data::dispose_cards(
	string date                 // date actioned
) {
	action_read_ = DISPOSE_CARDS;
	int nbox = boxes_.size() - 1;
	box_data& box = *boxes_[nbox];
	int cards = 0;
	for (auto it = (*box.counts).begin();
		it != (*box.counts).end(); it++) {
		cards += (*it).second;
	}
	// Add box to tail of queue
	if (tail_ != nbox - 1) {
		char message[100];
		snprintf(message, 100,
			"D: There is %d boxes between disposal queue and current", nbox - tail_);
		cerr << message << endl;
	}
	tail_ = nbox;
	log_action(DISPOSE, nbox, date, "", cards, 0);
	return cards;
}

// Recycle box - transfer cards from head of disposal queue to blue bin
int QBS_data::recycle_cards(
	string date,                // date actioned
	float weight                // weight of cards
) {
	action_read_ = RECYCLE_CARDS;
	int cards = 0;
	int calls = 0;
	box_data& box = *boxes_[head_];

	for (auto it = (*box.counts).begin();
		it != (*box.counts).end(); it++) {
		disposed_box_[(*it).first] += (*it).second;
		cards += (*it).second;
		if ((*it).second > 0) calls++;
	}
	box.recycle_info->count_recycled = calls;
	box.recycle_info->sum_recycled = cards;
	box.recycle_info->weight_kg = weight;
	log_action(RECYCLE, head_, date, "", cards, weight);
	disposal_report(head_);
	head_++;
	return cards;
}

// Totalise box - call after box first read in
int QBS_data::totalise_cards(
	int box_num,
	string date
) {
	box_data& box = *boxes_[box_num];

	log_action(TOTALISE, box_num, date, "", box.recycle_info->sum_received, 0.0);
	return box.recycle_info->sum_received;
}

// Set ham-data
int QBS_data::ham_data(
	string call,
	string name,
	string value )
{
	ham_info_[call][name] = value;
	string date = now(true, DATE_FORMAT);
	log_action(HAM_INFO, date, call, name, value);
	return 0;
}

// Get count for specific box/call
int QBS_data::get_count(int box_num, string call) {
	switch (box_num) {
	case IN_BOX:
		if (in_box_.find(call) == in_box_.end()) {
			return 0;
		}
		else {
			return in_box_.at(call);
		}
	case OUT_BOX:
		if (out_box_.find(call) == out_box_.end()) {
			return 0;
		}
		else {
			return out_box_.at(call);
		}
	case KEEP_BOX:
		if (keep_box_.find(call) == keep_box_.end()) {
			return 0;
		}
		else {
			return keep_box_[call];
		}
	case SASE_BOX:
		if (sases_.find(call) == sases_.end()) {
			return 0;
		}
		else {
			return sases_[call];
		}
	default:
		if (box_num >= (signed)boxes_.size() || box_num < 0) {
			return 0;
		}
		else {
			if (boxes_[box_num]->counts->find(call) == boxes_[box_num]->counts->end()) {
				return 0;
			}
			else {
				return boxes_[box_num]->counts->at(call);
			}
		}
	}
}

// Get current box_number
int QBS_data::get_current() {
	return boxes_.size() - 1;
}

// The disposal queue tail pointer
int QBS_data::get_tail() {
	return tail_;
}

// The disposal queue head pointer
int QBS_data::get_head() {
	return head_;
}

// Get batch name for box_number
string QBS_data::get_batch(int box_num) {
	if (box_num < 0 || box_num >= (signed)boxes_.size()) {
		if (box_num == (signed)boxes_.size()) {
			return next_batch();
		}
		else {
			return "Invalid";
		}
	}
	else {
		return boxes_[box_num]->id;
	}
}

// Return the set of counts for a particular box (or all cards)
count_data* QBS_data::get_count_data(int box_num) {
	if (box_num < 0 || box_num >= (signed)boxes_.size()) {
		switch (box_num) {
		case RCVD_BOX: return &received_box_;
		case SENT_BOX: return &sent_box_;
		case DISP_BOX: return &disposed_box_;
		case IN_BOX: return &in_box_;
		case OUT_BOX: return &out_box_;
		case KEEP_BOX: return &keep_box_;
		case SASE_BOX: return &sases_;
		default: return nullptr;
		}
	}
	else {
		return boxes_[box_num]->counts;
	}

}

// Get call in box - - return next one after this
// box_num invalid - return ""
// last call supplied - return ""
string QBS_data::get_next_call(int box_num, string call) {
	count_data* calls = get_count_data(box_num);;
	if (calls == nullptr) {
		return "";
	} else {
		auto it = calls->find(call);
		if (it == calls->end()) {
			it = calls->begin();
		}
		else {
			it++;
		}
		if (it == calls->end()) {
			return "";
		}
		else {
			return (*it).first;
		}
	}
}

// Get call in box - - return previous 1 to this
// box_num invalid - return ""
// last call supplied - return ""
string QBS_data::get_prev_call(int box_num, string call) {
	count_data* calls = get_count_data(box_num);
	if (calls == nullptr) {
		return "";
	}
	else {
		auto it = calls->find(call);
		// If at the beginning go back to end
		if (it == calls->begin()) {
			it = calls->end();
		}
		it--;
		if (it == calls->begin()) {
			return "";
		}
		else {
			return (*it).first;
		}
	}
};

// Return the first callsign in the box
string QBS_data::get_first_call(int box_num) {
	count_data* calls = get_count_data(box_num);
	auto it = calls->begin();
	if (it == calls->end()) {
		return "";
	}
	else {
		return (*it).first;
	}
}

// Return the last callsign in the box
string QBS_data::get_last_call(int box_num) {
	count_data* calls = get_count_data(box_num);
	auto it = calls->rbegin();
	if (it == calls->rend()) {
		return "";
	}
	else {
		return (*it).first;
	}
}

// Get recycle data
recycle_data& QBS_data::get_recycle_data(int box_num) {
	if (box_num < 0 || box_num >= (signed)boxes_.size()) {
		return recycle_dummy_;
	}
	else {
		if (boxes_[box_num]->recycle_info == nullptr) {
			return recycle_dummy_;
		}
		else {
			return *boxes_[box_num]->recycle_info;
		}
	}
}

// Connect to parent window and tell it the status
void QBS_data::set_window(QBS_window* w) {
	window_ = w;
	window_->update_actions();
	window_->update_import(0);
	window_->update_qbs(0);
}

// Load CSV files
// return true if load successful
bool QBS_data::import_cvs(string& directory) {
	reading_mode_ = IMPORT;
	clog << "Importing CSV files from " << directory << endl;
	bool result = false;
	if (wopen_qbs(filename_)) {
		QBS_import* reader = new QBS_import;
		if (reader->load_data(this, directory.c_str())) {
			result = true;
			reading_mode_ = WRITING;
		}
	}
	window_->update_actions();
	return result;
}

// Open file for writing
bool QBS_data::wopen_qbs(string& filename) {
	bool ok = true;
	clog << "Opening file for writing...." << endl;
	if (filename.length() == 0) {
		// Open file chooser with supplied data
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser->title("Please select file for saving");
		chooser->filter("QBS files\t*.{qbs,txt}");
		size_t pos = filename.find_last_of("/\\");
		if (pos != string::npos) {
			string dir_name = filename.substr(0, pos + 1);
			chooser->directory(dir_name.c_str());
			chooser->preset_file(filename.substr(pos + 1).c_str());
		}
		else {
			chooser->preset_file(filename.c_str());
		}
		switch (chooser->show()) {
		case 0:
			filename = string(chooser->filename());
			break;
		default:
			cerr << "Error selecting file" << endl;
			ok = false;
			break;
		}
	}
	if (filename.length() > 0 && ok) {
		clog << filename << endl;
		file_.open(filename, ios::trunc | ios::out);
		if (file_.good()) {
			window_->filename(filename_.c_str());
			ok = true;
		}
		else {
			cerr << "Failed to open file " << filename << endl;
			ok = false;
		}
	}
	return ok;
}

bool QBS_data::read_qbs(string& filename) {
	// Turn off logging as 
	reading_mode_ = READING;
	int box;
	string date;
	string call;
	string batch;
	int value;
	float f_value;
	command_t command;
	int check;
	string line;
	vector<string> words;
	string i_name;
	string i_value;
	bool do_check = true;
	int line_num = 0;

	// Open file
	file_.open(filename.c_str(), ios::in);
	while (file_.good()) {
		do_check = true;
		date = "";
		call = "";
		batch = "";
		box = 0;
		value = 0;
		check = 0;
		getline(file_, line);
		if (file_.good()) {
			split_line(line, words, '\t');
			if (words[0].length() == 1) command = (command_t)words[0][0];
			else command = (command_t)' ';
			switch (command) {
			case INHERIT:
				date = words[1];
				call = words[2];
				value = stoi(words[3]);
				check = discard_inherits(date, call, value);
				break;
			case CARDS:
				date = words[1];
				box = stoi(words[2]);
				call = words[3];
				value = stoi(words[4]);
				check = receive_cards(box, date, call, value);
				break;
			case BATCH:
				date = words[1];
				box = stoi(words[2]);
				batch = words[3];
				check = new_batch(box, date, batch);
				do_check = false;
				break;
			case SASES:
				date = words[1];
				call = words[2];
				value = stoi(words[3]);
				check = receive_sases(date, call, value);
				break;
			case OUTPUT:
				date = words[1];
				box = stoi(words[2]);
				call = words[3];
				value = stoi(words[4]);
				check = stuff_cards(box, date, call, value);
				break;
			case USE:
				date = words[1];
				call = words[2];
				value = stoi(words[3]);
				check = use_sases(date, call, value);
				break;
			case KEEP:
				date = words[1];
				box = stoi(words[2]);
				call = words[3];
				value = stoi(words[4]);
				check = keep_cards(box, date, call, value);
				break;
			case POST:
				date = words[1];
				box = stoi(words[2]);
				value = stoi(words[3]);
				check = post_cards(date);
				break;
			case DISPOSE:
				date = words[1];
				box = stoi(words[2]);
				value = stoi(words[3]);
				check = dispose_cards(date);
				break;
			case RECYCLE:
				date = words[1];
				box = stoi(words[2]);
				value = stoi(words[3]);
				f_value = stof(words[4]);
				check = recycle_cards(date, f_value);
				break;
			case TOTALISE:
				date = words[1];
				box = stoi(words[2]);
				value = stoi(words[3]);
				check = totalise_cards(box, date);
				break;
			case HAM_INFO:
				date = words[1];
				call = words[2];
				i_name = words[3];
				i_value = words[4];
				check = ham_data(call, i_name, i_value);
				do_check = false;
				break;
			case COMMENT:
				getline(file_, line);
				// TODO what to do with comments - just ignore
				break;
			default:
				cerr << " Invalid command " << (char)command << endl;
				cerr << "  Line " << line_num << ": " << line << endl;
				break;
			}
			if (do_check && check != value) {
				cerr << " Value mismatch reading " <<
					(char)command << ' ' << date << ' ' << call << " Expected = " << check << " Read = " << value << endl;
				cerr << "  Line " << line_num << ": " << line << endl;
			}
			line_num++;
		}
	} 
	if (file_.eof()) {
		reading_mode_ = WRITING;
		file_.close();
		file_.open(filename, ios::out | ios::app);
		return true;
	}
	else {
		return false;
	}
}

// Log the action
void QBS_data::log_action(
	command_t command,
	int box_num,
	string date,
	string id,
	int i_value,
	float f_value
) {
	// Trace commands
	switch (command) {
	case BATCH:
		clog << "New batch - box #" << box_num << ": " << id << endl;
		break;
	case POST:
		clog << "POST:    ";
		trace_boxes(clog);
		break;
	case RECYCLE:
		clog << "RECYCLE: ";
		trace_boxes(clog);
		break;
	case DISPOSE:
		clog << "DISPOSE: ";
		trace_boxes(clog);
		break;
	case CARDS:
	case SASES:
	case OUTPUT:
		clog << command << ':' << id << '\t';
		trace_boxes(clog);
		break;
	}
	// Log commands for rereading
	if (reading_mode_ != READING && (file_.eof() || file_.good())) {
		switch (command) {
		case INHERIT:
		case SASES:
		case USE:
			if (i_value != 0) file_ << command << '\t' << date << '\t' << id << '\t' << i_value << endl;
			break;
		case CARDS:
		case OUTPUT:
		case KEEP:
			if (i_value != 0) file_ << command << '\t' << date << '\t' << box_num << '\t' << id << '\t' << i_value << endl;
			break;
		case BATCH:
			file_ << command << '\t' << date << '\t' << box_num << '\t' << id << endl;
			break;
		case POST:
		case DISPOSE:
		case TOTALISE:
			if (i_value != 0) file_ << command << '\t' << date << '\t' << box_num << '\t' << i_value << endl;
			break;
		case RECYCLE:
			if (i_value != 0) file_ << command << '\t' << date << '\t' << box_num << '\t' << i_value << '\t' << f_value << endl;
			break;
		case COMMENT:
			file_ << command << '\t' << id << endl;
			break;
		default:
			cerr << "Invalid command " << command << '\t' << date << box_num << '\t' << '\t' << id << endl;
			break;
		}
		if (!file_.good() && !file_.eof()) {
			cerr << "Write file failed" << endl;
		}
	}
}

void QBS_data::log_action(
	command_t command,
	string date,
	string call,
	string name,
	string value
) {
	if (file_.good() && reading_mode_ != READING) {
		switch (command) {
		case HAM_INFO:
			file_ << command << '\t' << date << '\t' << call << '\t' << name << '\t' << value << endl;
			break;
		default:
			cerr << "Invalid command " << command << endl;
			break;
		}
		if (!file_.good()) {
			cerr << "Write file failed" << endl;
		}
	}
}

//TODO - design and implement disposal report
int QBS_data::disposal_report(
	int box_num
) {
//	cerr << "Disposal report for box " << box_num << " not yet implemented " << endl;
	return false;
}

void QBS_data::box_summary(int box, ostream& os) {
	count_data* counts = nullptr;
	box_data* box_data;
	string name;
	switch (box) {
	case KEEP_BOX:
		counts = &keep_box_;
		name = "KEEP";
		break;
	case OUT_BOX:
		counts = &out_box_;
		name = "OUT";
		break;
	case IN_BOX:
		counts = &in_box_;
		name = "IN";
		break;
	default:
		if (box >= 0 && box < (signed)boxes_.size()) {
			box_data = boxes_[box];
			counts = box_data->counts;
			name = "Box " + to_string(box);
			if (box == boxes_.size() - 1) name += 'C';
			if (box == tail_) name += 'T';
			if (box == head_) name += 'H';
			if (box == head_ - 1) name += 'D';
		}
		break;
	}
	int total = 0;
	int num_calls = 0;
	if (counts != nullptr) {
		for (auto it = counts->begin(); it != counts->end(); it++) {
			total += (*it).second;
			if ((*it).second > 0) {
				num_calls++;
			}
		}
	}
	os << name << " (" << total << "/" << num_calls << ") ";
}

void QBS_data::sase_summary(ostream& os) {
	int total = 0;
	int num_calls = 0;
	for (auto it = sases_.begin(); it != sases_.end(); it++) {
		total += (*it).second;
		if ((*it).second > 0) {
			num_calls++;
		}
	}
	os << "SASE (" << total << " / " << num_calls << ") ";
}

// Dump the box data
void QBS_data::trace_boxes(ostream& os) {
	box_summary(IN_BOX, os);
	box_summary(KEEP_BOX, os);
	box_summary(OUT_BOX, os);
	for (int box = boxes_.size() - 1; box >= head_ && box >= 0; box--) {
		box_summary(box, os);
	}
	if (head_ > 0) box_summary(head_ - 1, os);
	sase_summary(os);
	os << endl;
}

// Calculate the next batch
string QBS_data::next_batch() {
	string batch = get_batch(get_current());
	int year = stoi(batch.substr(0, 4));
	int qtr = stoi(batch.substr(6, 1));
	if (qtr >= 4) {
		qtr = 1;
		year++;
	}
	else {
		qtr++;
	}
	char temp[10];
	snprintf(temp, 10, "%4d Q%1d", year, qtr);
	return string(temp);
}

// Get the reading action
action_t QBS_data::get_action() {
	return action_read_;
}