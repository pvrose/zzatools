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
		//snprintf(mess, 256, "DEBUG: Received response from IC-7300: %s", raw_data.c_str());
		//message(ST_LOG, mess);

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
				snprintf(mess, 256, "RIG: Receieved no response from transceiver - CMD = %s", to_send.c_str());
				message(ST_WARNING, mess);
				given_warning_ = true;
			}
			ok = false;
			return "";
		}
		else {
			// For data - strip off reflected command if its present - note I have seen command returned multiple times
			int discard = cmd.length();
			int number_discards = 0;
			while (response.substr(0, discard) == cmd) {
				response = response.substr(discard);
				number_discards++;
			}
			//if (number_discards > 1) {
			//	snprintf(mess, 256, "RIG: Discarded %d copies of the reflected command %s", number_discards, hex_to_string(cmd).c_str());
			//	message(ST_DEBUG, mess);
			//}
			if (response.length() == 6 && response[4] > '\xf0') {
				if (response[4] == '\xfa') {
					// Got NAK response from transceiver
					snprintf(mess, 256, "RIG: Received a NAK response from transceiver - CMD = %s", to_send.c_str());
					message(ST_ERROR, mess);
					ok = false;
					return "";
				}
				else {
					//// Got ACK response from transceiver, but no data
					//snprintf(mess, 256, "RIG: Received an ACK response from transceiver - CMD = %s", to_send.c_str());
					//message(ST_DEBUG, mess);
					return ("");
				}
			}
			else {
				string expected = "\xFE\xFE\xE0\x94";
				if (response.length() >= 4 && response.substr(0, 4) == expected) {
					// Discard the CI-V preamble/postamble - xFExFEx94xF0 data xFD
					snprintf(mess, 256, "RIG: Received response %s from transceiver - CMD = %s", string_to_hex(response.substr(4)).c_str(), to_send.c_str());
					return response.substr(4, response.length() - 5);
				}
				else {
					// Not addressed to this
					snprintf(mess, 256, "RIG: Received a response for another node - CMD = %s RESP = %s", to_send.c_str(), response.c_str());
					return "";
				}
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