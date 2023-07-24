/*
ZZALOG - Amateur Radio Log
© - Copyright 2017, Philip Rose, GM3ZZA
All Rights Reserved

utils.h - Utility methods
*/
#include "QBS_utils.h"

#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Window.H>

#include <string>
#include <exception>
#include <stdexcept>
#include <vector>
#include <map>
#include <ctime>
#include <cmath>

using namespace std;

// Split the line into its separate words with specified separator
void split_line(const string& line, vector<string>& words, const char separator) {
	// Quotes will escape separator
	bool in_quotes = false;
	words.clear();
	// Name is a_word using indefinite arcticle as word is reserved
	string a_word = "";
	// For the length of the line
	for (size_t i1 = 0; i1 < line.length(); i1++) {
		// Look at every character in the line
		if (line[i1] == separator) {
			// Is it a separator?
			if (in_quotes) {
				// if in quotes copy separator to field
				a_word += line[i1];
			}
			else {
				// if not Save field and start afresh
				words.push_back(a_word);
				a_word = "";
			}
		}
		else if (line[i1] == '"') {
			// if it's '"' then toggle quoted flag
			in_quotes = !in_quotes;
		}
		else {
			// Else copy character to field
			a_word += line[i1];
		}
	}
	// at EOL save the last field
	words.push_back(a_word);
	// Tidy up
	a_word.shrink_to_fit();
}

// Converts display format text to a tm object for reformatting
bool string_to_tm(string source, tm& time, string format) {
	bool escaped = false;
	// Default YMD to 19700101 to avoid assertion when only setting time
	time.tm_year = 70;
	time.tm_mon = 0;
	time.tm_mday = 1;
	time.tm_hour = 0;
	time.tm_min = 0;
	time.tm_sec = 0;
	time.tm_isdst = 0;
	bool bad_value = false;
	int parse_point = 0;
	// "" regarded as an unspecified date. This will be interpreted by use.
	if (source != "") {
		// while there is format string and date string to process
		for (size_t ix = 0; ix < format.length() && ix < source.length(); ix++) {
			// Get format character
			char format_char = format[ix];
			// If previous character was %
			if (escaped) {
				// Look at character
				try {
					switch (format_char) {
					case 'Y':
						// %Y: 4 digit year
						time.tm_year = stoi(source.substr(parse_point, 4)) - 1900;
						parse_point += 4;
						break;
					case 'y':
						// %y: 2 digit year (1970-2069)
						time.tm_year = stoi(source.substr(parse_point, 2));
						if (time.tm_year >= 70) {
							time.tm_year;
						}
						else {
							time.tm_year += 100;
						}
						parse_point += 2;
						break;
					case 'm':
						// %m: 2 digit month
						time.tm_mon = stoi(source.substr(parse_point, 2)) - 1;
						parse_point += 2;
						break;
					case 'b': {
						// %b: 3 letter month
						// Get the locale's month names by generating twelve dates and seeing what %b returns
						map<string, int> month_map;
						tm date;
						date.tm_year = 70;
						date.tm_mday = 1;
						date.tm_hour = 0;
						date.tm_min = 0;
						date.tm_sec = 0;
						date.tm_isdst = false;
						for (int i = 0; i < 12; i++) {
							date.tm_mon = i;
							char name[4];
							strftime(name, 4, "%b", &date);
							month_map[string(name)] = i;
						}
						// Now look the month up - use .at() to deliverately force an exception
						time.tm_mon = month_map.at(source.substr(parse_point, 3));
						parse_point += 3;
						break;
					}
					case 'd':
						// %d: 2 digit day
						time.tm_mday = stoi(source.substr(parse_point, 2));
						parse_point += 2;
						break;
					case 'H':
						// %H: 2 digit hour (24-hour)
						time.tm_hour = stoi(source.substr(parse_point, 2));
						parse_point += 2;
						break;
					case 'M':
						// %M: 2 digit minute
						time.tm_min = stoi(source.substr(parse_point, 2));
						parse_point += 2;
						break;
					case 'S':
						// %S: 2 digit second
						time.tm_sec = stoi(source.substr(parse_point, 2));
						parse_point += 2;
						break;
					default:
						// %<other> - treat as bad data
						parse_point++;
						bad_value = true;
						break;
					}
				}
				catch (invalid_argument&) {
					// Invalid integer value detected
					bad_value = true;
				}
				escaped = false;
			}
			else {
				switch (format_char) {
				case '%':
					escaped = true;
					break;
				default:
					// Set bad data if the date string does not match the format string
					if (source[parse_point++] != format_char) bad_value = true;
					escaped = false;
					break;
				}
			}
		}
	}
	if (bad_value) return false;
	return true;
}

// Convert a string e.g. 00-06:08 to an array of UINTs {0,1,2,3,4,5,6,8}
void string_to_ints(string& text, vector<unsigned int>& ints) {
	int current = 0;
	int start = 0;
	size_t index = 0;
	string temp = text;
	ints.clear();
	bool invalid = false;
	while (temp.length() > 0 && !invalid) {
		// Inspect the whole string - look for first likely separator
		index = temp.find_first_of(";,-");
		try {
			if (index >= 0) {
				// One found - remeber the number
				current = stoi(temp.substr(0, index), 0, 10);
			}
			else {
				// One not found - remember the number - it's the only one remaining
				current = stoi(temp, 0, 10);
			}
		}
		catch (invalid_argument&) {
			invalid = true;
		}
		if (!invalid) {
			if (index == -1 || temp[index] == ';' || temp[index] == ',') {
				// It's either a single number (separated by ; or ') or it's the second of nn-nn pair
				// treat "nn" as "nn-nn"
				if (start == 0) {
					start = current;
				}
				// Expand nn-mm as a list of integeres from nn  to mm inclusive
				for (int i = start; i <= current; i++) {
					ints.push_back(i);
				}
				// skip the ; or .
				if (index != -1 && (temp[index] == ';' || temp[index] == ',')) {
					index++;
					start = 0;
				}
				// Remove characters just processed
				if (index == -1) {
					temp = "";
				}
				else {
					temp = temp.substr(index);
				}
			}
			else if (temp[index] == '-') {
				// else seeing '-' so would be the nn of nn-mm. Remember it for when we process the mm.
				start = current;
				index++;
				temp = temp.substr(index);
			}
			// invalid construct
			else {
				ints.clear();
			}
		}
		else {
			// Invalid construct
			ints.clear();
		}
	}
}

// returns the current time in supplied format
string now(bool local, const char* format) {
	// Get current time
	time_t now = time(nullptr);
	// convert to struct in selected timezone
	tm* figures = local ? localtime(&now) : gmtime(&now);
	char result[100];
	// convert to C string, then C++ string
	strftime(result, 99, format, figures);
	return string(result);
}

// copy tm to time_t and back to update derived values in tm struct
void refresh_tm(tm* date) {
	// I think mktime regards tm as a local time
	time_t first = mktime(date);
	*date = *(localtime(&first));
}

// return  the number of days in the month
int days_in_month(tm* date) {
	switch (date->tm_mon) {
	case 0:
	case 2:
	case 4:
	case 6:
	case 7:
	case 9:
	case 11:
		// J M M J A O D
		return 31;
		break;
	case 3:
	case 5:
	case 8:
	case 10:
		// A J S N
		return 30;
		break;
	case 1:
		// F
		if (is_leap(date)) {
			return 29;
		}
		else {
			return 28;
		}
		break;
	default:
		// We shouldn't get here
		return -1;
	}
}

// return true if the year is a leap year
bool is_leap(tm* date) {
	int year = date->tm_year + 1900;
	if (year % 4 != 0) {
		// Doesn't divide by 4 - NO
		return false;
	}
	else if (year % 400 == 0) {
		// Divides by 400 - YES
		return true;
	}
	else if (year % 100 == 0) {
		// Divides by 100 but not 400
		return false;
	}
	else {
		// All the remnaining years that divide by 4
		return true;
	}
}

// Create a tip window - tip text, position(root_x, root_y)
Fl_Window* tip_window(const string& tip, int x_root, int y_root) {
	// get the size of the text, set the font, default width
	fl_font(Fl_Tooltip::font(), Fl_Tooltip::size());
	int width = Fl_Tooltip::wrap_width();
	int height = 0;
	// Now get the actual width and height
	fl_measure(tip.c_str(), width, height, 0);
	// adjust sizes to allow for margins - use Fl_Tooltip margins.
	width += (2 * Fl_Tooltip::margin_width());
	height += (2 * Fl_Tooltip::margin_height());
	// Create the window
	Fl_Window* win = new Fl_Window(x_root, y_root, width, height, 0);
	win->clear_border();
	
	// Create the output widget.
	Fl_Multiline_Output* op = new Fl_Multiline_Output(0, 0, width, height, 0);
	// Copy the attributes of tool-tips
	op->color(Fl_Tooltip::color());
	op->textcolor(Fl_Tooltip::textcolor());
	op->textfont(Fl_Tooltip::font());
	op->textsize(Fl_Tooltip::size());
	op->wrap(true);
	op->box(FL_BORDER_BOX);
	op->value(tip.c_str());
	win->add(op);
	// set the window parameters: always on top, tooltip
	win->set_non_modal();
	win->set_tooltip_window();
	// Must be after set_tooltip_window.
	win->show();

	return win;
}

// Create an upper-case version of a string
string to_upper(const string& data) {
	size_t len = data.length();
	char* result = new char[3 * len + 1];
	memset(result, 0, 3 * len + 1);
	fl_utf_toupper((unsigned char*)data.c_str(), (int)len, result);
	string ret_value(result);
	delete[] result;
	return ret_value;
}

// Create an lower-case version of a string
string to_lower(const string& data) {
	size_t len = data.length();
	char* result = new char[3 * len + 1];
	memset(result, 0, 3 * len + 1);
	fl_utf_tolower((unsigned char*)data.c_str(), (int)len, result);
	string ret_value(result);
	delete[] result;
	return ret_value;
}

// Check a while string is an integer
bool is_integer(const string& data) {
	bool result = true;
	const char* src = data.c_str();
	while (*src != 0 && result) {
		result = isdigit(*src++);
	}
	return result;
}

// Search for any characters in match (assume its zero-terminated)
size_t find(const char* data, size_t length, const char* match) {
	size_t pos = length;
	bool found = false;
	// Compare each char in data with each char in find
	// pos should get set to the position that matches (or length) if none do
	for (size_t i = 0; i < length && !found; i++) {
		for (size_t j = 0; *(match + j) != '\0' && !found; j++) {
			if (*(data + i) == *(match + j)) {
				found = true;
				pos = i;
			}
		} 
	}
	return pos;
}

// Search for single character match
size_t find(const char* data, size_t length, const char match) {
	size_t pos = length;
	bool found = false;
	// Compare each char in data with match
	// pos should get set to the position that matches (or length if none do
	for (size_t i = 0; i < length && !found; i++) {
		if (*(data + i) == match) {
			found = true;
			pos = i;
		}
	}
	return pos;

}

// Find the occurence of match in data 
size_t find_substr(const char* data, size_t length, const char* match, size_t len_substr) {
	// Possible start of match
	size_t possible = 0;
	// Length of string remaining to match
	size_t rem_length = length;
	bool found = false;
	while (!found && possible + len_substr < rem_length) {
		// Find where first character of match occurs
		possible += find(data + possible, length - possible, *match);
		// Compare with all of match 
		found = (strncmp(data + possible, match, len_substr) == 0);
		// If it doesn't match step possible to find next possible
		if (!found) {
			possible++;
		}
	}
	return possible;
}

// Search for any characters not in match (assume its zero-terminated)
size_t find_not(const char* data, size_t length, const char* match) {
	size_t pos = 0;
	bool matches = true;
	// Compare each char in data with each char in find
	// pos should get set to the position that mismatches (or length) if none do
	for (pos = 0; pos < length && matches; pos++) {
		bool found = false;
		for (size_t j = 0; *(match + j) != '\0' && !found; j++) {
			if (*(data + pos) == *(match + j)) {
				found = true;
			}
		}
		if (found) matches = false;
	}
	return pos;
}

// Replace charatceters with %nnx if not in chars (allow) or in chars (~allow)
string escape_hex(string text, bool allow, const char* chars)
{
	string result = "";
	// For the length of the string
	for (size_t i = 0; i < text.length(); i++) {
		char c = text[i];
		// Is the character in the search string
		const char* pos = strchr(chars, c);
		if ((pos != nullptr && allow) || (pos == nullptr && !allow)) {
			// Use it directly
			result += c;
		}
		else {
			// Relace with %nnx
			char escape[5];
			snprintf(escape, 5, "%%%02x", c);
			result += escape;
		}
	}
	return result;
}

// Escape the non-usable characters in a url - not alphanumeric replace with %nnx 
string escape_url(string url) {
	return escape_hex(url, true, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

// Escape / and & for menu items
string escape_menu(string text) {
	size_t len = text.length() * 2 + 1;
	string dest = "";
	dest.reserve(len);
	const char* src = text.c_str();
	while (*src) {
		switch (*src) {
		case '/':
			dest += '\\';
			break;
		case '&':
			dest += '&';
			break;
		}
		dest += *src++;
	}
	return dest;
}

// Convert from %nn to ASCII character
string unescape_hex(string text) {
	string result = "";
	result.reserve(text.length());
	const char* src = text.c_str();
	// For the length of the string
	while (*src) {
		if (*src == '%') {
			unsigned char c = 0;
			src++;
			if (*src >= '0' && *src <= '9') c += (*src - '0');
			else  if (*src >= 'a' && *src <= 'f') c += (*src - 'a' + 10);
			else if (*src >= 'A' && *src <= 'F') c += (*src - 'A' + 10);
			c *= 16;
			src++;
			if (*src >= '0' && *src <= '9') c += (*src - '0');
			else  if (*src >= 'a' && *src <= 'f') c += (*src - 'a' + 10);
			else if (*src >= 'A' && *src <= 'F') c += (*src - 'A' + 10);
			result += c;
		}
		else {
			result += *src;
		}
		src++;
	}
	return result;
}

// Escape characters - add a '\' before any characters in escapees
string escape_string(const string text, const string escapees) {
	// Create a string sufficiently long to escape all characters
	string result = "";
	result.reserve(2 * text.length());
	// For the length of the string
	for (size_t i = 0; i < text.length(); i++) {
		// If the character is in one of escapeed, add a '\'
		if (strchr(escapees.c_str(), text[i]) != nullptr) {
			result += '\\';
		}
		result += text[i];
	}
	return result;
}

// Unescape characters - remove the '\' before ant character
string unescape_string(const string text) {
	// Create a string at least as long as the supplied string
	string result = "";
	result.reserve(text.length());
	// For the length of the string
	for (size_t i = 0; i < text.length(); i++) {
		// If the character is '\' do not copy it
		if (text[i] != '\\') {
			result += text[i];
		}
	}
	return result;
}

// Convert a floating point degree value to ° ' " N/E/S/W
string degrees_to_dms(float value, bool is_latitude) {
	int num_degrees;
	int num_minutes;
	double num_seconds;
	string text;
	// Strip sign off
	num_seconds = abs(value * 60.0 * 60.0);
	// Divide by 60 to get number of whole minutes
	num_minutes = (int)num_seconds / 60;
	// Get the number of additional seconds
	num_seconds = num_seconds - (double)(((long)num_minutes) * 60);
	// Divide by 60 to get number of degress
	num_degrees = num_minutes / 60;
	// Get additional minutes
	num_minutes = num_minutes % 60;
	// Now format the text 
	char temp[20]; // I count 11 but add a bit
	snprintf(temp, 20, "%d°%d'%.1f\"%c", num_degrees, num_minutes, num_seconds, value < 0 ? (is_latitude ? 'S' : 'W') : (is_latitude ? 'N' : 'E'));
	text = temp;
	return text;
}

// Convert latitude and longitode to 2, 4, 6, 8, 10 or 12 character grid square
string latlong_to_grid(lat_long_t location, int num_chars) {
	string result;
	result.resize(num_chars, ' ');
	// 'Normalise' location relative to 180W, 90S - i.e. AA00aa00aa00
	double norm_lat = location.latitude + 90.0;
	double norm_long = location.longitude + 180.0;
	double inc_lat = 10;
	double inc_long = 20;
	for (int i = 0; i < num_chars; i += 2) {
		switch (i) {
		case 0:
		case 4:
		case 8:
			result[i] = (int)trunc(norm_long / inc_long) + 'A';
			result[i + 1] = (int)trunc(norm_lat / inc_lat) + 'A';
			norm_long = fmod(norm_long, inc_long);
			norm_lat = fmod(norm_lat, inc_lat);
			inc_long /= 10.0;
			inc_lat /= 10.0;
			break;
		case 2:
		case 6:
		case 10:
			result[i] = (int)trunc(norm_long / inc_long) + '0';
			result[i + 1] = (int)trunc(norm_lat / inc_lat) + '0';
			norm_long = fmod(norm_long, inc_long);
			norm_lat = fmod(norm_lat, inc_lat);
			inc_long /= 24.0;
			inc_lat /= 24.0;
			break;
		}
	}
	return result;
}

// Convert gridsquare to latitude and longitude - returns the centre of the square
lat_long_t grid_to_latlong(string gridsquare) {
	double inc = 20.0;
	double next_inc;
	char cg;
	char ct;
	lat_long_t lat_long = { 0.0, 0.0 };
	for (unsigned int i = 0; i < gridsquare.length(); i += 2) {
		switch (i) {
		case 0:
			// First two letters - 18 * 18 squares - 10 * 20 degress
			cg = gridsquare[i] - 'A' - 9;
			ct = gridsquare[i + 1] - 'A' - 9;
			next_inc = inc / 10.0;
			break;
		case 4:
		case 8:
			// Second or third two letters - 24 * 24 squares - 2.5 * 5 minutes degress
			cg = gridsquare[i] - 'A' - 12;
			ct = gridsquare[i + 1] - 'A' -12;
			next_inc = inc / 10.0;
			break;
		case 2:
		case 6:
		case 10:
			// Numbers - 10 * 10 squares - 1 * 2 degress
			cg = gridsquare[i] - '0' - 5;
			ct = gridsquare[i + 1] - '0' - 5;
			next_inc = inc / 24.0;
			break;
		}
		lat_long.longitude += ((double)cg * inc) + inc / 2.0;
		lat_long.latitude += ((double)ct * inc / 2.0) + inc / 4.0;
		inc = next_inc;
	}
	return lat_long;
}


// Convert from base64 encoding - single character
unsigned char decode_base_64(unsigned char c) {
	// A-Z => 0x00 to 0x19
	// a-z => 0x1A to 0x33
	// 0-9 => 0x34 to 0x3D
	// +   => 0x3E
	// /   => 0x3F
	// =   => 0xFF (padding byte)
	if (c >= 'A' && c <= 'Z') {
		return c - 'A';
	}
	else if (c >= 'a' && c <= 'z') {
		return c - 'a' + 0x1A;
	}
	else if (c >= '0' && c <= '9') {
		return c - '0' + 0x34;
	}
	else if (c == '+') {
		return 0x3E;
	}
	else if (c == '/') {
		return 0x3F;
	}
	else {
		return (unsigned char)0xFF;
	}
}

// Convert from base64 encoding for string
string decode_base_64(string value) {
	string result;
	unsigned char out = 0;
	unsigned char in = 0;
	// Each data byte provides 6 bits of the required string
	for (size_t i = 0; i < value.length(); i++) {
		// Get the base64 character
		unsigned char base_64 = value[i];
		// Convert to the next 6 bits of the string
		in = decode_base_64(base_64);
		// Is it a padding byte?
		if (in != 0xFF) {
			// 4 base64 characters make 3 bytes
			switch (i % 4) {
			case 0:
				// First 6 bits of output byte 0
				out = in << 2;
				break;
			case 1:
				// Last 2 bits of output byte 0
				out |= (in & 0x30) >> 4;
				result += out;
				// First 4 bytes of output byte 1
				out = (in & 0x0F) << 4;
				break;
			case 2:
				// last 4 bits of output byte 1
				out |= (in & 0x3C) >> 2;
				result += out;
				// first 2 bits of output byte 2
				out = (in & 0x03) << 6;
				break;
			case 3:
				// last 6 bits of output byte 2
				out |= in;
				result += out;
				out = 0;
				break;
			}
		}
	}
	return result;
}

// Encode single character to base64
unsigned char encode_base_64(unsigned char c) {
	// Look up table to convert 6 bits to 
	// A-Z => 0x00 to 0x19
	// a-z => 0x1A to 0x33
	// 0-9 => 0x34 to 0x3D
	// +   => 0x3E
	// /   => 0x3F
	// =   => 0xFF (padding byte)
	const string look_up = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (c < 64)	return look_up[c];
	else return '=';
}

// Encode the string to base64
string encode_base_64(string value) {
	string result;
	unsigned char out = 0;
	unsigned char in = 0;
	// Each 6 bits of the input string provides a byte of the output string
	for (size_t i = 0; i < value.length(); i++) {
		in = value[i];
		// 3 bytes of input generate 4 bytes of output
		switch (i % 3) {
		case 0:
			// write out first 6 bits of byte 0
			out = (in & 0xFC) >> 2;
			result += encode_base_64(out);
			// save remeber last 2 bytes
			out = (in & 0x03) << 4;
			break;
		case 1:
			// concatenate first 4 bits of byte 1
			out |= (in & 0xF0) >> 4;
			result += encode_base_64(out);
			// save last 4 bytes
			out = (in & 0x0F) << 2;
			break;
		case 2:
			// concatenate first 2 bits of byte 2
			out |= (in & 0xC0) >> 6;
			result += encode_base_64(out);
			// write iut last 6 bits
			out = in & 0x3F;
			result += encode_base_64(out);
			break;
		}
		// Pad the resulting string
		while (result.length() % 4 != 0) {
			result += '=';
		}
	}
	return result;
}

// Decode hex
string to_hex(string data) {
	string result = "";
	for (size_t i = 0; i < data.length(); i++) {
		result += to_hex(data[i]);
	}
	return result;
}

// Encode hex
string to_ascii(string data) {
	string result = "";
	unsigned int ix = 0;
	while (ix < data.length()) {
		result += to_ascii(data, (signed int&)ix);
	}
	return result;
}

// Decode single character
string to_hex(unsigned char data, bool add_space /* = true */) {
	const char lookup[] = "0123456789ABCDEF";
	string result = "";
	result += lookup[data / 16];
	result += lookup[data % 16];
	if (add_space) {
		result += ' ';
	}
	return result;
}

// Encode single character
unsigned char to_ascii(string data, int&ix) {
	// Skip non hex
	while (!isxdigit(data[ix]) && ((unsigned)ix < data.length())) ix++;
	if (ix == data.length()) return 0;
	size_t next_ix;
	int val = stoi(data.substr(ix, 2), &next_ix, 16);
	ix += (int)next_ix;
	return (unsigned char)val;
}

// Convert int to BCD
string int_to_bcd(int value, int size, bool least_first) {
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
int bcd_to_int(string bcd, bool least_first) {
	int result = 0;
	if (least_first) {
		for (size_t i = bcd.length(); i > 0;) {
			unsigned char c = bcd[--i];
			int ls = c & '\x0f';
			int ms = c & '\xf0';
			ms >>= 4;
			result = result * 100 + ls + 10 * ms;
		}
	}
	else {
		for (size_t i = 0; i < bcd.length(); i++) {
			unsigned char c = bcd[i];
			int ls = c & '\x0f';
			int ms = c & '\xf0';
			ms >>= 4;
			result = result * 100 + ls + 10 * ms;
		}
	}
	return result;
}

// Convert BCD to doble
double bcd_to_double(string bcd, int decimals, bool least_first) {
	double digit_value = 1.0;
	for (int i = 0; i < decimals; i++) {
		digit_value *= 0.1;
	}
	double result = 0.0;
	if (!least_first) {
		for (size_t i = bcd.length(); i > 0;) {
			unsigned char c = bcd[--i];
			int digit = c & '\x0f';
			result += digit * digit_value;
			digit_value *= 10.0;
			digit = (c & '\xf0') >> 4;
			result += digit * digit_value;
			digit_value *= 10.0;
		}
	}
	else {
		for (size_t i = 0; i < bcd.length(); i++) {
			unsigned char c = bcd[i];
			int digit = c & '\x0f';
			result += digit * digit_value;
			digit_value *= 10.0;
			digit = (c & '\xf0') >> 4;
			result += digit * digit_value;
			digit_value *= 10.0;
		}
	}
	return result;
}

// Convert string to hex
string string_to_hex(string data, bool escape /*=true*/) {
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

// Convert hex representation to string
string hex_to_string(string data) {
	string result;
	int ix = 0;
	// For the length of the source striing
	while ((unsigned)ix < data.length()) {
		if (data[ix] == 'x') {
			ix++;
		}
		result += to_ascii(data, ix);
	}
	return result;
}

// Default message function
void default_error_message(status_t level, const char* message) {
	switch (level) {
	case ST_NONE:             // Uninitialised
	case ST_LOG:              // Only log the message, do not display it in status
	case ST_NOTE:             // An information message
	case ST_OK:               // Task successful
	case ST_WARNING:          // A warning message
		fl_message(message);
		break;
	case ST_ERROR:            // An error has been signaled
	case ST_SEVERE:           // A sever error that will result in reduced capability
	case ST_FATAL:             // A fatal (non-recoverable) error has been signaled
		fl_alert(message);
		break;
	}
}

// Calculate the great circle bearing and distance between two locations on the Earth's surface
void great_circle(lat_long_t source, lat_long_t destination, double& bearing, double& distance)
{
	// difference in longitude in radians
	double d_long = (destination.longitude - source.longitude) * DEGREE_RADIAN;
	double cos_d_long = cos(d_long);
	double sin_d_long = sin(d_long);
	// Latitudes in radians
	double src_latitude = source.latitude * DEGREE_RADIAN;
	double dest_latitude = destination.latitude * DEGREE_RADIAN;
	double cos_s_lat = cos(src_latitude);
	double sin_s_lat = sin(src_latitude);
	double tan_d_lat = tan(dest_latitude);
	double cos_t_lat = cos(dest_latitude);
	double sin_t_lat = sin(dest_latitude);

	// bearing (azimuth)
	// tan(a) = ( sin(l2-l1) / ( cos(ph1).tan(ph2) - sin(ph1).cos(l2-l1) )
	// l(ambda) - longitude, ph(i) - latitude
	// calculate denominator and use atan2 to get correct quadrant
	double denominator = ((cos_s_lat * tan_d_lat) - (sin_s_lat * cos_d_long));
	bearing = atan2(sin_d_long, denominator) * RADIAN_DEGREE;
	// Convert from -180->0 to +180->+360
	if (bearing < 0.0) bearing += 360.0;

	// Great circle distance
	// s = angle at centre of sphere
	// cos(s) = ( sin(ph1).sin(ph2) + cos(ph1).cos(ph2).cos(l2-l1) )
	double cos_angle = (sin_s_lat * sin_t_lat) + (cos_s_lat * cos_t_lat * cos_d_long);
	// Convert to distance = radius * angle in radians
	distance = EARTH_RADIUS * acos(cos_angle);
}
