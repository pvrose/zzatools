#ifndef __SERIAL__
#define __SERIAL__


#include <set>
#include <string>

using namespace std;



	// This class provides read/write access to a COM serial port
	class serial
	{
	public:
		serial();
		~serial();

		// Provide a set of all available ports
		bool available_ports(int num_ports, string* ports, bool all_ports, int& actual_ports);
	};


#endif
