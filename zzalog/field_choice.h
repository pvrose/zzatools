#ifndef __FIELD_CHOICE__
#define __FIELD_CHOICE__

#include <string>
#include <FL/Fl_Choice.H>

using namespace std;

namespace zzalog {

	// This class provides an extension to Fl_Choice to be used for an ADIF field selection choice.
	class field_choice : public Fl_Choice
	{
	public:
		field_choice(int X, int Y, int W, int H, const char* label = nullptr);
		~field_choice();
		// Repopulate the choice with either all fields or the most common ones and select the default feld.
		void repopulate(bool all_fields, string default_field);
	};

}
#endif

