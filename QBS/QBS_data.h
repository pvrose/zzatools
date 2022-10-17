#pragma once
#include "QBS_window.h"
#include "QBS_consts.h"

#include <string>
#include <ctime>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

#include "FL/Fl_Preferences.H"

using namespace std;

// Data structures

typedef map<string /* call */, int /*count*/> count_data;
typedef map<string /*name */, string /* value */> call_info;

struct recycle_data {
	int sum_recycled;            // Number of cards recycled from batch
	int sum_received;            // Total number of cards in batch
	int count_recycled;          // Number of calls recycled
	int count_received;          // Number of calls received
	float weight_kg;             // Weight (in kg) of cards recycled
	array<string, 20>* top_20;   // Top 20 callsigns recycled

	recycle_data() {
		sum_recycled = 0;
		sum_received = 0;
		count_recycled = 0;
		count_received = 0;
		weight_kg = 0.0F;
		top_20 = nullptr;
	}
};

struct box_data {
	string id;                   // eg "2020 Q2"
	string date_received;        // eg "2020-07-01(E)" - estimated
	string date_sent;            // or "2022-09-26" - actual date
	string date_recycled;        //
	recycle_data* recycle_info;  // Info for this box once disposed
	count_data* counts;          // Number of cards held or disposed

	box_data() {
		id = "Unknown";
		date_received = "";
		date_sent = "";
		date_recycled = "";
		recycle_info = new recycle_data;
		counts = new count_data;
		counts->clear();
	}
};

struct action_data {
	command_t command;           // Command implememted
	int box_num;                 // Box number (-1 == in-box)
	string date;                 // Date action occurs - may be estimated
	string call;                 // Call for batch
	int num_cards;               // Number of cards involved 
	int num_sases;               // Number of SASEs involved

};

enum special_box_t {
	IN_BOX = -1,                
	OUT_BOX = -2,
	KEEP_BOX = -3
};

class QBS_data {

protected:
	// Data items

	// The main sorting boxes
	vector <box_data*> boxes_;
	// Other boxes
	count_data sases_; // SASEs
	// In-box - for items received between batches
	count_data in_box_;
	// Out box - items awaiting posting
	count_data out_box_;
	// Keep box - for cards kept between batches
	count_data keep_box_;
	// Disposal queue tail - most recent added to queue
	int tail_;
	// Disposal queue head - next to be recycled
	int head_;
	// CAllsign data
	map<string, call_info> ham_info_;
	// Raw data
	vector<action_data*> actions_;

	// Processing mode
	process_mode_t mode_;
	process_mode_t process_mode_;

	// Filename
	string filename_;
	// CSV directory
	string csv_dirname_;
	// Window
	QBS_window* window_;
	// Dummy recycle data
	recycle_data recycle_dummy_;
	// Input/Output file
	fstream file_;

public:
	QBS_data();
	~QBS_data();

	inline process_mode_t mode() {
		return mode_;
	}
	inline void mode(process_mode_t value) {
		mode_ = value;
	}

	// Windows interface - get data
	// Get count for specific box/call
	int get_count(int box_num, string call);
	// Get current box_number
	int get_current();
	// Het dispose tal box_num
	int get_tail();
	// Get dispose head box_num
	int get_head();
	// Get batch name for box_number
	string get_batch(int box_num);
	// Get call in box - - return next one after this
	string get_next_call(int box_num, string call);
	string get_prev_call(int box_num, string call);
	// Get recycle data
	recycle_data& get_recycle_data(int box_num);
	// Set window
	void set_window(QBS_window* w);

	// Load QBS file
	bool read_qbs(string filename);
	// Load and parse .CSV files
	bool import_cvs(string directory);
	// Open QBS file for writing
	bool wopen_qbs(string& filename);

	inline QBS_window* window() {
		return window_;
	}
	

public:
	// Start cards to box
	int discard_inherits(
		string date,                // date received
		string call,                // callsign
		int value                   // num of cards
	);
	// Add cards to box
	int receive_cards(
		int box_num,                // box number
		string date,                // date received
		string call,                // callsign
		int value                   // num of cards
	);
	// ADd bew batch
	int new_batch(
		int box_num,                // box number
		string date,                // date created
		string batch                // batch "name" - e.g. "2022 Q4"
	);
	// Add/delete envelopes
	int receive_sases(
		string date,                // date actioned
		string call,                // callsign
		int value                   // num of envelopes
	);
	// send cards in envelopes
	int send_cards(
		int box_num,                // box number
		string date,                // date actioned
		string call,                // callsign
		int value                   // num of cards
	);
	// Use envelopes
	int use_sases(
		string date,                // date actioned
		string call,                // callsign
		int value                   // Number of envelopes
	);
	// keep cards
	int keep_cards(
		int box_num,                // box number
		string date,                // date actioned
		string call,                // callsign
		int value                   // num of cards
	);
	// Post cards - remove from out-tray to letter box
	int post_cards(
		string date                 // date actioned
	);
	// Dispose box - move current unsent to disposal queue
	int dispose_cards(
		string date                 // date actioned
	);
	// Recycle box - move head of disposal queue to blue bin
	int recycle_cards(
		string date,                // date actioned
		float weight                // weight of cards
	);
	// Totalise received cards
	int totalise_cards(
		int box_num,                // Box number
		string date                 // date actioned
	);
	// Add ham-data
	int ham_data(
		string call,                // Callsign
		string name,                // Call info name
		string value                // Call info value
	);

	// Log the action
	void log_action(
		command_t command,
		int box_num,
		string date,
		string id,
		int i_value,
		float f_value
	);
	void log_action(
		command_t command,
		string date,
		string call,
		string name,
		string value
	);

	// Trace boxes
	void trace_boxes(ostream& os);

protected:
	// Prepare disposal report
	int disposal_report(
		int box_num             // box being disposed
	);

	// Print box summary
	void box_summary(int box, ostream& os);
	// Print SASE summary
	void sase_summary(ostream& os);

};
