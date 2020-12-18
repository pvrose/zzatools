#include "ic7300.h"
#include "rig_if.h"


using namespace zzalib;

extern rig_if* rig_if_;
ic7300* ic7300_;

ic7300::ic7300() :
given_warning_(false)
{
	message = default_error_message;
}

ic7300::~ic7300() {

}


// Send command with zero length data
string ic7300::send_command(unsigned char command, string sub_command, bool& ok) {
	return send_command(command, sub_command, "", ok);
}

// Send command with data
string ic7300::send_command(unsigned char command, string sub_command, string data, bool& ok) {
	ok = true;
	char mess[256];
	// Only access if we have a rig connected
	if (rig_if_) {
		// THis is hard-coded for default IC-7300 (address 0x94)
		string cmd = "\xFE\xFE\x94\xE0";
		cmd += command;
		cmd += sub_command;
		cmd += data;
		cmd += '\xfd';
		string to_send = string_to_hex(cmd, true);
		// Send the message
		string raw_data = rig_if_->raw_message(to_send);

		// Ignore reflected data
		if (raw_data.substr(0, 2) != "FE") {
			// If this is the first warning
			if (!given_warning_) {
				snprintf(mess, 256,"RIG: Response not from IC-7300: %s", raw_data.c_str());
				message(ST_WARNING, mess);
				given_warning_ = true;
			}
			ok = false;
			return raw_data;
		}
		// Decode the response
		string response = hex_to_string(raw_data);
		if (response.length() == 0) {
			// TRansceiver has not responded at all
			if (!given_warning_) {
				snprintf(mess, 256, "RIG: Receieved no response from transceiver\nCMD = %s", to_send.c_str());
				message(ST_WARNING, mess);
				given_warning_ = true;
			}
			ok = false;
			return "";
		}
		else {
			// For data - strip off reflected command if its present
			int discard = cmd.length();
			if (response.substr(0, discard) == cmd.substr(0, discard)) {
				response = response.substr(discard);
				if (response.length() == 6 && response[4] > '\xf0') {
					if (response[4] == '\xfa') {
						// Got NAK response from transceiver
						snprintf(mess, 256, "RIG: Received a NAK response from transceiver\nCMD = %s", to_send.c_str());
						message(ST_ERROR, mess);
						ok = false;
						return "";
					}
					else {
						// Got ACK response from transceiver, but no data
						return ("");
					}
				}
				else {
					// Discard the CI-V preamble/postamble - xFExFEx94xF0 data xFD
					return response.substr(4, response.length() - 5);
				}
			}
			else {
				// Currently do not support the case where the rig is configured not to reflect command? 
				// This will also trap corrupted responses
				snprintf(mess, 256, "RIG: Unexpected response: %s\n%s\nCMD = %s", response.c_str(), string_to_hex(response).c_str(), to_send.c_str());
				message(ST_ERROR, mess);
				ok = false;
				return response;
			}
		}
	}
	else {
		ok = false;
		return "NO RIG";
	}
}

void ic7300::callback(void (*mess_func)(status_t, const char*)) {
	message = mess_func;
}