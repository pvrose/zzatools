#include "qso_manager.h"
#include "qso_data.h"
#include "qso_log.h"
#include "qso_misc.h"
#include "qso_tabbed_rigs.h"
#include "qso_clocks.h"
#include "qso_buttons.h"
#include "qso_qsl.h"
#include "callback.h"

#include "record.h"
#include "status.h"
#include "tabbed_forms.h"
#include "intl_widgets.h"
#include "spec_data.h"
#include "book.h"
#include "extract_data.h"
#include "import_data.h"
#include "menu.h"
#include "field_choice.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "utils.h"
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
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern spec_data* spec_data_;
extern book* book_;
extern extract_data* extract_records_;
extern import_data* import_data_;
extern book* navigation_book_;
extern menu* menu_;
extern tabbed_forms* tabbed_forms_;
extern import_data* import_data_;
extern qrz_handler* qrz_handler_;
extern main_window* main_window_;
extern bool closing_;
extern string COPYRIGHT;
extern string CONTACT;
extern string VENDOR;
extern string PROGRAM_ID;

// The main dialog constructor
qso_manager::qso_manager(int W, int H, const char* label) :
	Fl_Double_Window(W, H, label)
	, rig_group_(nullptr)
	, created_(false)
{
	callback(cb_close);
	load_values();
	create_form(0,0);
	update_rig();
	// update_qso(HT_SELECTED, book_->selection(), -1);
	change_rig(get_default(RIG));

}

// Destructor
qso_manager::~qso_manager()
{
	save_values();
}

// Handle FL_HIDE and FL_SHOW to get menu to update itself - 
// Handle CTRL-V to paste clipboard into log
int qso_manager::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		menu_->update_windows_items();
		break;
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

	return Fl_Double_Window::handle(event);
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

	// QSO Data form
	data_group_ = new qso_data(curr_x, curr_y, 50, 50, nullptr);
	data_group_->create_form(curr_x, curr_y);
	curr_x += data_group_->w();
	curr_y += data_group_->h();
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = X + GAP;
	curr_y += GAP;

	Fl_Group::current(this);
	// Rig control tabbed form
	rig_group_ = new qso_tabbed_rigs(curr_x, curr_y, 50, 50, nullptr);

	curr_x += rig_group_->w() + GAP;

	// Clocks tabbed form
	clock_group_ = new qso_clocks(curr_x, curr_y, 50, 50, nullptr);
	curr_x += clock_group_->w() + GAP;

	// Information tabbed form
	info_group_ = new qso_log(curr_x, curr_y, 50, 50, nullptr);
	curr_x += info_group_->w() + GAP;

	curr_y += max(
		max(clock_group_->h(), rig_group_->h()),
		info_group_->h());

	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	Fl_Box* b_cr = new Fl_Box(x(), max_y, max_x + GAP - x(), FOOT_HEIGHT);
	b_cr->copy_label(string(COPYRIGHT + " " + CONTACT + "     ").c_str());
	b_cr->labelsize(FL_NORMAL_SIZE - 1);
	b_cr->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

	this->resizable(nullptr);
	// Now position it within the screen
	int rw = max_x + GAP - X;
	int rh = max_y + FOOT_HEIGHT - Y;
	int rx = x_root();
	int ry = y_root();
	int sx, sy, sw, sh;
	Fl::screen_work_area(sx, sy, sw, sh);
	if (rx < sx) rx = sx;
    else if (rx + rw > sx + sw) rx = sx + sw - rw;
	if (ry < sy) ry = sy;
	else if (ry + rh > sy + sh) ry = sy + sh - rh;
	this->resize(rx, ry, rw, rh);

	b_cr->box(FL_FLAT_BOX);

	created_ = true;

	end();
	show();

	enable_widgets();
}

// Load values
void qso_manager::load_values() {
	// Get position of window
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences windows_settings(settings, "Windows");
	Fl_Preferences dash_settings(windows_settings, "Dashboard");
	int left, top;
	dash_settings.get("Left", left, 0);
	dash_settings.get("Top", top, 100);
	position(left, top);
}

// Write values back to settings - write the three groups back separately
void qso_manager::save_values() {

	// Save window position
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences windows_settings(settings, "Windows");
	Fl_Preferences dash_settings(windows_settings, "Dashboard");
	dash_settings.set("Left", x_root());
	dash_settings.set("Top", y_root());

	data_group_->save_values();
	rig_group_->save_values();
	info_group_->save_values();
}

// Enable the widgets - activate the group associated with each rig handler when that
// handler is enabled
void qso_manager::enable_widgets() {

	// Not all widgets may exist yet!
	if (!created_) return;

	rig_group_->enable_widgets();
	info_group_->enable_widgets();
	data_group_->enable_widgets();

}

// Close button clicked - invoke main window close
// v is not used
void qso_manager::cb_close(Fl_Widget* w, void* v) {
	// It is the window that raised this callback
	qso_manager* that = (qso_manager*)w;
	// If we are editing does the user want to save or cancel?
	if (that->editing()) {
		if (fl_choice("Modifying a record - save or cancel?", "Cancel", "Save", nullptr) == 1) {
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
	record* edit_qso;
	switch (data_group_->logging_state()) {
	case qso_data::NET_STARTED:
	case qso_data::NET_ADDING:
	case qso_data::NET_EDIT:
	case qso_data::QSO_WSJTX:
	case qso_data::QSO_FLDIGI:
		return true;
	case qso_data::QSO_EDIT:
	case qso_data::QSO_STARTED:
	case qso_data::QSO_ENTER:
		edit_qso = data_group_->current_qso();
		return book_->is_dirty_record(edit_qso);
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
	case qso_data::QSO_VIEW:
	case qso_data::QSO_ENTER:
		return false;
	case qso_data::QSO_STARTED:
	case qso_data::QSO_WSJTX:
	case qso_data::QSO_FLDIGI:
	case qso_data::NET_STARTED:
		return true;
	default:
		return false;
	}
}

// Switch the rig connection on or off
void qso_manager::switch_rig() {
	rig_group_->switch_rig();
}

// Get QSO information from previous record not rig
void qso_manager::update_rig() {
	data_group_->update_rig();
	info_group_->enable_widgets();
}

// Change rig
void qso_manager::change_rig(string rig_name) {
	bool change = false;
	if (!rig_group_->value()) change = true;
	else {
		const char* l = rig_group_->value()->label();
		if (l == nullptr || strcmp(rig_name.c_str(), l)) {
			change = true;
		}
	}
	if (change) {
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

// Get rig control
qso_rig* qso_manager::rig_control() {
	if (rig_group_) return (qso_rig*)rig_group_->value();
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
		if (data_group_->logging_state() == qso_data::QSO_INACTIVE) {
			data_group_->action_view();
		}
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
	case HT_IMPORT_QUERYSWL:
		data_group_->update_query(qso_data::QUERY_SWL, match_num, query_num);
		break;
	default:
		break;
	}
	// // Set this window to the top
	// show();
}

// Start QSO
void qso_manager::start_qso() {
	data_group_->start_qso(qso_data::QSO_AS_WAS);
}

// End QSO - add time off
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
	if (import_qso) {
		string mode = import_qso->item("MODE");
		string submode = import_qso->item("SUBMODE");
		// If we have a QSO to copy from and one to copy to, copy these fields.
		if (import_qso->item("MY_RIG").length() == 0)
			import_qso->item("MY_RIG", get_default(RIG));
		if (import_qso->item("MY_ANTENNA").length() == 0) 
			import_qso->item("MY_ANTENNA", get_default(ANTENNA));
	}
}

// Get the default value of the station item
string qso_manager::get_default(stn_item_t item) {
	record* source = data_group_->current_qso();
	// If there isn't a default QSO the use the latest one
	if (source == nullptr || source->size() == 0) {
		source = book_->get_latest();
	}
	if (source) {
		switch (item) {
		case RIG:
			return source->item("MY_RIG");
		case ANTENNA:
			return source->item("MY_ANTENNA");
		case CALLSIGN:
		case QTH:
		case OP:
			return data()->get_default_station(item);
		default:
			return "";
		}
	}
	else {
		return "";
	}
}

// Return current QSO
qso_data* qso_manager::data() {
	return data_group_;
}

// Update book with a QSO from the modem app
void qso_manager::update_modem_qso(bool log_it) {
	//cout << "Received record from modem:" << qso << endl;
	//adi_writer::to_adif(qso, cout);
	//cout << endl;
	data_group_->update_modem_qso(log_it);
}

// Start a modem QSO
record* qso_manager::start_modem_qso(string call, uchar source) {
	return data_group_->start_modem_qso(call, (qso_data::qso_init_t)source);
}

// Enter a modem QSO
void qso_manager::enter_modem_qso(record * qso) {
	data_group_->enter_modem_qso(qso);
}

// Cancel modem QSO
void qso_manager::cancel_modem_qso() {
	data_group_->action_cancel_modem();
	enable_widgets();
}

// Download QSLs;
void qso_manager::qsl_download(uchar server) {
	qsl_control()->qsl_download((import_data::update_mode_t)server);
}

// Extract QSLs;
void qso_manager::qsl_extract(uchar server) {
	qsl_control()->qsl_extract((extract_data::extract_mode_t)server);
}

// Upload QSLs;
void qso_manager::qsl_upload() {
	qsl_control()->qsl_upload();
}

// Print QSLs
void qso_manager::qsl_print() {
	qsl_control()->qsl_print();
}

// E-mail QSLs
void qso_manager::qsl_email() {
	qsl_control()->qsl_generate_png();
	qsl_control()->qsl_send_email();
}

// Mark QSL print done
void qso_manager::qsl_print_done() {
	qsl_control()->qsl_mark_done();
}

qso_qsl* qso_manager::qsl_control() {
	return info_group_->qsl_control();
}

qso_log_info* qso_manager::log_info() {
	return info_group_->log_info();
}

// Deactivate any QSO entry and rigs to 
void qso_manager::deactivate_all() {
	data_group_->action_deactivate();
	rig_group_->deactivate_rigs();
}

// Merge QRZ.com data
void qso_manager::merge_qrz_com() {
	data_group_->action_qrz_com();
}

