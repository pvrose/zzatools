#include "view73.h"
#include "../zzalib/rig_if.h"

#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>

using namespace zza7300;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern rig_if* rig_if_;

view73::view73(int X, int Y, int W, int H, const char* label) : Fl_Table_Row(X, Y, W, H, label) {
	Fl_Preferences view_settings(settings_, "View");
	view_settings.get("Type", (int&)type_, VT_MEMORIES);
	show();
}

view73::~view73() {
	delete_items();
}

// DElete items 
void view73::delete_items() {
	//for (int r = 0; r < rows(); r++) {
	//	string* item = *(items_ + r);
	//	for (int c = 0; c < cols(); c++) {
	//		delete (item + c);
	//	}
	//	delete (items_ + r);
	//	delete (row_headers_ + r);
	//}
	//for (int c = 0; c < cols(); c++) {
	//	delete (headers_ + c);
	//}
	items_ = nullptr;
	headers_ = nullptr;
	row_headers_ = nullptr;
}

void view73::draw_cell(TableContext context, int R, int C, int X, int Y,
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
					fl_color(FL_BLACK);
					fl_draw((row_headers_ + R)->c_str(), X, Y, W, H, FL_ALIGN_LEFT);
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
				fl_color(fl_contrast(FL_BLACK, bg_colour));
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
void view73::open_view(view_type type) {
	// delete the existing set of itesm
	items_valid_ = false;
	delete_items();
	type_ = type;
	// delete current table contents
	clear();
	switch (type_) {
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
}

// Open memory view
void view73::draw_memory_view() {
	// Define the table parameters
	rows(101);
	cols(15);
	items_ = new string*[rows()];
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

	bool ok = true;
	// Create the memory list
	for (int i = 1; i <= rows() && ok; i++) {
		// Add memory number to sub_command
		string cmd_data = int_to_bcd(i, 2, false);
		// This is a bit of a frig 
		string sub_command = "";
		sub_command.resize(1, '\x00');
		// Fetch the memory contents
		string data = send_command('\x1A', sub_command, cmd_data, ok);
		// Strip the command, sub-command and address
		data = data.substr(4);
		if (data.length() != 0) {
			if (data.length() < 38) data.resize(38);
			// Parse the string
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
			item[2] = bcd_to_string(data.substr(1, 5), 6, false);
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
			item[6] = bcd_to_string(data.substr(9, 3), 1, true);
			// Tone squelch settin
			item[7] = bcd_to_string(data.substr(12, 3), 1, true);
			// Split frequency (or common frequency)
			item[8] = bcd_to_string(data.substr(15, 5), 6, false);
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
			item[12] = bcd_to_string(data.substr(23, 3), 1, true);
			// Tone squelch settin
			item[13] = bcd_to_string(data.substr(26, 3), 1, true);
			// Mmoory name
			item[14] = data.substr(29, 10);
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

}

// Scope band edges view
void view73::draw_scope_bands_view() {
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

	// Set row headers
	row_headers_[0] = "0.03~1.6MHz #1";
	row_headers_[1] = "0.03~1.6MHz #2";
	row_headers_[2] = "0.03~1.6MHz #3";
	row_headers_[3] = "1.6~2.0MHz #1";
	row_headers_[4] = "1.6~2.0MHz #2";
	row_headers_[5] = "1.6~2.0MHz #3";
	row_headers_[6] = "2.0~6.0MHz #1";
	row_headers_[7] = "2.0~6.0MHz #2";
	row_headers_[8] = "2.0~6.0MHz #3";
	row_headers_[9] = "6.0~8.0MHz #1";
	row_headers_[10] = "6.0~8.0MHz #2";
	row_headers_[11] = "6.0~8.0MHz #3";
	row_headers_[12] = "8.0~11.0MHz #1";
	row_headers_[13] = "8.0~11.0MHz #2";
	row_headers_[14] = "8.0~11.0MHz #3";
	row_headers_[15] = "11.0~15.0MHz #1";
	row_headers_[16] = "11.0~15.0MHz #2";
	row_headers_[17] = "11.0~15.0MHz #3";
	row_headers_[18] = "15.0~20.0MHz #1";
	row_headers_[19] = "15.0~20.0MHz #2";
	row_headers_[20] = "15.0~20.0MHz #3";
	row_headers_[21] = "20.0~22.0MHz #1";
	row_headers_[22] = "20.0~22.0MHz #2";
	row_headers_[23] = "20.0~22.0MHz #3";
	row_headers_[24] = "22.0~26.0MHz #1";
	row_headers_[25] = "22.0~26.0MHz #2";
	row_headers_[26] = "22.0~26.0MHz #3";
	row_headers_[27] = "26.0~30.0Hz #1";
	row_headers_[28] = "26.0~30.0MHz #2";
	row_headers_[29] = "26.0~30.0MHz #3";
	row_headers_[30] = "30.0~45.0MHz #1";
	row_headers_[31] = "30.0~45.0MHz #2";
	row_headers_[32] = "30.0~45.0MHz #3";
	row_headers_[33] = "45.0~60.0MHz #1";
	row_headers_[34] = "45.0~60.0MHz #2";
	row_headers_[35] = "45.0~60.0MHz #3";
	row_headers_[36] = "60.0~74.8MHz #1";
	row_headers_[37] = "60.0~74.8MHz #2";
	row_headers_[38] = "60.0~74.8MHz #3";
	// Column headers
	headers_[0] = "Lower edge MHz";
	headers_[1] = "Upper edge MHz";
	// For each scope edge register
	bool ok = true;
	for (int r = 0; r < rows() && ok; r++) {
		// First scope band edge is at 1A/050112 and rest increment from their
		int address = 112 + r;
		string subcommand = (char)'\x05' + int_to_bcd(address, 2, false);
		// Fetch it
		string data = send_command('\x1a', subcommand.c_str(), ok).substr(4);
		string* item = new string[cols()];
		item[0] = bcd_to_string(data.substr(0, 3), 4, false);
		item[1] = bcd_to_string(data.substr(3, 3), 4, false);
		*(items_ + r) = item;
	}
}

// Display transmitter fixed bands and associated user bands - add user defined bands to the tx band row and add further rows for subsequent user defined bands
void view73::draw_user_bands_view() {
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
	bool ok = true;
	// Get number of TX bands and user defined bands
	string sub_command = " ";
	sub_command[0] = 0;
	int num_txbands = bcd_to_int(send_command('\x1e', sub_command, ok).substr(2));
	sub_command[1] = 2;
	int num_userbands = bcd_to_int(send_command('\x1e', sub_command, ok).substr(2));
	string* tx_bands = new string[num_txbands];
	string* user_bands = new string[num_userbands];
	headers_[0] = "TX band - lower MHz";
	headers_[1] = "TX band - upper MHz";
	headers_[2] = "User band - lower MHz";
	headers_[3] = "User band - upper MHz";
	// Read all the hardware defined bands
	for (int i = 0; i < num_txbands && ok; i++) {
		sub_command[0] = 1;
		*(tx_bands + i) = send_command('\x1e', sub_command, int_to_bcd(i + 1, 1, false), ok).substr(3);
	}
	// Read all the user-defined bands
	for (int i = 0; i < num_userbands && ok; i++) {
		sub_command[0] = 3;
		*(user_bands + i) = send_command('\x1e', sub_command, int_to_bcd(i + 1, 1, false), ok).substr(3);
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
		string tx_lower = bcd_to_string(tx_bands[i].substr(0, 5), 6, false);
		string tx_upper = bcd_to_string(tx_bands[i].substr(6, 5), 6, false);
		item[0] = tx_lower;
		item[1] = tx_upper;
		bool user_valid = true;
		int num_users_in_tx = 0;
		while (user_valid && user < num_userbands) {
			if (user_bands[0] == "\xff") {
				// User band is not valid - step to next user, but not next TX
				item[2] = "";
				item[3] = "";
				user++;
			}
			else {
				// User band is valid - is it in the tx band currently being looked at?
				string user_lower = bcd_to_string(user_bands[user].substr(0, 5), 6, false);
				string user_upper = bcd_to_string(user_bands[user].substr(6, 5), 6, false);
				if (user_lower >= tx_lower && user_upper <= tx_upper) {
					// User band is for the current tx band - add to columns 2 and 3 - step user and row
					num_users_in_tx++;
					if (num_users_in_tx > 1) {
						item = new string[cols()];
						*(items_ + r) = item;
						item[0] = "";
						item[1] = "";
						item[2] = user_lower;
						item[3] = user_upper;
						r++;
					}
					user++;
				}
				else {
					// User band is not for the tx band - step tx band
					user_valid = false;
				}
			}
		} 
	}
	// Now we know the real number of rows can configure rows.
	rows(r);
	row_height_all(FONT_SIZE + 1);
}

void view73::draw_message_view(bool cw) {
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
	row_header_width(20);
	headers_[0] = "Count-up Trigger";
	headers_[1] = "Data";

	bool ok = true;

	if (cw) {
		// set transceiver into CW mode
		(void)send_command('\x06', "", "\x03\x01", ok);
	}
	else {
		// set transceiver into RTTY mode
		(void)send_command('\x06', "", "\x04\x01", ok);
	}
	// Get the count up trigger memory number
	int trigger = bcd_to_int(send_command('\x1a', "\x05\x01\x56", ok));
	// Now read the keyer memory
	for (int r = 0; r < rows() && ok; r++) {
		string* item = new string[cols()];
		items_[r] = item;
		// Add memory name as row header
		char name[3];
		sprintf(name, "M%1d", r + 1);
		row_headers_[r] = name;
		// Is it trigger
		if (r + 1 == trigger) {
			item[0] = "Yes";
		}
		else {
			item[0] = "";
		}
		// Read memory contents - ASCII
		string sub_command = (char)'\x02' + int_to_bcd(r + 1, 1, false);
		item[1] = send_command('\x1a', sub_command, ok);
	}
}

// Convert BCD to string
string view73::bcd_to_string(string bcd, int decimals, bool least_first) {
	string result;
	int l_result = bcd.length() * 2;
	result.resize(l_result + (decimals ? 1 : 0), ' ');
	int decimal_point = l_result - decimals;
	int curr_digit = 0;

	if (least_first) {
		for (size_t i = 0; i < bcd.length(); i++) {
			unsigned char c = bcd[i];
			if (decimals && curr_digit == decimal_point) {
				result[curr_digit] = '.';
				curr_digit++;
			}
			result[curr_digit] = '0' + (c >> 4);
			curr_digit++;
			if (decimals && curr_digit == decimal_point) {
				result[curr_digit] = '.';
				curr_digit++;
			}
			result[curr_digit] = '0' + (c & '\x0f');
			curr_digit++;
		}
	}
	else {
		for (size_t i = bcd.length(); i > 0;) {
			// Decrement before it's used
			i--;
			unsigned char c = bcd[i];
			if (decimals && curr_digit == decimal_point) {
				result[curr_digit] = '.';
				curr_digit++;
			}
			result[curr_digit] = '0' + (c >> 4);
			curr_digit++;
			if (decimals && curr_digit == decimal_point) {
				result[curr_digit] = '.';
				curr_digit++;
			}
			result[curr_digit] = '0' + (c & '\x0f');
			curr_digit++;
		}
	}
	return result;
}

// Convert int to BCD
string view73::int_to_bcd(int value, int size, bool least_first) {
	string result;
	result.resize(size, ' ');

	int value_left = value;
	// Convert each part of the integer into two BCD characters
	if (least_first) {
		for (int i = 0; i < size; i++) {
			unsigned char c;
			int remainder = value_left % 100;
			c = ((remainder / 10) << 4) + (remainder % 10);
			value_left /= 100;
			result[i] = c;
		}
	}
	else {
		for (int i = size; i > 0;) {
			unsigned char c;
			int remainder = value_left % 100;
			c = ((remainder / 10) << 4) + (remainder % 10);
			value_left /= 100;
			result[--i] = c;
		}
	}
	return result;
}

// Convert BCD to int
int view73::bcd_to_int(string bcd) {
	int result = 0;
	for (int i = 0; i < bcd.length(); i++) {
		unsigned char c = bcd[i];
		int ls = c & '\x0f';
		int ms = c & '\xf0';
		ms >>= 4;
		result = result * 100 + ls + 10 * ms;
	}
	return result;
}

// Send command
string view73::send_command(unsigned char command, string sub_command, bool& ok) {
	return send_command(command, sub_command, "", ok);
}

string view73::send_command(unsigned char command, string sub_command, string data, bool& ok) {
	ok = true;
	if (rig_if_) {
		string cmd = "\xFE\xFE\x94\xE0";
		cmd += command;
		cmd += sub_command;
		cmd += data;
		cmd += '\xfd';
		string to_send = string_to_hex(cmd, true);

		string raw_data = rig_if_->raw_message(to_send);

		// Ignore reflected data
		if (raw_data.substr(0,2) != "FE") {
			fl_alert("Response not from IC-7300: %s", raw_data.c_str());
			ok = false;
			return raw_data;
		}
		string response = hex_to_string(raw_data);
		if (response.length() == 0) {
			// TRansceiver has not responded at all
			fl_alert("Receieved no response from transceiver");
			ok = false;
			return "";
		}
		else {
			// For data - strip off command and subcommand
			int discard = cmd.length();
			if (response.substr(0, discard) == cmd.substr(0, discard)) {
				response = response.substr(discard);
				if (response.length() == 6 && response[4] > '\xf0') {
					if (response[4] == '\xfa') {
						// Got NAK response from transceiver
						fl_alert("Received a bad response from transceiver");
						ok = false;
						return "";
					}
					else {
						// Got ACK response from transceiver, but no data
						return ("");
					}
				}
				else {
					// Discard the CI-V preamble/postamble
					return response.substr(4, response.length()-5);
				}
			}
			else {
				fl_alert("Unexpected response: %s\n%s", response.c_str(), string_to_hex(response).c_str());
				return response;
			}
		}
	}
	else {
		return "NO RIG";
	}
}

string view73::string_to_hex(string data, bool escape /*=true*/) {
	string result;
	char hex_chars[] = "0123456789ABCDEF";
	result.resize(data.length() * 4, ' ');
	int ix = 0;
	for (size_t i = 0; i < data.length(); i++) {
		unsigned char c = data[i];
		if (escape) result[ix++] = 'x';
		result[ix++] = hex_chars[(c >> 4)];
		result[ix++] = hex_chars[(c & '\x0f')];
		result[ix++] = ' ';
	}
	return result;
}

string view73::hex_to_string(string data) {
	string result;
	int ix = 0;
	// For the length of the source striing
	while (ix < data.length()) {
		if (data[ix] == 'x') {
			ix++;
		}
		result += to_ascii(data, ix);
	}
	return result;
}

void view73::type(view_type type) {
	open_view(type);
}

view_type view73::type() {
	return type_;
}