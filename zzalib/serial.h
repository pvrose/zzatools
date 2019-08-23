#ifndef __SERIAL__
#define __SERIAL__

#include <set>
#include <string>

#include <Windows.h>
#include <fileapi.h>

using namespace std;

class serial
{
public:
	serial();
	~serial();
		
	// Provide a set of all available ports
	bool available_ports(int num_ports, string* ports, bool all_ports, int& actual_ports);
	// Open port named device with specified baud-rate and read-only or read-write
	bool open_port(string device, unsigned int baud_rate, bool monitor);
	// Copy data from the port to the supplied string
	bool read_buffer(string& buffer);
	// Write data from the supplied string to the port
	bool write_buffer(string buffer);
	// Close the port
	void close_port();

protected:
	void log_error(string method);
	// handle to the comms port
	HANDLE h_port_;
	// name of the comms port
	string port_name_;

};

#endif
