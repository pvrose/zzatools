#ifndef __FIELD_CHOICE__
#define __FIELD_CHOICE__

#include "spec_data.h"
#include "intl_widgets.h"
#include "record.h"

#include <string>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>

using namespace std;



	// This class provides an extension to Fl_Choice to be used for an ADIF field selection choice.
	class field_choice : public Fl_Choice
	{
	public:
		field_choice(int X, int Y, int W, int H, const char* label = nullptr);
		~field_choice();

		// Set dataset
		void set_dataset(string dataset_name, string field = "");
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


	// This class provides an extension to Fl_Input_Choice to add the menu
	// if the field is an enumeration
	class field_input : public Fl_Input_Choice
	{
	public:
		field_input(int X, int Y, int W, int H, const char* label = nullptr);
		~field_input();

		// Overload handle to set this as a recipient of int'l pastes
		int handle(int event);
		// The value() methods of Fl_Input_Choice are not virtual, so need to make them 
		// all virtual here otherwise the compiler cannot find them
		// Overload value
		virtual const char* value();
		// Make the others virtual as well
		virtual void value(const char* val);
		virtual void value(int i);
	
		void field_name(const char* field_name, record* qso = nullptr);
		const char* field_name();

		// Get reason for leaving 
		enum exit_reason_t {
			IR_NULL,       // Normal behaviour - enter or lose focus
			IR_RIGHT,      // Tab right
			IR_LEFT,       // Tab left (Shift-tab)
			IR_UP,         // Up key pressed
			IR_DOWN,       // Down key pressed
			IR_ENTER,    // Do not do anything
		};
		exit_reason_t reason();

		void reload_choice(record* qso = nullptr);

		virtual void type(uchar t);

	protected:

		// Populate the choice
		void populate_choice(string name);
		// Populate a case choice
		void populate_case_choice();
		// Has a field that can have case changed
		bool is_string(string field);
		// Name of field
		string field_name_;
		// Get reason for leaving -1 - LEFT TAB, +1 - RIGHT TAB, 0 any other
		exit_reason_t reason_;
	};
#endif

