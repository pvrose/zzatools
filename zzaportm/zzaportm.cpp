// zzaportm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "serial.h"

#include <iostream>
#include <string>

using namespace std;
using namespace zzaportm;

// The device to open
string port_name;
// The baud-rate
int baud_rate;

void parse_args(int argc, char** argv) {
	for (int i = 0; i < argc; i++) {
		if (strncmp(argv[i], "port=", 5) == 0) {
			port_name = string(argv[i] + 5, strlen(argv[i] - 5));
			cout << "Port: " << port_name << '\n';
		}
		else if (strncmp(argv[i], "baud=", 5) == 0) {
			baud_rate = atoi(argv[i] + 5);
			cout << "Baud: " << baud_rate << '\n';
		}
		else {
			cerr << "Unexpected parameter " << i << ": " << argv[i] << '\n';
		}
	}
}

void display_data(string line) {
	for (size_t i = 0; i < line.length(); i++) {
		char hex[4];
		snprintf(hex, 4, "%0x ", line[i]);
		cout << hex;
	}
	cout << '\n';
}

int main(int argc, char** argv)
{
	parse_args(argc, argv); 
	
	serial port;

	if (port.open_port(port_name, baud_rate, true)) {
		string buffer;
		while (port.read_buffer(buffer)) {
			display_data(buffer);
		}
	}
	else while (1);

	port.close_port();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
