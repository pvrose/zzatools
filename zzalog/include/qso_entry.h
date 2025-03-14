#pragma once

#include "fields.h"

#include <map>
#include <vector>
#include <set>

#include <FL/Fl_Group.H>

class qso_data;
class record;
class field_choice;
class field_input;
class Fl_Input;
typedef size_t qso_num_t;


// A class that allows the entry of a QSO
class qso_entry :
    public Fl_Group
{

public:
	// Bitwise flags to define which fields get copied from 1 QSO to another
	// See below for which fields get copied
	enum copy_flags : int {
		CF_NONE = 0,
		CF_RIG_ETC = 1,
		CF_CAT = 2,
		CF_TIME = 4,
		CF_CALL = 8,
		CF_DETAILS = 16,
		CF_REPORTS = 32,
		CF_DATE = 64,
		CF_CONTACT = CF_CALL | CF_DETAILS,
		CF_QSO = CF_RIG_ETC | CF_CAT | CF_TIME | CF_DATE | CF_CONTACT,
		CF_COPY = CF_RIG_ETC | CF_CAT | CF_DATE, 
		CF_ALL_FLAGS = -1
	};

	// Map indicating which fields get copied per the above flags
	const map < copy_flags, set<string> > COPY_FIELDS =
	{
		{ CF_RIG_ETC, { "MY_RIG", "MY_ANTENNA", "STATION_CALLSIGN", "APP_ZZA_QTH", "APP_ZZA_OP" } },
		{ CF_CAT, { "MODE", "FREQ", "SUBMODE", "TX_PWR", "BAND" } },
		{ CF_DATE, { "QSO_DATE" } },
		{ CF_TIME, { "QSO_DATE_OFF", "TIME_ON", "TIME_OFF" } },
		{ CF_CALL, { "CALL" }},
		{ CF_DETAILS, { "NAME", "QTH", "DXCC", "STATE", "CNTY", "GRIDSQUARE", "CQZ", "ITUZ", "NOTES" } },
		{ CF_REPORTS, { "RST_SENT", "RST_RCVD", "SRX", "STX" }}
	};

	// The set of all copy flags
	const set < copy_flags > COPY_SET = { CF_RIG_ETC, CF_CAT, CF_DATE, CF_TIME, CF_CALL, CF_DETAILS, CF_REPORTS };

	// Loggable field names
	static const int NUMBER_FIXED = 4;
	static const int NUMBER_TOTAL = NUMBER_FIXED + 20;

	// Variable fields - per usage
	static collection_t* field_map_;

protected:
	// Maps fieldname to index of the Input used for its value
	map<string, int> field_ip_map_;
	// Used field list
	field_list fields_in_use_;

public:
    qso_entry(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_entry();

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();

	// Handle override
	int handle(int event);
	// Clock ticker
	static void cb_ticker(void* v);

	// Set/get records
	record* qso();
	void qso(qso_num_t qso_number);
	void qso(record* qso);
	// Return the original QSO data (pre-edit)
	record* original_qso();
	// The current QSO number
	qso_num_t qso_number();

	// Append QSO to book
	void append_qso();
	// Delete qso
	void delete_qso();

	// Copy from an existing record
	void copy_qso_to_qso(record* old_record, int flags);
	// Copy fields from CAT
	void copy_cat_to_qso(bool clear = false);
	// Copy clock to QSO
	void copy_clock_to_qso();
	// Copy default value to QSO
	void copy_default_to_qso();
	// Copy contest defaults to QSO
	void copy_contest_to_qso();
	// Initialise fields
	void initialise_field_map();
	// Get defined fields
	string get_defined_fields();
	// Clear display fields
	void clear_display();
	// Clear QSO fields
	void clear_qso();
	// Copy fields from record
	void copy_qso_to_display(int flags);
	// Update rig
	void update_rig();
	// Set focus on call
	void set_focus_call();
	// Save focus
	void save_focus(Fl_Widget* w);

	// Action add field to widgets
	void action_add_field(int ix, string field);
	// Action delete field
	void action_del_field(int ix);

	// Initialise fields
	void initialise_fields();

protected:
	// Field input - v: field name
	static void cb_ip_field(Fl_Widget* w, void* v);
	// Field choice - v: widget no. X1 to X7
	static void cb_ch_field(Fl_Widget* w, void* v);
	// Notes input field
	static void cb_ip_notes(Fl_Widget* w, void* v);
	// Set focus to saved 
	void set_focus_saved();



protected:
	// Notes
	Fl_Input* ip_notes_;
	// Field choices
	field_choice* ch_field_[NUMBER_TOTAL];
	// field inputs
	field_input* ip_field_[NUMBER_TOTAL];
	// Parent qso_data
	qso_data* qso_data_;
	// Number of locked fields
	int number_locked_;
	// Records
	record* qso_;
	// A copy of the original QSO before any editing
	record* original_qso_;
	// Current QSO number
	qso_num_t qso_number_;
	// Previous value
	string previous_qth_;
	string previous_locator_;
	// Previous contest serial number
	int previous_serial_;
	// Input index that should get focus on showing
	static int focus_ix_;
};

