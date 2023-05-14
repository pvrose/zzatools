#include "qso_manager.h"
#include "callback.h"
#include "rig_if.h"

#include "record.h"
#include "pfx_data.h"
#include "prefix.h"
#include "status.h"
#include "tabbed_forms.h"
#include "intl_widgets.h"
#include "spec_data.h"
#include "book.h"
#include "extract_data.h"
#include "import_data.h"
#include "menu.h"
#include "field_choice.h"
#ifdef _WIN32
#include "dxa_if.h"
#endif
#include "band_view.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "alarm_dial.h"
#include "qth_dialog.h"
#include "utils.h"
#include "qsl_viewer.h"
#include "qrz_handler.h"
#include "main_window.h"
#include "adi_writer.h"

#include <set>
#include <iostream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Output.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Tooltip.H>




// External declarations
extern Fl_Preferences* settings_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern spec_data* spec_data_;
extern book* book_;
extern extract_data* extract_records_;
extern import_data* import_data_;
extern book* navigation_book_;
extern menu* menu_;
#ifdef _WIN32
extern dxa_if* dxa_if_;
#endif
extern band_view* band_view_;
extern tabbed_forms* tabbed_forms_;
extern import_data* import_data_;
extern qrz_handler* qrz_handler_;
extern main_window* main_window_;
extern bool read_only_;
extern bool closing_;

// The main dialog constructor
qso_manager::qso_manager(int W, int H, const char* label) :
	Fl_Window(W, H, label)
	, rig_group_(nullptr)
	, created_(false)
	, ticker_in_progress_(false)
{
	callback(cb_close);
	load_values();
	create_form(0,0);
	update_rig();
	update_qso(HT_SELECTED, book_->selection(), -1);

}

// Destructor
qso_manager::~qso_manager()
{
	save_values();
}

// Handle FL_HIDE and FL_SHOW to get menu to update itself
int qso_manager::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		menu_->update_windows_items();
		return true;
	case FL_KEYBOARD:
		// This 
		switch (Fl::event_key()) {
		case 'v':
			// CTRL-V
			if (Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R)) {
				// Treat as paste clipboard
				Fl::paste(*main_window_, 1);
				return true;
			}
			break;
		}
	}

	return Fl_Window::handle(event);
}

// create the form
void qso_manager::create_form(int X, int Y) {

	// Used to evaluate total width and height of the window
	int max_x = X;
	int max_y = Y;
	// Used to maintain the relative positions of the groups
	int curr_x = X + GAP;
	int curr_y = Y + GAP;

	begin();

	data_group_ = new qso_data(curr_x, curr_y, 0, 0, nullptr);
	data_group_->create_form(curr_x, curr_y);
	curr_x += data_group_->w();
	curr_y += data_group_->h();
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = X + GAP;
	curr_y += GAP;
	int save_y = curr_y;

	rig_group_ = new qso_tabbed_rigs(curr_x, curr_y, 0, 0, nullptr);

	curr_x += rig_group_->w() + GAP;

	clock_group_ = new qso_clock(curr_x, curr_y, 0, 0, nullptr);
	clock_group_->create_form(curr_x, curr_y);
	curr_x += clock_group_->w() + GAP;

	curr_y += max(clock_group_->h(), rig_group_->h());

	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	this->resizable(nullptr);
	this->size(max_x + GAP - X, max_y + GAP - Y);
	created_ = true;

	end();
	show();

	enable_widgets();
}

// Load values
void qso_manager::load_values() {
	// Get position of window
	int top;
	int left;
	Fl_Preferences dash_settings(settings_, "Dashboard");
	dash_settings.get("Left", left, 100);
	dash_settings.get("Top", top, 100);
	if (top < 0) top = 100;
	if (left < 0) left = 100;
	position(left, top);
}

// Write values back to settings - write the three groups back separately
void qso_manager::save_values() {

	// Save window position
	Fl_Preferences dash_settings(settings_, "Dashboard");
	dash_settings.set("Left", x_root());
	dash_settings.set("Top", y_root());

//	Fl_Preferences stations_settings(settings_, "Stations");
	data_group_->save_values();
	rig_group_->save_values();
	clock_group_->save_values();
}

// Enable the widgets - activate the group associated with each rig handler when that
// handler is enabled
void qso_manager::enable_widgets() {

	// Not all widgets may exist yet!
	if (!created_) return;

	data_group_->enable_widgets();
	rig_group_->enable_widgets();
}

// Close button clicked - invoke main window close
// v is not used
void qso_manager::cb_close(Fl_Widget* w, void* v) {
	// It is the window that raised this callback
	qso_manager* that = (qso_manager*)w;
	// If we are editing does the user want to save or cancel?
	if (that->data_group_->logging_state() == qso_data::qso_data::QSO_EDIT ||
		that->data_group_->logging_state() == qso_data::qso_data::QSO_STARTED) {
		if (fl_choice("Entering a record - save or cancel?", "Cancel", "Save", nullptr) == 1) {
			qso_buttons::cb_save(that->data_group_, v);
		}
		else {
			qso_buttons::cb_cancel(that->data_group_, v);
		}
	}
	// Call the main window callback to close the app.
	that->close_by_dash_ = true;
	main_window_->do_callback();
}

// Return that we are currently editing or creating ot QSO or net
bool qso_manager::editing() {
	switch (data_group_->logging_state()) {
	case qso_data::NET_STARTED:
	case qso_data::NET_ADDING:
	case qso_data::NET_EDIT:
	case qso_data::QSO_EDIT:
	case qso_data::QSO_STARTED:
		return true;
	default:
		return false;
	}
}

// The QSO number being selected is outwith that being edited
bool qso_manager::outwith_edit(qso_num_t number) {
	return !(data_group_->qso_editing(number));
}

// Return true if we have a current QSO
bool qso_manager::qso_in_progress() {
	switch (data_group_->logging_state()) {
	case qso_data::QSO_PENDING:
	case qso_data::QSO_INACTIVE:
	case qso_data::QSO_EDIT:
		return false;
	case qso_data::QSO_STARTED:
		return true;
	}
	return false;
}

// Switch the rig connection on or off
void qso_manager::switch_rig() {
	rig_group_->switch_rig();
}

// Get QSO information from previous record not rig
void qso_manager::update_rig() {
	data_group_->update_rig();
}

// Change rig
void qso_manager::change_rig(string rig_name) {
	if (rig_group_->label() == nullptr || strcmp(rig_name.c_str(), rig_group_->label())) {
		// Change the rig's name and reload the connection data
		rig_group_->copy_label(rig_name.c_str());
		//rig_group_->load_values();
		rig_group_->switch_rig();
	}
}

// Get rig
rig_if* qso_manager::rig() {
	if (rig_group_) return rig_group_->rig();
	else return nullptr;
}

// Called whenever another view updates a record (or selects a new one)
void qso_manager::update_qso(hint_t hint, qso_num_t match_num, qso_num_t query_num) {
	record* target = book_->get_record(match_num, false);
	switch (hint) {
	case HT_SELECTED:
	case HT_ALL:
	case HT_CHANGED:
	case HT_DELETED:
	case HT_INSERTED:
	case HT_INSERTED_NODXA:
	case HT_MINOR_CHANGE:
	case HT_NEW_DATA:
	case HT_RESET_ORDER:
		data_group_->update_qso(match_num);
		if (target != nullptr) {
			change_rig(book_->get_record(match_num, false)->item("MY_RIG"));
		}
		break;
	case HT_IMPORT_QUERY:
		data_group_->update_query(qso_data::QUERY_MATCH, match_num, query_num);
		break;
	case HT_IMPORT_QUERYNEW:
		data_group_->update_query(qso_data::QUERY_NEW, match_num, query_num);
		break;
	case HT_DUPE_QUERY:
		data_group_->update_query(qso_data::QUERY_DUPE, match_num, query_num);
		break;
	case HT_MERGE_DETAILS:
		data_group_->update_query(qso_data::QRZ_MERGE, match_num, query_num);
	}
	// Set this window to the top
	show();
}

// Start QSO
void qso_manager::start_qso() {
	data_group_->start_qso(qso_data::QSO_AS_WAS);
}

// End QSO - add time off
// TODO: Can be called without current_qso_ - needs to be set by something.
void qso_manager::end_qso() {
	data_group_->end_qso();
}

// Edit QSO
void qso_manager::edit_qso() {
	data_group_->edit_qso();
}

// Dummy QSO - only current date and time
record* qso_manager::dummy_qso() {
	return data_group_->dummy_qso();
}

// Copy the sleected QSOs MY_RIG etc to the supplied qso record
void qso_manager::update_import_qso(record* import_qso) {
	record* use_qso;
	string mode = import_qso->item("MODE");
	string submode = import_qso->item("SUBMODE");
	// Get the default record for copying
	use_qso = book_->get_record(data_group_->get_default_number(), false);
	// If we have a QSO to copy from and one to copy to, copy these fields.
	if (use_qso && import_qso) {
		if (import_qso->item("MY_RIG").length() == 0) 
			import_qso->item("MY_RIG", use_qso->item("MY_RIG"));
		if (import_qso->item("MY_ANTENNA").length() == 0) 
			import_qso->item("MY_ANTENNA", use_qso->item("MY_ANTENNA"));
		if (import_qso->item("STATION_CALLSIGN").length() == 0) 
			import_qso->item("STATION_CALLSIGN", use_qso->item("STATION_CALLSIGN"));
		if (import_qso->item("APP_ZZA_QTH").length() == 0) 
			import_qso->item("APP_ZZA_QTH", use_qso->item("APP_ZZA_QTH"));
	}
}

// Get the default value of the station item
string qso_manager::get_default(stn_item_t item) {
	record* source = data_group_->current_qso();
	// If there isn't a default QSO the use the latest one
	if (source == nullptr) {
		source = book_->get_latest();
	}
	if (source) {
		switch (item) {
		case RIG:
			return source->item("MY_RIG");
		case ANTENNA:
			return source->item("MY_ANTENNA");
		case CALLSIGN:
			return source->item("STATION_CALLSIGN");
		case QTH:
			return source->item("APP_ZZA_QTH");
		default:
			return "";
		}
	}
	else {
		return "";
	}
}

// Handle 1 second timer
void qso_manager::ticker() {
	if (!ticker_in_progress_ && !closing_) {
		ticker_in_progress_ = true;
		rig_group_->ticker();
		data_group_->ticker();
		ticker_in_progress_ = false;
	}
}

// Stop 1 second ticker
void qso_manager::stop_ticker() {
	clock_group_->stop_ticker();
}

// Return current QSO
qso_data* qso_manager::data() {
	return data_group_;
}

// Update book with a QSO from the modem app
void qso_manager::update_modem_qso(record* qso) {
	cout << "Received record from modem:" << qso << endl;
	adi_writer::to_adif(qso, cout);
	cout << endl;
	data_group_->update_modem_qso(qso);
}