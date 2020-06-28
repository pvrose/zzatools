#include "ic7300_table.h"
#include "../zzalib/ic7300.h"
#include "../zzalib/drawing.h"
#include "../zzalib/utils.h"
#include "book.h"
#include "status.h"

#include <iostream>

#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>


using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern ic7300* ic7300_;
extern status* status_;

ic7300_table::ic7300_table(int X, int Y, int W, int H, const char* label, field_ordering_t order) : 
Fl_Table_Row(X, Y, W, H, label)
, view()
{
	Fl_Preferences view_settings(settings_, "View");
	view_settings.get("Type", (int&)type_, VT_MEMORIES);
	show();
}

ic7300_table::~ic7300_table() {
	delete_items();
	Fl_Preferences view_settings(settings_, "View");
	view_settings.set("Type", (int)type_);

}

// DElete items 
void ic7300_table::delete_items() {
	//for (int r = 0; r < rows(); r++) {
	//	string* item = *(items_ + r);
	//	for (int c = 0; c < cols(); c++) {
	//		delete (item + c);
	//	}
	//	delete[] (items_ + r);
	//	delete (row_headers_ + r);
	//}
	//for (int c = 0; c < cols(); c++) {
	//	delete (headers_ + c);
	//	delete (col_widths_ + c);
	//}
	items_ = nullptr;
	headers_ = nullptr;
	row_headers_ = nullptr;
	col_widths_ = nullptr;
}

void ic7300_table::draw_cell(TableContext context, int R, int C, int X, int Y,
	int W, int H) {
	if (items_valid_) {
		switch (context) {

		case CONTEXT_STARTPAGE:
			// Set the table font
			fl_font(FONT, FONT_SIZE);
			return;

		case CONTEXT_COL_HEADER:
			if (headers_) {
				// put field header text into header (top-most row)
				fl_push_clip(X, Y, W, H);
				{
					fl_draw_box(FL_BORDER_BOX, X, Y, W, H, col_header_color());
					fl_color(FL_BLACK);
					// text is field header
					fl_draw((headers_ + C)->c_str(), X, Y, W, H, FL_ALIGN_CENTER);
				}
				fl_pop_clip();
			}
			return;

		case CONTEXT_ROW_HEADER:
			if (row_headers_) {
				fl_push_clip(X, Y, W, H);
				{
					fl_draw_box(FL_BORDER_BOX, X, Y, W, H, row_header_color());
					fl_color(FL_BLACK);
					fl_draw((row_headers_ + R)->c_str(), X + 5, Y, W - 5, H, FL_ALIGN_LEFT);
				}
				fl_pop_clip();
			}
			return;

		case CONTEXT_CELL:
			// Get content from record R, field given by fields[C]
			// Note we may be redrawing before we've updated the number of rows
			fl_push_clip(X, Y, W, H);
			{
				// BG COLOR - fill the cell with a colour
				Fl_Color bg_colour = row_selected(R) ? selection_color() : FL_WHITE;
				fl_color(bg_colour);
				fl_rectf(X, Y, W, H);

				// TEXT - contrast its colour to the bg colour.
				if (type_ == VT_MEMORIES && C > 7 && C < 14 && **(items_ + R) != "SPLIT") {
					fl_color(FL_GRAY);
				}
				else {
					fl_color(fl_contrast(FL_BLACK, bg_colour));
				}
				fl_draw((*(items_ + R) + C)->c_str(), X + 1, Y, W - 1, H, FL_ALIGN_CENTER);

				// BORDER - unselected colour
				fl_color(color());
				// draw top and right edges only
				fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
			}
			fl_pop_clip();
			return;

		default:
			return;
		}
	}
}

// OPen specific view
void ic7300_table::open_view(view_type type) {
	// Use egg-timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	// delete the existing set of itesm
	items_valid_ = false;
	delete_items();
	clear();
	type_ = type;
	// delete current table contents
	clear();
	switch (type_) {
	case VT_NONE:
		hide();
		break;
	case VT_MEMORIES:
		draw_memory_view();
		break;
	case VT_SCOPE_BANDS:
		draw_scope_bands_view();
		break;
	case VT_USER_BANDS:
		draw_user_bands_view();
		break;
	case VT_CW_MESSAGES:
		draw_message_view(true);
		break;
	case VT_RTTY_MESSAGES:
		draw_message_view(false);
		break;
	}
	items_valid_ = true;
	fl_cursor(FL_CURSOR_DEFAULT);
	Fl_Preferences view_settings(settings_, "View");
	view_settings.set("Type", (int)type_);
	status_->misc_status(ST_OK, "RIG: Finished");
}

// Open memory view
void ic7300_table::draw_memory_view() {
	// Define the table parameters
	rows(101);
	cols(15);
	items_ = new string * [rows()];
	headers_ = new string[cols()];
	row_headers_ = new string[rows()];
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_GRAY);
	row_header(true);
	row_header_color(FL_GRAY);
	row_height_all(FONT_SIZE + 3);
	row_header_width(20);
	col_widths_ = new int[cols()];
	for (int i = 0; i < cols(); i++) {
		*(col_widths_ + i) = 10;
	}

	bool ok = true;
	status_->progress(rows(), OT_MEMORY, " memories", false);
	status_->misc_status(ST_NOTE, "RIG: Starting memory fetch");
	// Create the memory list
	for (int i = 1; i <= rows(); i++) {
		// Add memory number to sub_command
		string cmd_data = int_to_bcd(i, 2, false);
		// This is a bit of a frig 
		string sub_command = "";
		sub_command.resize(1, '\x00');
		// Fetch the memory contents
		string data;
		if (ok) {
			data = ic7300_->send_command('\x1A', sub_command, cmd_data, ok);
			status_->progress(i, OT_MEMORY);
		}
		else {
			status_->progress("Rig error", OT_MEMORY);
		}
		// Create a new data item
		string* item = new string[cols()];
		items_[i - 1] = item;

		// Memory number - use as row header
		if (i < 100) {
			row_headers_[i - 1] = to_string(i);
		}
		else if (i == 100) {
			row_headers_[i - 1] = "P1";
		}
		else if (i == 101) {
			row_headers_[i - 1] = "P2";
		}

		if (ok) {
			// parse the command, sub-command and address
			data = data.substr(4);
			if (data.length() != 0) {
				if (data.length() < 38) data.resize(38);
				// Split
				switch (data[0] & '\xf0') {
				case 0:
					item[0] = "NO SPLIT";
					break;
				case 'x80':
					item[0] = "SPLIT";
					break;
				default:
					item[0] = "INVALID";
					break;
				}
				// Memory group
				switch (data[0] & '\xF') {
				case 0:
					item[1] = "OFF";
					break;
				case 1:
					item[1] = "*1";
					break;
				case 2:
					item[1] = "*2";
					break;
				case 3:
					item[1] = "*3";
					break;
				default:
					item[1] = "INVALID";
				}
				// RX frequency (or common frequency)
				item[2] = to_string(bcd_to_double(data.substr(1, 5), 6, true));
				// Operating mode
				switch (data[6]) {
				case 0:
					item[3] = "LSB";
					break;
				case 1:
					item[3] = "USB";
					break;
				case 2:
					item[3] = "AM";
					break;
				case 3:
					item[3] = "CW";
					break;
				case 4:
					item[3] = "RTTY";
					break;
				case 5:
					item[3] = "FM";
					break;
				case 7:
					item[3] = "CW-R";
					break;
				case 8:
					item[3] = "RTTY-R";
					break;
				default:
					item[3] = "INVALID";
					break;
				}
				// Filter
				switch (data[7]) {
				case 1:
					item[4] = "FILTER 1";
					break;
				case 2:
					item[4] = "FILTER 2";
					break;
				case 3:
					item[4] = "FILTER 3";
					break;
				default:
					item[4] = "INVALID";
					break;
				}
				// Data mode
				switch (data[8] & '\xF0') {
				case 0:
					break;
				case '\x10':
					item[3] += "-D";
					break;
				default:
					item[3] += " INVALID DATA";
					break;
				}
				// Tone mode
				switch (data[8] & '\x0f') {
				case 0:
					item[5] = "OFF";
					break;
				case 1:
					item[5] = "TONE";
					break;
				case 2:
					item[5] = "TSQL";
					break;
				default:
					item[5] = "INVALID";
					break;
				}
				// REpeater tone setting
				double d;
				d = bcd_to_double(data.substr(9, 3), 1, false);
				char temp[10];
				sprintf(temp, "%.1f", d);
				item[6] = temp;
				// Tone squelch settin
				d = bcd_to_double(data.substr(12, 3), 1, false);
				sprintf(temp, "%.1f", d);
				item[7] = temp;
				// Split frequency (or common frequency)
				item[8] = to_string(bcd_to_double(data.substr(15, 5), 6, true));
				// Operating mode
				switch (data[20]) {
				case 0:
					item[9] = "LSB";
					break;
				case 1:
					item[9] = "USB";
					break;
				case 2:
					item[9] = "AM";
					break;
				case 3:
					item[9] = "CW";
					break;
				case 4:
					item[9] = "RTTY";
					break;
				case 5:
					item[9] = "FM";
					break;
				case 7:
					item[9] = "CW-R";
					break;
				case 8:
					item[9] = "RTTY-R";
					break;
				default:
					item[9] = "INVALID";
					break;
				}
				// Filter
				switch (data[21]) {
				case 1:
					item[10] = "FILTER 1";
					break;
				case 2:
					item[10] = "FILTER 2";
					break;
				case 3:
					item[10] = "FILTER 3";
					break;
				default:
					item[10] = "INVALID";
					break;
				}
				// Data mode
				switch (data[22] & '\xF0') {
				case 0:
					break;
				case '\x80':
					item[9] += "-D";
					break;
				default:
					item[9] += " INVALID DATA";
					break;
				}
				// Tone mode
				switch (data[22] & '\x0f') {
				case 0:
					item[11] = "OFF";
					break;
				case 1:
					item[11] = "TONE";
					break;
				case 2:
					item[11] = "TSQL";
					break;
				default:
					item[11] = "INVALID";
					break;
				}
				// REpeater tone setting
				d = bcd_to_double(data.substr(23, 3), 1, false);
				sprintf(temp, "%.1f", d);
				item[12] = temp;
				// Tone squelch settin
				d = bcd_to_double(data.substr(26, 3), 1, false);
				sprintf(temp, "%.1f", d);
				item[13] = temp;
				// Mmoory name
				item[14] = data.substr(29, 10);
			}
		}
		else {
			for (int j = 0; j < cols(); j++) {
				item[j] = "INVALID";
			}
		}
	}
	// Now define the headers
	headers_[0] = "Split";
	headers_[1] = "Group";
	headers_[2] = "Main freq. MHz";
	headers_[3] = "Mode";
	headers_[4] = "Filter";
	headers_[5] = "Tone";
	headers_[6] = "Repeater Tone Hz";
	headers_[7] = "Tone squelch Hz";
	headers_[8] = "Second freq. MHz";
	headers_[9] = "Mode";
	headers_[10] = "Filter";
	headers_[11] = "Tone";
	headers_[12] = "Repeater Tone Hz";
	headers_[13] = "Tone squelch Hz";
	headers_[14] = "Name";
	resize_cols();
	if (ok) {
		status_->misc_status(ST_OK, "RIG: Complete");
	}
	else {
		status_->misc_status(ST_ERROR, "RIG: Error when reading a register");
	}

}

// Scope band edges view
void ic7300_table::draw_scope_bands_view() {
	// Define table parameters
	rows(39);
	cols(2);
	items_ = new string * [rows()];
	headers_ = new string[cols()];
	row_headers_ = new string[rows()];
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_GRAY);
	row_header(true);
	row_header_color(FL_GRAY);
	row_height_all(FONT_SIZE + 3);
	row_header_width(100);
	col_widths_ = new int[cols()];
	for (int i = 0; i < cols(); i++) {
		*(col_widths_ + i) = 100;
	}

	// Set row headers
	row_headers_[0] = "0.03~1.6MHz";
	row_headers_[1] = "";
	row_headers_[2] = "";
	row_headers_[3] = "1.6~2.0MHz";
	row_headers_[4] = "";
	row_headers_[5] = "";
	row_headers_[6] = "2.0~6.0MHz";
	row_headers_[7] = "";
	row_headers_[8] = "";
	row_headers_[9] = "6.0~8.0MHz";
	row_headers_[10] = "";
	row_headers_[11] = "";
	row_headers_[12] = "8.0~11.0MHz";
	row_headers_[13] = "";
	row_headers_[14] = "";
	row_headers_[15] = "11.0~15.0MHz";
	row_headers_[16] = "";
	row_headers_[17] = "";
	row_headers_[18] = "15.0~20.0MHz";
	row_headers_[19] = "";
	row_headers_[20] = "";
	row_headers_[21] = "20.0~22.0MHz";
	row_headers_[22] = "";
	row_headers_[23] = "";
	row_headers_[24] = "22.0~26.0MHz";
	row_headers_[25] = "";
	row_headers_[26] = "";
	row_headers_[27] = "26.0~30.0MHz";
	row_headers_[28] = "";
	row_headers_[29] = "";
	row_headers_[30] = "30.0~45.0MHz";
	row_headers_[31] = "";
	row_headers_[32] = "";
	row_headers_[33] = "45.0~60.0MHz";
	row_headers_[34] = "";
	row_headers_[35] = "";
	row_headers_[36] = "60.0~74.8MHz";
	row_headers_[37] = "";
	row_headers_[38] = "";
	// Column headers
	headers_[0] = "Lower edge MHz";
	headers_[1] = "Upper edge MHz";
	// For each scope edge register
	bool ok = true;
	status_->progress(rows(), OT_MEMORY, " Scope bands", false);
	status_->misc_status(ST_NOTE, "RIG: Starting scope band fetch");
	for (int r = 0; r < rows(); r++) {
		string* item = new string[cols()];
		if (ok) {
			status_->progress(r + 1, OT_MEMORY);
			// First scope band edge is at 1A/050112 and rest increment from their
			int address = 112 + r;
			string subcommand = (char)'\x05' + int_to_bcd(address, 2, false);
			// Fetch it
			string data = ic7300_->send_command('\x1a', subcommand.c_str(), ok).substr(4);
			if (ok) {
				double d = bcd_to_double(data.substr(0, 3), 4, true);
				char temp[10];
				sprintf(temp, "%.4f", d);
				item[0] = temp;
				// Tone squelch settin
				d = bcd_to_double(data.substr(3, 3), 4, true);
				sprintf(temp, "%.4f", d);
				item[1] = temp;
			}
			else {
				item[0] = "INVALID";
				item[1] = "INVALID";
			}
		}
		else {
			status_->progress("Rig error", OT_MEMORY);
			item[0] = "INVALID";
			item[1] = "INVALID";
		}
		*(items_ + r) = item;
	}
	resize_cols();
}

// Display transmitter fixed bands and associated user bands - add user defined bands to the tx band row and add further rows for subsequent user defined bands
void ic7300_table::draw_user_bands_view() {
	// Define know table parameters (row count only known after we've read all bands
	cols(4);
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_GRAY);
	row_header(true);
	row_header_color(FL_GRAY);
	row_header_width(20);
	headers_ = new string[cols()];
	col_widths_ = new int[cols()];
	for (int i = 0; i < cols(); i++) {
		*(col_widths_ + i) = 100;
	}
	bool ok = true;
	headers_[0] = "TX band - lower MHz";
	headers_[1] = "TX band - upper MHz";
	headers_[2] = "User band - lower MHz";
	headers_[3] = "User band - upper MHz";
	// Get number of TX bands and user defined bands
	string sub_command = " ";
	sub_command[0] = 0;
	int num_txbands = bcd_to_int(ic7300_->send_command('\x1e', sub_command, ok).substr(2), false);
	if (!ok) num_txbands = 1;
	sub_command[1] = 2;
	int num_userbands;
	if (ok) num_userbands = bcd_to_int(ic7300_->send_command('\x1e', sub_command, ok).substr(2), false);
	else num_userbands = 1;
	if (ok) {
		string* tx_bands = new string[num_txbands];
		string* user_bands = new string[num_userbands];
		status_->progress(num_txbands + num_userbands, OT_MEMORY, "user bands", false);
		status_->misc_status(ST_NOTE, "RIG: Starting user band fetch");
		// Read all the hardware defined bands
		for (int i = 0; i < num_txbands && ok; i++) {
			sub_command[0] = 1;
			status_->progress(i + 1, OT_MEMORY);
			*(tx_bands + i) = ic7300_->send_command('\x1e', sub_command, int_to_bcd(i + 1, 1, false), ok).substr(3);
		}
		// Read all the user-defined bands
		for (int i = 0; i < num_userbands && ok; i++) {
			sub_command[0] = 3;
			status_->progress(i + 1 + num_txbands, OT_MEMORY);
			*(user_bands + i) = ic7300_->send_command('\x1e', sub_command, int_to_bcd(i + 1, 1, false), ok).substr(3);
		}
		// This will create more items than we use
		items_ = new string * [num_txbands + num_userbands];

		int r = 0;
		int user = 0;
		// For each tx band
		for (int i = 0; i < num_txbands && ok; i++) {
			// Create a new item
			string* item = new string[cols()];
			*(items_ + r) = item;
			// Add tx band to left two columns
			string raw_tx_band = *(tx_bands + i);
			double tx_lower = bcd_to_double(raw_tx_band.substr(0, 5), 6, true);
			double tx_upper = bcd_to_double(raw_tx_band.substr(6, 5), 6, true);
			cout << "TX: lower = " << tx_lower << "; upper = " << tx_upper << endl;
			item[0] = to_string(tx_lower);
			item[1] = to_string(tx_upper);
			item[2] = "";
			item[3] = "";
			bool user_valid = true;
			int num_users_in_tx = 0;
			while (user_valid && user < num_userbands) {
				string raw_user_band = *(user_bands + user);
				if (raw_user_band[0] == '\xff' || raw_user_band.length() != 11) {
					// User band is not valid - step to next user, but not next TX or row
					cout << "USER: Not valid!" << endl;
					user++;
				}
				else {
					// User band is valid - is it in the tx band currently being looked at?
					double user_lower = bcd_to_double(raw_user_band.substr(0, 5), 6, true);
					double user_upper = bcd_to_double(raw_user_band.substr(6, 5), 6, true);
					cout << "USER: lower = " << user_lower << "; upper = " << user_upper;
					if (user_lower >= tx_lower && user_upper <= tx_upper) {
						cout << " In current TX! written row " << r << endl;
						// User band is for the current tx band - add to columns 2 and 3 - step user and row
						// Need to create a new row
						num_users_in_tx++;
						if (num_users_in_tx > 1) {
							item = new string[cols()];
							*(items_ + r) = item;
							item[0] = "";
							item[1] = "";
							item[2] = to_string(user_lower);
							item[3] = to_string(user_upper);
						}
						else {
							item[2] = to_string(user_lower);
							item[3] = to_string(user_upper);
						}
						user++;
						r++;
					}
					else if (user_lower > tx_lower && num_users_in_tx == 0) {
						// No user bands in this TX band - write TX band
						cout << " Writing TX to row " << r << endl;
						r++;
						user_valid = false;
					} else
					{
						// User band is not for the tx band - step tx band and not user or row
						cout << " Not in current TX!" << endl;
						user_valid = false;
					}
				}
			}
			// Increment row count
		}
		// Now we know the real number of rows can configure rows.
		rows(r);
		delete[] tx_bands;
		delete[] user_bands;
	}
	else {
		items_ = new string * [1];
		string* item = new string[cols()];
		*items_ = item;
		for (int i = 0; i < cols(); i++) {
			item[i] = "INVALID";
		}
		rows(1);
	}
	row_headers_ = new string[rows()];
	for (int r = 0; r < rows(); r++) {
		row_headers_[r] = to_string(r);
	}
	row_height_all(FONT_SIZE + 3);
	resize_cols();
}

void ic7300_table::draw_message_view(bool cw) {
	// Define table parameters
	rows(8);
	cols(2);
	items_ = new string * [rows()];
	headers_ = new string[cols()];
	row_headers_ = new string[rows()];
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_GRAY);
	row_header(true);
	row_header_color(FL_GRAY);
	row_height_all(FONT_SIZE + 3);
	row_header_width(30);
	col_widths_ = new int[cols()];
	for (int i = 0; i < cols(); i++) {
		*(col_widths_ + i) = 100;
	}
	headers_[0] = "Count-up Trigger";
	headers_[1] = "Data";

	status_->progress(rows(), OT_MEMORY, "messages", false);
	status_->misc_status(ST_NOTE, "RIG: Starting message fetch");

	bool ok = true;

	if (cw) {
		// set transceiver into CW mode
		(void)ic7300_->send_command('\x06', "", "\x03\x01", ok);
	}
	else {
		// set transceiver into RTTY mode - TODO: This doesn't deliver RTTY messages.
		(void)ic7300_->send_command('\x06', "", "\x04\x01", ok);
	}

	// Get the count up trigger memory number
	int trigger;
	if (ok) trigger = bcd_to_int(ic7300_->send_command('\x1a', "\x05\x01\x56", ok).substr(4), false);
	// Now read the keyer memory
	for (int r = 0; r < rows(); r++) {
		status_->progress(r + 1, OT_MEMORY);
		string* item = new string[cols()];
		items_[r] = item;
		// Add memory name as row header
		char name[4];
		if (cw) {
			sprintf(name, "M%1d", r + 1);
		}
		else {
			sprintf(name, "RT%1d", r + 1);
		}
		row_headers_[r] = name;
		// Is it trigger
		if (ok && r + 1 == trigger) {
			item[0] = "Yes";
		}
		else {
			item[0] = "";
		}
		// Read memory contents - ASCII
		string sub_command = (char)'\x02' + int_to_bcd(r + 1, 1, false);
		if (ok) item[1] = ic7300_->send_command('\x1a', sub_command, ok).substr(3);
		else item[1] = "INVALID";
	}
	resize_cols();
}

void ic7300_table::type(view_type type) {
	if (type == VT_UNCHANGED) {
		open_view(type_);
	}
	else {
		open_view(type);
	}
}

view_type ic7300_table::type() {
	return type_;
}

// Adjust all column widths to the largest data.
void ic7300_table::resize_cols() {
	fl_font(FONT, FONT_SIZE);
	int w;
	int h;
	for (int c = 0; c < cols(); c++) {
		// Column header
		w = 0;
		h = 0;
		fl_measure((headers_[c]).c_str(), w, h, 0);
		w += 10;
		if (w > col_widths_[c]) {
			col_widths_[c] = w;
		}
		// Now the contents of each cell in the column
		for (int r = 0; r < rows(); r++) {
			int w = 0;
			int h = 0;
			fl_measure(((items_[r])[c]).c_str(), w, h, 0);
			w += 10;
			if (w > col_widths_[c]) {
				col_widths_[c] = w;
			}
		}
		col_width(c, col_widths_[c]);
	}
	// And repeat for row headers
	for (int r = 0; r < rows(); r++) {
		int w = 0;
		int h = 0;
		fl_measure((row_headers_[r]).c_str(), w, h, 0);
		w += 10;
		if (w > row_header_width()) {
			row_header_width(w);
		}
	}
}

// Use table row version of handle for the time being until we know design memory editing
int ic7300_table::handle(int event) {
	return Fl_Table_Row::handle(event);
}

// Null function
void ic7300_table::update(hint_t hint, unsigned int record_num_1, unsigned int record_num_2) {
	return;
}
