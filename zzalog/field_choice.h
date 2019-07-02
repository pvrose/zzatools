#ifndef __FIELD_CHOICE__
#define __FIELD_CHOICE__

#include <string>
#include <FL/Fl_Choice.H>

using namespace std;

namespace zzalog {

	class field_choice : public Fl_Choice
	{
	public:
		field_choice(int X, int Y, int W, int H, const char* label = nullptr);
		~field_choice();

		void repopulate(bool all_fields, string default_field);
	};

}
#endif

