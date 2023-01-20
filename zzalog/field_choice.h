#ifndef __FIELD_CHOICE__
#define __FIELD_CHOICE__

#include "spec_data.h"

#include <string>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>

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
		// Set hierarchic
		void hierarchic(bool h);

	protected:
		spec_dataset* dataset_;
		// Drop down list is hierarchic
		bool hierarchic_;

	};

	// This class is a combo between Fl_Input and field_choice (one or the other)
	class field_input : public Fl_Group
	{
	public:
		field_input(int X, int Y, int W, int H, const char* label = nullptr);
		~field_input();

		void field_name(const char* field_name);
		const char* field_name();

		// Overloaded value 
		const char* value();
		void value(const char* v);

		// Reload choice
		void reload_choice();

	protected:
		// The two callbacks
		static void cb_ip(Fl_Widget* w, void* v);
		static void cb_ch(Fl_Widget* w, void* v);

		// Activate the appropriate widget
		Fl_Widget* show_widget();
		// Populate the choice
		void populate_choice(string name);
		// Process the field
		Fl_Input* ip_;
		field_choice* ch_;
		// Name of field
		string field_name_;
		// VAlue
		const char* value_;
		// Specific choice menu
		string menu_data_;
	};

}
#endif

