#ifndef __IC7300__
#define __IC7300__

#include <string>

using namespace std;

namespace zzalib {
	class ic7300
	{
	public:
		ic7300();
		~ic7300();
		// Send command
		string send_command(unsigned char command, string sub_command, bool& ok);
		string send_command(unsigned char command, string sub_command, string data, bool& ok);

	protected:
		bool given_warning;

	};

}
#endif

