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

		// Diagnostic information
		snprintf(mess, 256, "RIG: Sent raw command to IC-7300: %s", to_send.c_str());
		message(ST_DEBUG, mess);
		snprintf(mess, 256, "RIG: Received response from IC-7300: %s", raw_data.c_str());
		message(ST_DEBUG, mess);


		// Decode the response
		// Parse the received data
		// Convert to bytes rather than hex
		string response = hex_to_string(raw_data);
		const char* src = response.c_str();
		const char* start = response.c_str();
		string result = "";
		int len = response.length();
		// Skip to response
		bool response_found = false;
		while (!response_found && src - start < len) {
			// Skip leading xFE
			while (src - start < len && *src == '\fe') {
				src++;
			}
			if (src - start < len) {
				if (*src == cmd[2] && src[1] == cmd[3]) {
					// command repeated
					src += 2;
					while (*src != '\fd') {
						src++;
					}
					src++;
				}
				else if ((*src == cmd[3] || *src == '\x00') && src[1] == cmd[2]) {
					response_found = true;
					src += 2;
					bool ended = false;
					while (!ended && src - start < len) {
						switch (*src) {
						case '\xfa':
							// Got NAK response from transceiver
							snprintf(mess, 256, "RIG: Received a NAK response from transceiver - CMD = %s", to_send.c_str());
							message(ST_ERROR, mess);
							ok = false;
							return "";
						case '\xfb':
							//// Got ACK response from transceiver, but no data
							//snprintf(mess, 256, "RIG: Received an ACK response from transceiver - CMD = %s", to_send.c_str());
							//message(ST_DEBUG, mess);
							return result;
						case '\xfd':
							snprintf(mess, 256, "RIG: Received response data %s - CMD = %s", string_to_hex(result).c_str(), to_send.c_str());
							message(ST_DEBUG, mess);
							return result;
						default:
							result += *src;
							src++;
							break;
						}
					}
				}
				else {
					snprintf(mess, 256, "RIG: Unexpected response %s - CMD = %s", string_to_hex(response).c_str(), to_send.c_str());
					return "";
				}
			}
		}

		if (!response_found) {
			// Not decoded response (no xFD encountered)
			if (!given_warning_) {
				snprintf(mess, 256, "RIG: Receieved invalid response %s - CMD = %s", string_to_hex(response).c_str(), to_send.c_str());
				message(ST_WARNING, mess);
				given_warning_ = true;
			}
			ok = false;
			return "";
		}

	}

	// I don't think I can fall out into here
	return "";

}

void ic7300::callback(void (*mess_func)(status_t, const char*)) {
	message = mess_func;
}