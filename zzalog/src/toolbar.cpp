#include "toolbar.h"
#include "menu.h"
#include "import_data.h"
#include "icons.h"
#include "callback.h"
#include "book.h"
#include "tabbed_forms.h"
#include "pfx_data.h"
#include "utils.h"
#include "status.h"
#include "drawing.h"
#include "intl_widgets.h"
#include "extract_data.h"
#include "main_window.h"
#include "qso_manager.h"
#include "import_data.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Single_Window.H>




extern menu* menu_;
extern book* navigation_book_;
extern book* book_;
extern extract_data* extract_records_;
extern tabbed_forms* tabbed_forms_;
extern pfx_data* pfx_data_;
extern status* status_;
extern main_window* main_window_;
extern qso_manager* qso_manager_;
extern import_data* import_data_;

// Constructor - most buttons invoke a menu item
toolbar::toolbar(int X, int Y, int W, int H, const char* label) :
	Fl_Group(X, Y, W, H, label)
	, search_text_("")
	, record_num_(0)
	, ip_search_(nullptr)
	, min_w_(0)
{
	int curr_x = X;
	// File->New
	Fl_Button* bn = new Fl_Button(curr_x, Y, H, H, "@filenew");
	bn->callback(cb_bn_menu, (void*)"&File/&New");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("New file");
	add(bn);
	curr_x += H;
	// File->Open
	bn = new Fl_Button(curr_x, Y, H, H, "@fileopen");
	bn->callback(cb_bn_menu, (void*)"&File/&Open");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Open file");
	add(bn);
	curr_x += H;
	// File->Save
	bn = new Fl_Button(curr_x, Y, H, H, "@filesave");
	bn->callback(cb_bn_menu, (void*)"&File/&Save");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Save file");
	add(bn);
	curr_x += H;
	// File->Save As
	bn = new Fl_Button(curr_x, Y, H, H, "@filesaveas");
	bn->callback(cb_bn_menu, (void*)"&File/Save &As");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Save file as");
	add(bn);
	curr_x += H + TOOL_GAP;
	// Navigate->First
	bn = new Fl_Button(curr_x, Y, H, H, "@$->|");
	bn->callback(cb_bn_menu, (void*)"&Navigate/&First");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Go to first record");
	add(bn);
	curr_x += H;
	// Navigate->Previous
	bn = new Fl_Button(curr_x, Y, H, H, "@<-");
	bn->callback(cb_bn_menu, (void*)"&Navigate/&Previous");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Go to previous record");
	add(bn);
	curr_x += H;
	// Navigate->Next
	bn = new Fl_Button(curr_x, Y, H, H, "@->");
	bn->callback(cb_bn_menu, (void*)"&Navigate/Ne&xt");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Go to next record");
	add(bn);
	curr_x += H;
	// Navigate->Last
	bn = new Fl_Button(curr_x, Y, H, H, "@->|");
	bn->callback(cb_bn_menu, (void*)"&Navigate/&Last");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Go to last record");
	add(bn);
	curr_x += H;
	// Navigate->Find->New
	bn = new Fl_Button(curr_x, Y, H, H, "@search");
	bn->labelcolor(FL_BLUE);
	bn->callback(cb_bn_menu, (void*)"&Navigate/F&ind/&New");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Go to matching record");
	curr_x += H;
	// Navigate->Find->Next
	bn = new Fl_Button(curr_x, Y, H, H, "@>>");
	bn->labelcolor(FL_BLUE);
	bn->callback(cb_bn_menu, (void*)"&Navigate/F&ind/Ne&xt");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Go to next matching record");
	curr_x += H + TOOL_GAP;
	// Log->New Record
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Log/&New record");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_NEW_QSO, 16, 16, 4));
	bn->tooltip("Start new QSO");
	add(bn);
	curr_x += H;
	// Log->Save Record
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Log/&Save record");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_SAVE_QSO, 16, 16, 4));
	bn->tooltip("Save QSO");
	add(bn);
	curr_x += H;
	// Log->Cancel
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Log/&Cancel");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_CNCL_QSO, 16, 16, 4));
	bn->tooltip("Cancel QSO capture or edit");
	add(bn);
	curr_x += H;
	// Log->Delete Record
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Log/&Delete record");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_DEL_QSO, 16, 16, 4));
	bn->tooltip("Delete QSO record");
	add(bn);
	curr_x += H;
	// Log->Retime Record
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Log/Re&time record");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_RETIME_QSO, 16, 16, 4));
	bn->tooltip("Reset QSO TIME_OFF");
	add(bn);

	curr_x += H + TOOL_GAP;
	// Import->Download->eQSL
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Import/Download e&QSL");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_DL_EQSL, 16, 16, 4));
	bn->tooltip("Download and import eQSL records");
	add(bn);
	curr_x += H;
	// Import->Download->LotW
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Import/Download &LotW");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_DL_LOTW, 16, 16, 4));
	bn->tooltip("Download and import LotW records");
	add(bn);
	curr_x += H;
	// Extract->eQSL
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"E&xtract/e&QSL");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_EXTR_EQSL, 16, 16, 4));
	bn->tooltip("Extract unsent eQSL records");
	add(bn);
	curr_x += H;
	// Extract->LotW
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"E&xtract/&LotW");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_EXTR_LOTW, 16, 16, 4));
	bn->tooltip("Extract unsent LotW records");
	add(bn);
	curr_x += H;
	// Extract->LotW
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"E&xtract/Club&Log");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_EXTR_CLOG, 16, 16, 4));
	bn->tooltip("Extract unsent ClubLog records");
	add(bn);
	curr_x += H;
	// Extract->Upload
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"E&xtract/&Upload");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_UPLOAD, 16, 16, 4));
	bn->tooltip("Upload extracted records");
	add(bn);
	curr_x += H + TOOL_GAP;
	// Connect/disconnect rig
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_rig, nullptr);
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_RIG_ON, 16, 16, 4));
	bn->tooltip("Connect or disconnect rig");
	add(bn);
	curr_x += H;
	// Log->Mode->Import
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_import, nullptr);
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_IMPORT, 16, 16, 4));
	bn->tooltip("Switch to auto-import data mode (disconnects rig and gets data from other apps)");
	add(bn);
	curr_x += H + TOOL_GAP;
	// Extract->Criteria
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"E&xtract/&Criteria");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_EXTR_ON, 16, 16, 4));
	bn->tooltip("Set extract criteria");
	add(bn);
	curr_x += H;
	// Extract->Clear
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"E&xtract/Clea&r");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_EXTR_OFF, 16, 16, 4));
	bn->tooltip("Clear extract criteria");
	add(bn);
	curr_x += H;
	// Extract->Display
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	// Call the menu's callback direct to get the tooltip displayed on this button.
	bn->callback(menu::cb_mi_ext_disp);
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_EXTR_DISP, 16, 16, 4));
	bn->tooltip("Display extract criteria");
	add(bn);
	curr_x += H + TOOL_GAP;
	// Information->Google Maps
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(cb_bn_menu, (void*)"&Information/Google &Maps");
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_MAP, 16, 16, 4));
	bn->tooltip("Look up location in Google Maps");
	add(bn);
	curr_x += H;
	// Information->QRZ.com
	bn = new Fl_Button(curr_x, Y, H, H, 0);
	bn->callback(menu::cb_mi_info_qrz, &search_text_);
	bn->when(FL_WHEN_RELEASE);
	bn->image(new Fl_RGB_Image(ICON_QRZ_COM, 16, 16, 4));
	bn->tooltip("Look up contact in QRZ.com");
	add(bn);
	curr_x += H;
	// Help->Intl
	bn = new Fl_Button(curr_x, Y, H, H, "Æ");
	bn->callback(cb_bn_menu, (void*)"&Help/&Intl");
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Hide/Show International character set");
	add(bn);
	curr_x += H + TOOL_GAP;
	// Input widget for entering search text to look for a call or to parse the call
	search_text_ = "";
	record_num_ = -1;
	intl_input* ip = new intl_input(curr_x, Y, WSMEDIT, H, 0);
	ip->callback(cb_value<intl_input, string>, &search_text_);
	ip->when(FL_WHEN_CHANGED);
	ip->value(search_text_.c_str());
	ip->tooltip("Enter will search for the callsign typed");
	add(ip);
	// Make it visible to load the text up
	ip_search_ = ip;
	curr_x += WSMEDIT;
	// Button to look for the next occurence log for above callsign
	bn = new Fl_Button(curr_x, Y, H, H, "@2->");
	bn->callback(cb_bn_search, (void*)false);
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Jump to next occurence of call in log");
	add(bn);
	curr_x += H;
	// Button to extract all records with this callsign
	bn = new Fl_Button(curr_x, Y, H, H, "@2>[]");
	bn->callback(cb_bn_extract);
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Display all QSOs with thsi callsign in extract log");
	add(bn);
	curr_x += H;
	// Button to parse and explain the callsign in a tooltip
	bn = new Fl_Button(curr_x, Y, H, H, "DX?");
	bn->callback(cb_bn_explain);
	bn->when(FL_WHEN_RELEASE);
	bn->tooltip("Parse the call");
	add(bn);
	curr_x += H + TOOL_GAP;


	end();
	// now copy the active images to the deactivated images, note they will be greyed out
	for (int i = 0; i < children(); i++) {
		Fl_RGB_Image* image = (Fl_RGB_Image*)child(i)->image();
		if (image != nullptr) {
			// The button has an image
			child(i)->deimage(image->copy());
			child(i)->deimage()->inactive();
		}
	}
	// Don't allow the buttons to be resized when the window is resized
	resizable(nullptr);
	show();
	// Do not allow the toolbar to be resized narrower than the width of the buttons
	min_w_ = curr_x;

}

// Destructor
toolbar::~toolbar()
{
	// For each button
	for (int i = 0; i < children(); i++) {
		// if it has an image - delete the image and the inactive image
		if (child(i)->image() != nullptr) {
			delete child(i)->image();
			delete child(i)->deimage();
		}
	}
}

// Button callback - 
// v provides the label of the desired menu item
void toolbar::cb_bn_menu(Fl_Widget*w, void*v) {
	// Get the menu item
	char* item_name = (char*)v;
	const Fl_Menu_Item* item = menu_->find_item(item_name);
	char* message = new char[strlen((char*)v) + 50];
	// If the menu item exists - invoke its callback if it can be picked
	if (item != nullptr) {
		if (item->active()) {
			item->do_callback(menu_);
		}
		else {
			// log that inactive menu item selected
			sprintf(message, "MENU: Tried to select tool bar for inactive item %s", (char*)v);
			status_->misc_status(ST_LOG, message);
		}
	}
	else {
		// If the menu item does not exists - programming error
		sprintf(message, "MENU: Unable to find the menu item - %s", (char*)v);
		status_->misc_status(ST_SEVERE, message);
	}
	delete[] message;
}

// Search the log for the next occurence of the callsign
// v indicates whether to start a new search (true) or resume (false)
void toolbar::cb_bn_search(Fl_Widget* w, void* v) {
	toolbar* that = ancestor_view<toolbar>(w);
	bool reset = (bool)(long)v;
	if (reset) {
		that->record_num_ = 0;
	}
	bool found = false;
	bool keep_on = true;
	string search_call = to_upper(that->search_text_);
	while (keep_on) {
		// For each record from the last search result until found
		for (item_num_t i = that->record_num_; i < navigation_book_->size() && !found; i++) {
			// Compare the callsign against the search input
			record* record = navigation_book_->get_record(i, false);
			if (record->item("CALL") == search_call) {
				// select the record that was found
				found = true;
				navigation_book_->selection(i, HT_SELECTED);
				// Remember the record found
				that->record_num_ = i + 1;
			}
		}
		if (!found) {
			// Callsign not found
			if (that->record_num_ == 0) {
				// Still at the start of the book so no entries at all of this callsign
				char* message = new char[that->search_text_.length() + 50];
				sprintf(message, "LOG: %s not found", that->search_text_.c_str());
				status_->misc_status(ST_WARNING, message);
				delete[] message;
				keep_on = false;
			}
			else {
				// We have had at least one occurence
				char* message = new char[that->search_text_.length() + 50];
				sprintf(message, "LOG: No more instances of %s found", that->search_text_.c_str());
				status_->misc_status(ST_NOTE, message);
				delete[] message;
				if (fl_choice("Reached the end of the log, do you want to start again?", "Yes", "No", nullptr) == 1) {
					// User wants to stop
					keep_on = false;
				}
				else {
					// Go back to start - while loop will continue
					that->record_num_ = 0;
				}
			}
		}
		else {
			// We have found the callsign, stop the search
			keep_on = false;
		}
	}
}

// Callback to extract all the records for the callsign in the search text box
// v is not used
void toolbar::cb_bn_extract(Fl_Widget* w, void* v) {
	toolbar* that = ancestor_view<toolbar>(w);
	cb_value<intl_input, string>((Fl_Widget*)that->ip_search_, (void*)& that->search_text_);
	extract_records_->extract_call(that->search_text_);
}

// Open a tooltip that displays the parse results for the callsign in the search text box
// v is not used
void toolbar::cb_bn_explain(Fl_Widget* w, void* v) {
	toolbar* that = ancestor_view<toolbar>(w);
	// Create a temporary record to parse theh callsign
	record* tip_record = qso_manager_->dummy_qso();
	string message = "";
	// Set the callsign in the temporary record
	tip_record->item("CALL", to_upper(that->search_text_));
	// Parse the temporary record
	message = pfx_data_->get_tip(tip_record);
	// Create a tooltip window at the explain button (in w) X and Y
	Fl_Window* tw = ::tip_window(message, main_window_->x_root() + w->x(), main_window_->y_root() + w->y());
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
	delete tip_record;
}

// Set logging mode
// v is not used
void toolbar::cb_bn_import(Fl_Widget* w, void* v) {
	import_data_->start_auto_update();
}

// Connect or disconnect rig
// v is not used
void toolbar::cb_bn_rig(Fl_Widget* w, void* v) {
	qso_manager_->switch_rig();
}

// Return the minimum width required
int toolbar::min_w() { return min_w_; }

// Set the record number to get the default input for the search input
void toolbar::search_text(int record_num) {
	record_num_ = record_num;
	if (book_->size() > record_num_) {
		search_text_ = book_->get_record(record_num_, false)->item("CALL");
	}
	else {
		search_text_ = "";
	}
	record_num_++;
	((intl_input*)ip_search_)->value(search_text_.c_str());
	redraw();
}

// Set the search text to a specific callsign
void toolbar::search_text(string callsign) {
	search_text_ = callsign;
	((intl_input*)ip_search_)->value(search_text_.c_str());
	redraw();
}

// Update items - activate those that are linked to active menu items
void toolbar::update_items() {
	int num_children = children();
	// For all widgets
	for (int i = 0; i < num_children; i++) {
		Fl_Widget* w = child(i);
		// If it has the menu item callback, then set the widget active as the menu item is
		if (w->callback() == &cb_bn_menu) {
			const Fl_Menu_Item* item = menu_->find_item((char*)w->user_data());
			if (item == nullptr) {
				// Menu item does not exist - deactivate the toolbar button and report
				if (status_) {
					char message[128];
					snprintf(message, 128, "MENU: Broken menu item %s", (char*)w->user_data());
					status_->misc_status(ST_SEVERE, message);
				}
				w->deactivate();
			} else if (item->active() && !(item->radio() && item->value())) {
				// It exists, is active and not a selected radio item, activate the toolbar button
				w->activate();
			}
			else {
				// It exists, is inactive or a selected radio item, deactivate the toolbar button
				w->deactivate();
			}
		}
		else if (w->callback() == cb_bn_rig) {
			if (!qso_manager_) {
				w->deactivate();
			} else {
				w->activate();
			}
		}
	}
}