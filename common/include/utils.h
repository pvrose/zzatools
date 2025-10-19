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
#include <cmath>

#include <FL/Fl_Widget.H>





	//! Latitude/longitude pair
	struct lat_long_t {
		double latitude{ 0.0 };    //!< latitude (positive = N)
		double longitude{ 0.0 };   //!< longitude (positive = E)

		//! Return NAN if either coordinate is not a valid number.
		bool is_nan() {
			return std::isnan(latitude) || std::isnan(longitude);
		}

		//! Returns true if both cordinates compare.
		bool operator==(lat_long_t rhs) {
			return (latitude == rhs.latitude && longitude == rhs.longitude);
		}
	};

	//! Tooltip display - 2 seconds
	const double TIP_SHOW = 2.0;
	//! π to maximum precision in double - let the compiler take the strain
	const double PI = 3.14159265358979323846264338327950288419716939937510;
	// Conversion factors - angle
	const double DEGREE_RADIAN = PI / 180.0;    //!< Conversion from degress to radians
	const double RADIAN_DEGREE = 180.0 / PI;    //!< Conversion from radians to degrees

	//! radius of earth in kilometres - yes I know the earth is an oblate sphere
	const double EARTH_RADIUS = 6371.0088;

	//! Split a text \p line into separate \p words on \p separator
	void split_line(const std::string& line, std::vector<std::string>& words, const char separator);
	//! Recombine separate \p words into a std::string with \p separator and return the result.
	std::string join_line(std::vector<std::string> words, const char separator);
	//! Converts display format text to a tm object for reformatting
	bool string_to_tm(std::string text, tm& time, std::string format);
	//! Convert a std::string e.g. 00-06:08 to an array of UINTs {0,1,2,3,4,5,6,8}
	void string_to_ints(std::string& text, std::vector<unsigned int>& ints);
	//! Returns the current time in specific fomat
	std::string now(bool local, const char* format, bool add_ms = false);
	//! Returns the current time to milliseconds
	std::string now_ms();
	//! Returns true if the /p date is a leap year
	bool is_leap(tm* date);
	//! refresh the tm struct after modifying it
	void refresh_tm(tm* date);
	//! Retunrs the number of days in the month
	int days_in_month(tm* date);
	//! Create a tip window - data \p tip, position(\p root_x, \p root_y)
	Fl_Window* tip_window(const std::string& tip, int x_root, int y_root);
	//! Returns \p data in upper case
	std::string to_upper(const std::string& data);
	//! Returns \p data in lower case
	std::string to_lower(const std::string& data);
	//! Returns true if the whole std::string is an integer
	bool is_integer(const std::string& data);
	//! Returns the position of any characters in \p match in \p data (\p length): returns \p length if not found. 
	size_t find(const char* data, size_t length, const char* match);
	//! Returns the position of \p match in \p data (\p length): returns \p length if not found. 
	size_t find(const char* data, size_t length, const char match);
	//! Returns the position of std::string \p match (\p len_substr) in \p data (\p length): returns \p length if not found. 
	size_t find_substr(const char* data, size_t length, const char* match, size_t len_substr);
	//! Returns the position of any characters not in \p match in \p data (\p length): returns \p length if not found. 
	size_t find_not(const char* data, size_t length, const char* match);
	//! \brief Returns \p text with any character in \p chars (\p allow = false) or
	//! not in \p chars (\p allow = true) replaced with hex representation. 
	std::string escape_hex(std::string text, bool allow, const char* chars);
	//! Returns \p text with any non-alphanumeric characters replaced with hex representation 
	std::string escape_url(std::string text);
	//! Returns \p text with && and %% characters escaped for menu items.
	std::string escape_menu(std::string text);
	//! Returns \p text with hex characters (%%nnx) replaced by 8-bit character.
	std::string unescape_hex(std::string text);
	//! Escape characters - Returns \p text adding a '\' before any characters in \p escapees.
	std::string escape_string(const std::string text, const std::string escapees);
	//! Unescape characters - Returns .p with '\' removed.
	std::string unescape_string(const std::string text);
	//! \brief Returns \p value expressed as degrees, minutes and seconds: 
	//! if \p is_lat is true treat as latitude, otherwise as longitude.
	std::string degrees_to_dms(float value, bool is_lat);
	//! Returns coordinate pair as gridsquare equivalent with \p num_chars characters.
	std::string latlong_to_grid(lat_long_t location, int num_chars);
	//! Returns coordinate pair for \p gridsquare.
	lat_long_t grid_to_latlong(std::string gridsquare);
	//! Returns character equivalent of base64 encoded character \p c
	
	/*! \code
	A-Z => 0x00 to 0x19
	a-z => 0x1A to 0x33
	0-9 => 0x34 to 0x3D
    +   => 0x3E
	/   => 0x3F
	=   => 0xFF (padding byte)
	\endcode
	*/
	unsigned char decode_base_64(unsigned char c);
	//! Returns decoded base64 std::string \p s.
	std::string decode_base_64(std::string s);
	//! Returns base64 encoding of 6-bit character \p c

	/*! \code
	A-Z <= 0x00 to 0x19
	a-z <= 0x1A to 0x33
	0-9 <= 0x34 to 0x3D
	+   <= 0x3E
	/   <= 0x3F
	=   <= 0x40 to 0xFF
	\endcode
	*/
	unsigned char encode_base_64(unsigned char c);
	//! Returns base64 encoding of std::string \p s.
	std::string encode_base_64(std::string s);
	//! Returns \p data as hex encoded std::string.
	std::string to_hex(std::string data);
	//! Returns hex-encoded std::string \p data as std::string of 8-bit characters.
	std::string to_ascii(std::string data);
	//! Returns std::string representing hex encode of \p data: a std::string is added in \p add_space is true.
	std::string to_hex(unsigned char data, bool add_space = true);
	//! \brief Returns the single 8-bit byte from hex-encoded std::string \p data at position \p ix, which then
	//! points to the position after the decoded characters. 
	unsigned char to_ascii(std::string data, int& ix);
	//! Returns value in BCD format.
	
	//! \param value Integer to encode.
	//! \param size umber of bytes in encoded std::string.
	//! \param least_first Data returned least signficant byte first.
	std::string int_to_bcd(int value, int size, bool least_first);
	//! Returns BCD value as an integer
	
	//! \param least_first First byte of data is least significant.
	int bcd_to_int(std::string, bool least_first);
	//! Returns BCD value as a double-precision value

	//! \param decimals Number of characters after the decimal point.
	//! \param least_first First byte of data is least significant.
	double bcd_to_double(std::string, int decimals, bool least_first);
	//! Convert std::string to hex
	std::string string_to_hex(std::string, bool escape = false);
	//! Convert std::string to hex
	std::string hex_to_string(std::string);
	//! Calculate the great circle bearing and distance between two locations on the Earth's surface
	
	//! \param source Coordinates of location measuring from.
	//! \param destination Coordinates of location measuring to.
	//! \param bearing Receives the bearing from \p source to \p destination (in degress).
	//! \param distance Receives the distance from \p sourec to \p destination (in kilometres). 
	void great_circle(lat_long_t source, lat_long_t destination, double& bearing, double& distance);
	//! Replace '/' with '_' throughout \p data.
	void de_slash(std::string& data);
	//! Replace '_' with '/' throughout \p data.
	void re_slash(std::string& data);
	//! Replace '\\' (single backslass character) with '/'
	void forward_slash(std::string& data);
	//! Returns the directroy part of a a filename
	std::string directory(std::string filename);
	//! Returns the terminal part of a filename.
	std::string terminal(std::string filename);
	//! Returns a simple 8-bit hash of \p src (zero-terminated)
	uchar hash8(const char* src);
	//! Performs "XOR" encrypt/decrypt
	 
	//! \param str std::string of chaarcters to encrypt/decrypt in-place.
	//! \param len length of std::string \p str. Zero-termination cannot be used 
	//! as an encrypted value may contain them.
	//! \param seed Seed to generate a pseudo-random sequence of characters to use as an encryption key.
	//! \param offset Offset into generated pseudo-random sequence to start process.
	void xor_crypt(char* str, int len, uint32_t seed, uchar offset);
	//! Performs "XOR" encrypt/decrypt using std::string data and returns encrypted std::string.
	
	//! \see xor_crypt.
	std::string xor_crypt(std::string, uint32_t seed, uchar offset);

	//! Convert ISO date-time format to time_t
	std::time_t convert_iso_datetime(std::string value);

	//! Convert ISO date-time format from time_t
	std::string convert_iso_datetime(std::time_t t);

	//! Returns the widget of class WIDGET that encloses \p w.
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