#ifndef __SERIAL__
#define __SERIAL__


#include <set>
#include <string>





	//! This class provides utilities to support serial port access.
	class serial
	{
	public:
		//! Constructor.
		serial();
		//! Destructor. 
		~serial();

		//! Provides a std::set of all available ports
		
		//! \param num_ports the size of array \p ports.
		//! \param ports An array of stringsto receive the port names.
		//! \param all_ports Provide all ports even if they are not available for use.
		//! \param actual_ports Receives the number of ports in the std::list.
		//! \return true if the array \p ports was big eneough.
		bool available_ports(int num_ports, std::string* ports, bool all_ports, int& actual_ports);
	};


#endif
