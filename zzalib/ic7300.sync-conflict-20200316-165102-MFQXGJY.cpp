#include "ic7300.h"
#include "rig_if.h"

using namespace zzalib;

extern rig_if* rig_if_;
ic7300* ic7300_;

// Convert int to BCD
string ic7300::int_to_bcd(int value, int size, bool least_first) {
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
int ic7300::bcd_to_int(string bcd, bool least_first) {
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
double ic7300::bcd_to_double(string bcd, int decimals, bool least_first) {
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

// Send command
string ic7300::send_command(unsigned char command, string sub_command, bool& ok) {
	return send_command(command, sub_command, "", ok);
}

string ic7300::send_command(unsigned char command, string sub_command, string data, bool& ok) {
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
		if (raw_data.substr(0, 2) != "FE") {
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
					return response.substr(4, response.length() - 5);
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

string ic7300::string_to_hex(string data, bool escape /*=true*/) {
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

string ic7300::hex_to_string(string data) {
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
