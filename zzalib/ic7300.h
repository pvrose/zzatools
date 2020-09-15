#ifndef __IC7300__
#define __IC7300__

#include "utils.h"

#include <string>

using namespace std;

// Class to send commands to an IC7300 (and other Icom rigs)

namespace zzalib {
	class ic7300
	{
	public:
		ic7300();
		~ic7300();
		// Send command
		string send_command(unsigned char command, string sub_command, bool& ok);
		string send_command(unsigned char command, string sub_command, string data, bool& ok);
		void callback(void(*mess_func)(status_t, const char*));

	protected:
		// Flag to prevent multiple warnings
		bool given_warning_;
		void (*message)(status_t, const char*);

	};

}
#endif

