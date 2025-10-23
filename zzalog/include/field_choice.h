#ifndef __FIELD_CHOICE__
#define __FIELD_CHOICE__

#include <string>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input_Choice.H>



class record;
struct spec_dataset;



	//! This class provides an extension to Fl_Choice to be used for an ADIF field selection choice.
	
	//! If there are a large number of possible values, the std::list is broken into a hierarchy.
	class field_choice : public Fl_Choice
	{
	public:
		//! Constructor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		field_choice(int X, int Y, int W, int H, const char* L = nullptr);
		//! Destructor.
		~field_choice();

		//! Get the data for \p dataset_name from spec_data and std::set the \p default_value choice value.
		void set_dataset(std::string dataset_name, std::string default_value = "");
		//! Get the selected value, removing any hierarchy.
		const char* value();
		//! Set the value.
		void value(const char* field);
		//! Set hierarchic std::list of values.
		void hierarchic(bool h);

	protected:
		//! Pointer to the dataet inside spec_data containing the std::list of fields 
		spec_dataset* dataset_;
		//! Drop down std::list is hierarchic
		bool hierarchic_;

	};


	//! \brief This class provides an extension to Fl_Input_Choice to add the menu
	//! if the field is an enumeration.
	class field_input : public Fl_Input_Choice
	{
	public:
		//! Constructor
		
		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		field_input(int X, int Y, int W, int H, const char* L = nullptr);
		//! Destructor.
		~field_input();

		//! Overload to handle selective mosue and keyboard events.
		
		//! - Left button push sets the input the focus of intl_dialog pastes.
		//! - Right button release opens a tip window.
		//! - Navigation keys have specific actions.
		int handle(int event);
		//! Get the input or selected value.
		virtual const char* value();
		//! \cond
		// The value() methods of Fl_Input_Choice are not virtual, so need to make them 
		// all virtual here otherwise the compiler cannot find them.
		virtual void value(const char* val);
		virtual void value(int i);
		//! \endcond
		
		//! \brief Set the field supported by the input to \p field_name, setting its
		//! initial value to that in \p qso.
		void field_name(const char* field_name, record* qso = nullptr);
		//! The field name used to populate the selection.
		const char* field_name();

		//! Set the QSO record to \p q.
		void qso(record* q);

		//! Certain keystrokes can be passed to parent widget.
		enum exit_reason_t {
			IR_NULL,       //!< Normal behaviour - enter or lose focus
			IR_RIGHT,      //!< Tab right
			IR_LEFT,       //!< Tab left (Shift-tab)
			IR_UP,         //!< Up key pressed
			IR_DOWN,       //!< Down key pressed
			IR_ENTER,      //!< Do not do anything
		};
		//! Return reason for closing widget.
		exit_reason_t reason();

		//! Reloasds the choice values and possibly select that in the given \p QSO.
		void reload_choice(record* qso = nullptr);
		//! Allow the Fl_Input component to be either INPUT or OUTPUT
		virtual void type(uchar t);
		//! Overload draw - deactivates menubutton if type is OUTPUT.
		virtual void draw();

		//! Callback for menu changes.
		static void cb_menu(Fl_Widget* w, void* v);

	protected:

		//! Populate the choice from field named \p name.
		void populate_choice(std::string name);
		//! Populate a case choice - UPPER, lower and Mixed versions of the text value.
		void populate_case_choice();
		//! Is it a field that can have its value's case changed
		bool is_string(std::string field);
		//! Name of field
		std::string field_name_;
		//! QSO
		record* qso_;
		//! Get reason for leaving
		exit_reason_t reason_;
		//! activate the menubutton
		bool use_menubutton_;
		//! Tip window to be shown on right button click.
		static Fl_Window* tip_window_;
	};
#endif

