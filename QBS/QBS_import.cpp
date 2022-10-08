#include "QBS_import.h"
#include "QBS_consts.h"
#include "QBS_data.h"
#include "QBS_window.h"
#include "../zzalib/utils.h"

#include <string>
#include <iostream>
#include <fstream>

using namespace zzalib;
using namespace std;

QBS_import::QBS_import() {
	clear_maps();
}

QBS_import::~QBS_import()
{
	clear_maps();
}

void QBS_import::clear_maps() {
	card_matrix_.clear();
	sase_matrix_.clear();
	dates_.clear();
	calls_.clear();
	boxes_.clear();
	batch_names_.clear();
	batch_weights_.clear();
}

bool QBS_import::load_data(QBS_data* data, const char* directory) {
	data_ = data;
	directory_ = directory;
	const int NUM_FILES = 3;
	bool ok = true;

	data_->window()->update_import(1);

	ok &= read_batches();
	data_->window()->update_import(1.25F);
	ok &= read_card_data();
	data_->window()->update_import(1.50F);
	ok &= read_sase_data();
	data_->window()->update_import(1.75F);
	ok &= copy_data();
	data_->window()->update_import(2.00F);
	return ok;
}

bool QBS_import::read_batches() {
	string filename = directory_ + "\\batches.csv";
	clog << "Reading.. " << filename << endl;
	string line;
	int count = 0;
	in_.open(filename.c_str(), ios::in);
	if (in_.good()) {
		getline(in_, line);
		split_line(line, columns_, ',');
		int ix = 0;
		while (in_.good()) {
			getline(in_, line);
			read_batch(ix++, line);
			count++;
		}
	}
	bool ok = in_.good() || in_.eof();
	if (!ok) clog << " Failed!";
	else clog << count << " batches read";
	clog << endl;
	in_.close();
	return ok;
}

// Read the batches data - store in the holding arrays
bool QBS_import::read_batch(int box, string line) {
	vector<string> values;
	split_line(line, values, ',');
	// Check col 0 is a valid batch name
	if (values[0].substr(0, 2) == "20") {
		string batch = values[0];
		clog << batch << '\r';
		batch_names_[box] = batch;
		boxes_[batch] = box;
		for (unsigned int col = 1; col < values.size() && col < columns_.size(); col++) {
			string& val = values[col];
			string column = to_upper(columns_[col]);
			// Received date
			if (column == "RECEIVED" && val.length() > 0) {
				event_t event = { batch, RECEIVED };
				if (dates_.find(val) == dates_.end()) {
					dates_[values[col]] = event;
				}
				else {
					cerr << "Duplicate date " << val << " encountered" << endl;
				}
			}
			// Recycled date
			else if (column == "SENT" && val.length() > 0) {
				event_t event = { batch, SENT };
				if (dates_.find(val) == dates_.end()) {
					dates_[values[col]] = event;
				}
				else {
					cerr << "Duplicate date " << val << " encountered" << endl;
				}
			}
			// Recycled date
			else if (column == "DISPOSED" && val.length() > 0) {
				event_t event = { batch, RECYCLED };
				if (dates_.find(val) == dates_.end()) {
					dates_[values[col]] = event;
				}
				else {
					cerr << "Duplicate date " << val << " encountered" << endl;
				}
			}
			// Batch recycled weight
			else if (
				column == "WEIGHT" && values[col].length() > 0) {
				batch_weights_[batch] = stof(values[col]);
			}
		}
	}
	return true;
}

// Read card data
bool QBS_import::read_card_data() {
	string filename = directory_ + "\\cards.csv";
	clog << "Reading.. " << filename << endl;
	string line;
	int count = 0;
	in_.open(filename.c_str(), ios::in);
	if (in_.good()) {
		getline(in_, line);
		split_line(line, columns_, ',');
		int ix = 0;
		while (in_.good()) {
			getline(in_, line);
			read_call(true, line);
			count++;
		}
	}
	bool ok = in_.good() || in_.eof();
	if (!ok) clog << "Failed!";
	else clog << count << " data items read";
	in_.close();
	clog << endl;
	return ok;
}

// Read SASE data
bool QBS_import::read_sase_data() {
	string filename = directory_ + "\\sases.csv";
	clog << "Reading.. " << filename << endl;
	string line;
	int count = 0;
	in_.open(filename.c_str(), ios::in);
	if (in_.good()) {
		getline(in_, line);
		split_line(line, columns_, ',');
		int ix = 0;
		while (in_.good()) {
			getline(in_, line);
			read_call(false, line);
			count++;
		}
	}
	bool ok = in_.good() || in_.eof();
	in_.close();
	if (!ok) clog << "Failed!";
	else clog << count << " data items read";
	clog << endl;
	return ok;
};

// Read a line of the card or SASE files
bool QBS_import::read_call(bool card, string line) {
	vector<string> values;
	split_line(line, values, ',');
	// values[0] is callsign
	if (values[0][0] == 'G') {
		string call = values[0];
		clog << call << "              " << '\r';
		if (call.length() > 6) clog << endl;
		call_data& info = calls_[call];
		for (unsigned int ix = 1; ix != values.size() && ix < columns_.size(); ix++) {
			string column = to_upper(columns_[ix]);
			string& value = values[ix];
			map<string, vector<int>>& row = card ? card_matrix_[call] : sase_matrix_[call];
			// read the call information
			if (column == "RSGB") {
				try { info.rsgb_id = stoi(value); }
				catch (exception e) { info.rsgb_id = 0; }
			}
			else if (column == "INFO") {
				info.info = value;
			}
			else if (column == "NAME") {
				info.name = value;
			}
			else if (column == "E-MAIL") {
				info.e_mail = value;
			}
			else if (column == "ADDRESS") {
				info.address = value;
			}
			else if (column.length() > 8) {
				string batch = column.substr(0, 7);
				row[batch].resize(5);
				direction_t dirn = DIRN_INVALID;
				int count;
				try { count = stoi(value); }
				catch (exception e) { count = 0; }
				switch (column[8]) {
				case 'S':
					dirn = SENT;
					break;
				case 'R':
					dirn = RECEIVED;
					break;
				case 'D':
					dirn = RECYCLED;
					break;
				case 'K':
					dirn = KEPT;
					break;
				}
				row[batch][dirn] = count;
			}
		}
	}
	return true;
}

// Get the count value at specific point
int QBS_import::count (
	bool card, 
	string call, 
	string batch, 
	QBS_import::direction_t dirn) {
	auto it = card ? card_matrix_.find(call) : sase_matrix_.find(call);
	auto it_end = card ? card_matrix_.end() : sase_matrix_.end();
	if (it != it_end) {
		auto it_b = (*it).second.find(batch);
		if (it_b != (*it).second.end()) {
			return (*it_b).second[dirn];
		}
	}
	return 0;
}

// Copy internal data to QBS_data
// In date order - from the event generate the call to QBS_data
// to replicate the spreadsheet updates
bool QBS_import::copy_data() {
	clog << "Copying data...." << endl;
	for (auto d = dates_.begin(); d != dates_.end(); d++) {
		event_t& event = (*d).second;
		const string& date = (*d).first;
		string batch = event.batch;
		int box = boxes_[batch];
		direction_t dirn = event.in_out;
		float weight = batch_weights_[batch];
		int value = 0;
		int curr = data_->get_current();
		int tail = data_->get_tail();
		int head = data_->get_head();
		switch (dirn) {
		case RECEIVED:
			data_->new_batch(box, date, batch);
			clog << "RECEIVED " << batch << ' ' << date;
			switch (box) {
			case 0:
				clog << " --- NOT PROCESSED" << endl;
				break;
			case 1:
				clog << " --- INITIAL BATCH" << endl;
				for (auto it = calls_.begin(); it != calls_.end(); it++) {
					const string& call = (*it).first;
					data_->ham_data(call, calls_[call]);
					value = count(true, call, batch_names_[0], RECYCLED);
					if (value != 0) {
						data_->discard_inherits(date, call, value);
					}
					value = count(true, call, batch_names_[0], KEPT) +
						count(true, call, batch, RECEIVED) -
						count(true, call, batch_names_[0], RECYCLED);
					if (value != 0) {
						data_->receive_cards(box, date, call, value);
					}
					value = count(true, call, batch, SENT);
					if (value != 0) {
						data_->send_cards(box, date, call, value);
					}
					value = count(true, call, batch, KEPT);
					if (value != 0) {
						data_->keep_cards(box, date, call, value);
					}
					value = count(false, call, batch, KEPT);
					if (value != 0) {
						data_->receive_sases(date, call, value);
					}
				}
				break;
			default:
				clog << endl;
				// Card processing
				for (auto it = calls_.begin(); it != calls_.end(); it++) {
					const string& call = (*it).first;
					// TODo we need to consider IN_BOX and KEEP_BOX
					value = count(true, call, batch, RECEIVED);
					data_->receive_cards(box, date, call, value);
					value = count(true, call, batch, SENT);
					if (value != 0) {
						data_->send_cards(box, date, call, value);
					}
					value = count(true, call, batch, KEPT);
					if (value != 0) {
						data_->keep_cards(box, date, call, value);
					}
					value = count(false, call, batch, SENT);
					if (value != 0) {
						data_->use_sases(date, call, value);
					}
				}
				break;
			}
			data_->totalise_cards(box, date);
			data_->trace_boxes(clog);
			break;
		case SENT:
			clog << "SENT      " << batch << ' ' << date;
			switch (box) {
			case 0:
				clog << " --- NOT PROCESSED" << endl;
				break;
			default:
				clog << endl;
				data_->post_cards(date);
				data_->dispose_cards(date);
				data_->trace_boxes(clog);
				break;
			}
			break;
		case RECYCLED:
			clog << "RECYCLED  " << batch << ' ' << date << endl;
			data_->recycle_cards(date, weight);
			data_->trace_boxes(clog);
			break;
		default:
			cerr << "Unexpected processing action at " << date << ' ' << dirn <<
				"Current " << curr << " Tail " << tail << "Head " << head << endl;
			data_->trace_boxes(cerr);
			break;
		}
	}
	return true;
}


