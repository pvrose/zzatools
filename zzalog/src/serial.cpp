#include "serial.h"

#include <iostream>
#include <string>

#ifdef _WIN32
// Note the code is Windows only - need Linux version
#include <Windows.h>
#else 
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
#endif

// Constructor - initialise "file" handle to port
serial::serial() {
};

// Destrutor
serial::~serial() {
}

// Find all existing COM ports - upto COM255
// Returns true if the string array was large enough for all ports.
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
	// Implement Linux version
	return false;
#endif

}
