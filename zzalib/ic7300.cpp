#include "ic7300.h"
#include "rig_if.h"

using namespace zzalib;

extern rig_if* rig_if_;
ic7300* ic7300_;

ic7300::ic7300() :
given_warning(false)
{

}

ic7300::~ic7300() {

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
			if (!given_warning) {
				fl_alert("Response not from IC-7300: %s", raw_data.c_str());
				given_warning = true;
			}
			ok = false;
			return raw_data;
		}
		string response = hex_to_string(raw_data);
		if (response.length() == 0) {
			// TRansceiver has not responded at all
			if (!given_warning) {
				fl_alert("Receieved no response from transceiver\nCMD = %s", to_send.c_str());
				given_warning = true;
			}
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
						fl_alert("Received a NAK response from transceiver\nCMD = %s", to_send.c_str());
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
				fl_alert("Unexpected response: %s\n%s\nCMD = %s", response.c_str(), string_to_hex(response).c_str(), to_send.c_str());
				return response;
			}
		}
	}
	else {
		ok = false;
		return "NO RIG";
	}
}
