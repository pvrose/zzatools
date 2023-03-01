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

	// Latitude/longitude pair
	struct lat_long_t {
		double latitude;    // latitude (positive = N)
		double longitude;   // longitude (positive = E)
	};

	// The status of the various messages
	enum status_t {
		ST_NONE,             // Uninitialised
		ST_LOG,              // Only log the message, do not display it in status
		ST_DEBUG,            // Debug message
		ST_NOTE,             // An information message
		ST_OK,               // Task successful
		ST_WARNING,          // A warning message
		ST_NOTIFY,           // A notification to the user
		ST_ERROR,            // An error has been signaled
		ST_SEVERE,           // A sever error that will result in reduced capability
		ST_FATAL             // A fatal (non-recoverable) error has been signaled
	};



	// Tip display - 2 seconds
	const double TIP_SHOW = 2.0;
	// π to maximum precision in double - let the compiler take the strain
	const double PI = 3.14159265358979323846264338327950288419716939937510;
	// Conversion factors - angle
	const double DEGREE_RADIAN = PI / 180.0;
	const double RADIAN_DEGREE = 180.0 / PI;
	// radius of earth in kilometers - yes I know the earth is an oblate sphere
	const double EARTH_RADIUS = 6371.0088;

	// Split a text line into separate "words" on separator
	void split_line(const string& line, vector<string>& words, const char separator);
	// Converts display format text to a tm object for reformatting
	bool string_to_tm(string text, tm& time, string format);
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
	// Check the whole string is an integer
	bool is_integer(const string& data);
	// Limited string search - return the position of the sought char(s) or length if not present
	size_t find(const char* data, size_t length, const char* match);
	size_t find(const char* data, size_t length, const char match);
	size_t find_substr(const char* data, size_t length, const char* match, size_t len_substr);
	size_t find_not(const char* data, size_t length, const char* match);
	// Escape certain characters (when generating URLs)
	string escape_hex(string text, bool allow, const char* chars);
	string escape_url(string text);
	string escape_menu(string text);
	string unescape_hex(string text);
	// Escape characters - add a '\' before any characters in escapees
	string escape_string(const string text, const string escapees);
	// Unescape characters - remove '\'
	string unescape_string(const string text);
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
	// Default error message
	void default_error_message(status_t level, const char* message);
	// Calculate the great circle bearing and distance between two locations on the Earth's surface
	void great_circle(lat_long_t source, lat_long_t destination, double& bearing, double& distance);


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


#endif