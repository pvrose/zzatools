#include "serial.h"

#include <iostream>

#include <Windows.h>

serial::serial() {
	h_port_ = nullptr;
};

serial::~serial() {
}

bool serial::open_port(string port_name, unsigned int baud_rate, bool monitor) {
	// Try an open the port - return false if fail.
	port_name_ = port_name;
	char* device = new char[5 + port_name.length()];
	snprintf(device, sizeof(device), "//./%s", port_name.c_str());
	DWORD access_mode;
	if (monitor) {
		access_mode = GENERIC_READ;
	}
	else {
		access_mode = GENERIC_READ | GENERIC_WRITE;
	}
	h_port_ = CreateFile(device, access_mode, 0, 0, OPEN_EXISTING, 0, 0);
	if (h_port_ == INVALID_HANDLE_VALUE) {
		log_error("CreateFile");
		return false;
	}

	// Now attempt to configure port
	DCB control;
	if (!GetCommState(h_port_, &control)) {
		log_error("GetCommState");
		CloseHandle(h_port_);
		return false;
	}
	// Now set the required configuration
	control.DCBlength = sizeof(control);
	control.BaudRate = baud_rate;
	control.ByteSize = 8;
	control.Parity = NOPARITY;
	control.StopBits = ONESTOPBIT;
	control.fBinary = true;
	control.fDsrSensitivity = false;
	control.fParity = false;
	control.fOutX = false;
	control.fInX = false;
	control.fNull = false;
	control.fAbortOnError = false;
	control.fOutxCtsFlow = false;
	control.fOutxDsrFlow = false;
	control.fErrorChar = false;

	if (!SetCommState(h_port_, &control)) {
		log_error("SetCommStae");
		CloseHandle(h_port_);
		return false;
	}

	// Set timeouts
	COMMTIMEOUTS timeouts;
	// Wait upto 10 s for any data to come back
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = 10000;
	timeouts.WriteTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	if (!SetCommTimeouts(h_port_, &timeouts)) {
		log_error("SetCommTimeouts");
		CloseHandle(h_port_);
		return false;
	}

	return true;
}

// Copy data from the port to the supplied string
bool serial::read_buffer(string& buffer) {
	char data[1024];
	DWORD bytes_read;
	if (!ReadFile(h_port_, &data, sizeof(data), &bytes_read, nullptr)) {
		log_error("ReadFile");
		return false;
	}
	else {
		buffer = string(data, (size_t)bytes_read);
		return true;
	}
}
// Write data from the supplied string to the port
bool serial::write_buffer(string buffer) {
	DWORD bytes_written;
	if (!WriteFile(h_port_, buffer.c_str(), buffer.length(), &bytes_written, nullptr)) {
		log_error("WriteFile");
		return false;
	}
	return true;
}

void serial::log_error(string method) {
	DWORD error_code = GetLastError();
	char message[1028];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, message, sizeof(message), nullptr);
	cerr << method << " failed: " << message << '\n';
}

void serial::close_port() {
	CloseHandle(h_port_);
}

bool serial::available_ports(int num_ports, string* ports, bool all_ports, int& actual_ports) {
#ifdef _WIN32
	const unsigned int MAX_TTY = 255;
	actual_ports = 0;
	// Find which ports exists (not just available) by trying to open the port
	for (unsigned int i = 0; i < MAX_TTY; i++) {
		char dev[16];
		bool use_port = false;
		snprintf(dev, sizeof(dev), "//./COM%u", i);
		HANDLE fd = CreateFile(dev, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (fd != INVALID_HANDLE_VALUE) {
			// Accessed OK - so exists and available for use
			CloseHandle(fd);
			use_port = true;
		}
		else {
			// Check if the port was there but access denied - implies exists but in use
			long error_code = GetLastError();
			if (error_code == ERROR_ACCESS_DENIED && all_ports) {
				use_port = true;
			}
		}
		// Add it to the list of 
		if (use_port) {
			if (actual_ports < num_ports) {
				char port[16];
				snprintf(port, 16, "COM%d", i);
				ports[actual_ports] = port;
			}
			actual_ports++;
		}
	}
	return (actual_ports <= num_ports);
#else
	// Implement posix version
#endif

}