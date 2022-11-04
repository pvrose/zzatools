#ifndef __FIELD_CHOICE__
#define __FIELD_CHOICE__

#include "spec_data.h"

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

		// Set dataset
		void set_dataset(string dataset_name, string default = "");
		// Override value methods for fundamental string
		const char* value();
		void value(const char* field);

	protected:
		spec_dataset* dataset_;
		// Drop down list is hierarchic
		bool hierarchic_;

	};

}
#endif

