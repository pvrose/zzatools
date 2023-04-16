#pragma once

#include "book.h"
#include "field_choice.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>

class qso_data;

class qso_entry :
    public Fl_Group
{

public:

	enum copy_flags : int {
		CF_NONE = 0,
		CF_RIG_ETC = 1,
		CF_CAT = 2,
		CF_TIME = 4,
		CF_CONTACT = 8,
		CF_REPORTS = 16,
		CF_QSO = CF_RIG_ETC | CF_CAT | CF_TIME | CF_CONTACT,
		CF_ALL_FLAGS = -1
	};

	const map < copy_flags, set<string> > COPY_FIELDS =
	{
		{ CF_RIG_ETC, { "MY_RIG", "MY_ANTENNA", "STATION_CALLSIGN", "APP_ZZA_QTH" } },
		{ CF_CAT, { "MODE", "FREQ", "SUBMODE", "TX_PWR", "BAND"} },
		{ CF_TIME, { "QSO_DATE", "QSO_DATE_OFF", "TIME_ON", "TIME_OFF" } },
		{ CF_CONTACT, { "CALL", "NAME", "QTH", "DXCC", "STATE", "CNTY", "GRIDSQUARE", "CQZ", "ITUZ" } },
		{ CF_REPORTS, { "RST_SENT", "RST_RCVD", "SRX", "STX" }}
	};

	const set < copy_flags > COPY_SET = { CF_RIG_ETC, CF_CAT, CF_TIME, CF_CONTACT, CF_REPORTS };

	// Loggable field names
	static const int NUMBER_FIXED = 10;
	static const int NUMBER_TOTAL = NUMBER_FIXED + 12;
protected:
	const string fixed_names_[NUMBER_FIXED] = {
		"MY_RIG", "MY_ANTENNA", "APP_ZZA_QTH", "STATION_CALLSIGN",
		"QSO_DATE", "TIME_ON", "CALL", "FREQ",
		"MODE", "TX_PWR" };
	map<string, int> field_ip_map_;
	string field_names_[NUMBER_TOTAL];
	int number_fields_in_use_;

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

	// Copy from an existing record
	void copy_qso_to_qso(record* old_record, int flags);
	// Copy fields from CAT
	void copy_cat_to_qso();
	// Copy clock to QSO
	void copy_clock_to_qso();
	// Update rig in QSO
	void update_rig();
	// Initialise fields
	void initialise_fields();
	// Get defined fields
	string get_defined_fields();
	// Clear display fields
	void clear_display();
	// Clear QSO fields
	void clear_qso();
	// Copy fields from record
	void copy_qso_to_display(int flags);

	// Action add field to widgets
	void action_add_field(int ix, string field);
	// Action delete field
	void action_del_field(int ix);

	// Get callsign
	string get_call();

	// Initialise fields
	void initialise_fields(string fields, bool new_fields, bool lock_preset);
	void initialise_values(string fields, int contest_serial);


protected:
	// Field input - v: field name
	static void cb_ip_field(Fl_Widget* w, void* v);
	// Field choice - v: widget no. X1 to X7
	static void cb_ch_field(Fl_Widget* w, void* v);
	// Notes input field
	static void cb_ip_notes(Fl_Widget* w, void* v);



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

};

