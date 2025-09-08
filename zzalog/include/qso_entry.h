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


//! This class provides the abiity to view and edit a QSO.

//! It has a number of choice widgets to select fields to view or edit,
//! and inputs for the values of these fields.
class qso_entry :
    public Fl_Group
{

public:
	//! Bitwise flags to define which fields get copied from 1 QSO to another.
	enum copy_flags : int {
		CF_NONE = 0,        //!< No fields.
		CF_RIG_ETC = 1,     //!< Station-related fields.
		CF_CAT = 2,         //!< Rig-related fields.
		CF_TIME = 4,        //!< Time fields.
		CF_CALL = 8,        //!< Callsign.
		CF_DETAILS = 16,    //!< QSO details.
		CF_REPORTS = 32,    //!< Reports, including contest exchanges.
		CF_DATE = 64,       //!< Start date.
		CF_CONTACT = CF_CALL | CF_DETAILS,
		                    //!< CF_CALL | CF_DETAILS
		CF_QSO = CF_RIG_ETC | CF_CAT | CF_TIME | CF_DATE | CF_CONTACT,
		                    //!< CF_RIG_ETC | CF_CAT | CF_TIME | CF_DATE | CF_CONTACT
		CF_COPY = CF_RIG_ETC | CF_CAT | CF_DATE, 
		                    //!< CF_RIG_ETC | CF_CAT | CF_DATE
		CF_ALL_FLAGS = -1   //!< All fields.
	};

	//! Map copy_flags to the specific fields being copied.
	const map < copy_flags, set<string> > COPY_FIELDS =
	{
		{ CF_RIG_ETC, { "MY_RIG", "MY_ANTENNA", "STATION_CALLSIGN" } },
		{ CF_CAT, { "MODE", "FREQ", "SUBMODE", "TX_PWR", "BAND" } },
		{ CF_DATE, { "QSO_DATE" } },
		{ CF_TIME, { "QSO_DATE_OFF", "TIME_ON", "TIME_OFF" } },
		{ CF_CALL, { "CALL" }},
		{ CF_DETAILS, { "NAME", "QTH", "DXCC", "STATE", "CNTY", "GRIDSQUARE", "CQZ", "ITUZ", "NOTES" } },
		{ CF_REPORTS, { "RST_SENT", "RST_RCVD", "SRX", "STX" }}
	};

	//! The set of all copy flags
	const set < copy_flags > COPY_SET = { CF_RIG_ETC, CF_CAT, CF_DATE, CF_TIME, CF_CALL, CF_DETAILS, CF_REPORTS };

	// Loggable field names
	static const int NUMBER_FIXED = 2;                 //!< Fields that cannot be changed.
	static const int NUMBER_TOTAL = NUMBER_FIXED + 20; //!< Total number of fields that can be displayed.

	//! List of fields that are displayed: this is common for all instances.
	static field_list* field_map_;

protected:
	//! Maps fieldname to the index of the input used for its value.
	map<string, int> field_ip_map_;
	//! List of fields that are displayed.
	field_list fields_in_use_;

public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_entry(int X, int Y, int W, int H, const char* L = nullptr);
	//! Desructor.
    ~qso_entry();

	//! Instantiate component widgets.
	void create_form(int X, int Y);
	//! Configure component widgets after data change.
	void enable_widgets();

	//! Override Fl_Group::handle to accept focus so that keyboard F1 opens userguide.
	virtual int handle(int event);
	//! Callback every 1 second to update time and data from rig when entering real-time QSO records.
	static void cb_ticker(void* v);

	//! Returns current QSO record.
	record* qso();
	//! Set the current QSO from its index in full log.
	void qso(qso_num_t qso_number);
	//! Set the current QSO, if not yet in book.
	void qso(record* qso);
	//! Returns the original QSO data (pre-edit)
	record* original_qso();
	//! Returns the current QSO number
	qso_num_t qso_number();

	//! Add current QSO record to book.
	void append_qso();
	//! Delete the QSO record.
	void delete_qso();

	//! Copy fields in \p flags from an existing record \p old_record.
	void copy_qso_to_qso(record* old_record, int flags);
	//! Copy fields from CAT: if \p clear overwrite existing values.
	void copy_cat_to_qso(bool clear = false);
	//! Copy clock to QSO record.
	void copy_clock_to_qso();
	//! Copy default value to QSO record.
	void copy_default_to_qso();
	//! Copy contest defaults to QSO record.
	void copy_contest_to_qso();
	//! Initialise which fields are processed by which pair of widgets (choice and input).
	void initialise_field_map();
	//! Returns fields in use as a comma-separated list.
	string get_defined_fields();
	//! Clear all input widgets for field values.
	void clear_display();
	//! Clear fields in current QSO record and copy itt o the display.
	void clear_qso();
	//! Copy fields indicated by \p flags from record
	void copy_qso_to_display(int flags);
	//! Update station details in QSO and copy to display.
	void update_rig();
	//! Set focus on callsign input widget.
	void set_focus_call();
	//! Save current focus in \p w.
	void save_focus(Fl_Widget* w);

	//! Add the supplied \p field to the display at index \p ix.
	void action_add_field(int ix, string field);
	//! Remove the field at index \p ix of the display.
	void action_del_field(int ix);

	//! Initialise fields.
	
	//! Configure the widgets in the display:
	//! - Set the field choice to the required name.
	//! - Configure the field value input, populating its drop-down menu 
	//! as necessary.
	void initialise_fields();
	//! Restore focus to saved widget. 
	void set_focus_saved();

protected:
	//! Callback from selecting field input value: \p v provides index in field list.
	static void cb_ip_field(Fl_Widget* w, void* v);
	//! Callback from selecting field choice: \p v provides index in field list.
	static void cb_ch_field(Fl_Widget* w, void* v);
	//! Callback from "NOTES" input.
	static void cb_ip_notes(Fl_Widget* w, void* v);



protected:
	//! Input "NOTES" field value.
	Fl_Input* ip_notes_;
	//! Choices: For the field names.
	field_choice* ch_field_[NUMBER_TOTAL];
	//! Inputs: For the field values.
	field_input* ip_field_[NUMBER_TOTAL];
	//! Parent qso_data
	qso_data* qso_data_;
	//! Number of locked fields
	int number_locked_;
	//! Current QSO record.
	record* qso_;
	//! A copy of the original QSO record before any editing
	record* original_qso_;
	//! Current QSO number
	qso_num_t qso_number_;
	//! Previous value of QTH.
	string previous_qth_;
	//! Previous value of locator.
	string previous_locator_;
	//! Previous contest serial number
	int previous_serial_;
	//! Index of the input that should get focus when redrawn.
	static int focus_ix_;
};

