/*
ZZALOG - Amateur Radio Log
© - Copyright 2017, Philip Rose, GM3ZZA
All Rights Reserved

utils.h - various utility methods
*/
#ifndef __UTILS__
#define __UTILS__

#include <string>
#include <ctime>
#include <vector>

#include <FL/Fl_Widget.H>

using namespace std;

namespace zzalib {

	// Latitude/longitude pair
	struct lat_long_t {
		double latitude;    // latitude (positive = N)
		double longitude;   // longitude (positive = E)
	};

	// Tip display - 2 seconds
	const double TIP_SHOW = 2.0;
	// Library version
	const string LIBRARY_VERSION = "1.0.2";

	// Split a text line into separate "words" on separator
	void split_line(const string& line, vector<string>& words, const char separator);
	// Converts display format text to a tm object for reformatting
	void string_to_tm(string text, tm& time, string format);
	// Convert a string e.g. 00-06:08 to an array of UINTs {0,1,2,3,4,5,6,8}
	void string_to_ints(string& text, vector<unsigned int>& ints);
	// get the time in specific fomat
	string now(bool local, const char* format);
	// test for leap year
	bool is_leap(tm* date);
	// refresh the tm struct after modifying it
	void refresh_tm(tm* date);
	// get the days in the month
	int days_in_month(tm* date);
	// Create a tip window - data tip, position(root_x, root_y)
	Fl_Window* tip_window(const string& tip, int x_root, int y_root);
	// Create upper version of a string
	string to_upper(const string& data);
	// Create lower version of a string
	string to_lower(const string& data);
	// Limited string search - return the position of the sought char(s) or length if not present
	size_t find(const char* data, size_t length, const char* match);
	size_t find(const char* data, size_t length, const char match);
	size_t find_substr(const char* data, size_t length, const char* match, size_t len_substr);
	size_t find_not(const char* data, size_t length, const char* match);
	// Escape certain characters (when generating URLs
	string escape_url(string text);
	// Escape characters - add a '\' before any characters in escapees
	string escape_string(const string text, const string escapees);
	// Convert floating degree value to ° ' " (degree, minute, second)
	string degrees_to_dms(float value, bool is_lat);
	// Convert lat long pair to gridsquare
	string latlong_to_grid(lat_long_t location, int num_chars);
	// Convert grid square to lat long pair
	lat_long_t grid_to_latlong(string gridsquare);
	// Decode Base 64
	unsigned char decode_base_64(unsigned char c);
	string decode_base_64(string s);
	// Encode Base 64
	unsigned char encode_base_64(unsigned char c);
	string encode_base_64(string s);
	// Decode hex
	string to_hex(string data);
	// Encode hex
	string to_ascii(string data);
	// Decode single ch aracter
	string to_hex(unsigned char data, bool add_space = true);
	// Encode single character - starting at data[ix] and incrementing ix to end of it.
	unsigned char to_ascii(string data, int& ix);
	// Convert int to BCD
	string int_to_bcd(int value, int size, bool least_first);
	// Convert BCD to int
	int bcd_to_int(string, bool least_first);
	// Convert BCD tp Float
	double bcd_to_double(string, int decimals, bool least_first);
	// Convert string to hex
	string string_to_hex(string, bool escape = false);
	// Convert string to hex
	string hex_to_string(string);


	// template function to find the enclosing widget of class WIDGET
	template <class WIDGET>
	WIDGET* ancestor_view(Fl_Widget* w) {
		Fl_Widget* p = w;
		// Keep going up the parent until we found one that casts to WIDGET or we run out of ancestors
		while (p != nullptr && dynamic_cast<WIDGET*>(p) == nullptr) {
			p = p->parent();
		}
		// Return null if we don't find one, else the one we did
		if (p == nullptr) return nullptr;
		else return dynamic_cast<WIDGET*>(p);
	}

};

#endif