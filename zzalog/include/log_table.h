#ifndef __LOG_TABLE__
#define __LOG_TABLE__


#include "view.h"
#include "fields.h"
//#include "edit_input.h"
#include "field_choice.h"

#include <string>
#include <vector>
#include <FL/Fl_Table_Row.H>
#include <FL/Enumerations.H>



class book;
class field_choice;
class Fl_Window;


	//! This class implements a tabular view of a book. 
	
	//! It inherits the tabular view from Fl_Table_Row
	//! and the necessary book view features from view
	class log_table : public Fl_Table_Row, public view

	{
	public:
		//! Constructor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param label label
		//! \param app field usage mode.
		log_table(int X, int Y, int W, int H, const char* label, field_app_t app);
		//! Destructor.
		virtual ~log_table();

		// Public methods
	public:
		//! Inherited from view
		
		//! \param hint indicates how the logbook has been changed to control redrawing.
		//! \param record_num_1 the index in full log of the record that has changed.
		//! \param record_num_2 the index in full log of an ancilliary record.
		virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0);
		//! Inherited from Fl_Table_Row - draws individual components of the table taking text
		//! from records in the log book.
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);
		//! Inherited from Fl_Table_Row to handle various mavigation and focus events.
		virtual int handle(int event);

		//! Process changes from the edit_input_.
		void edit_save(field_input::exit_reason_t exit_type);
		//! Set font and fontsize
		static void set_font(Fl_Font font, Fl_Fontsize size);

		//! Returns the name and attributes of the fields currently being displayed
		collection_t& fields();

		//! Display order.
		enum sort_order : uchar {
			FIRST_TO_LAST,               //!< Chronological order.
			LAST_TO_FIRST,               //!< Reverse chronological.
			SORTED_UP,                   //!< Sorted alphabetically on a field.
			SORTED_DOWN                  //!< Reverse alphabetical on a field.
		};

		// Protected methods
	protected:
		//! Callback to handle click within the table: opening edit_input_ and handling column changes.
		static void cb_tab_log(Fl_Widget* w, void* v);
		//! Callback to handle changes in edit_input_.
		static void cb_input(Fl_Widget* w, void* v);

		//! Read fields from the fields database.
		void get_fields();
		//! Open edit_input_ over row \p R, column \p C.
		void edit_cell(int R, int C);
		//! Finish edit, remembering the row that was edited if \p keep_row is true.
		void done_edit(bool keep_row);
		//! Open a tooltip to describe the cell at row \p item, column \p C.
		void describe_cell(int item, int C);
		//! Handle a double click on the column \p C header.
		void dbl_click_column(int C);
		//! Handle a drag on the column header \p C.
		void drag_column(int C);
		//! Select and position the current record within the display.
		void display_current();
		//! Adjust row height and header width to fit font and window size.
		void adjust_row_sizes();


		// Protected attributes:
	protected:
		//! Column definition
		collection_t* log_fields_;
		//! Current selected item (index within the version of the log displayed).
		unsigned int current_item_num_;
		//! Last event.
		int last_event_;
		//! Mouse Button clicked for last event.
		int last_button_;
		//! Was it multiple clicks?
		int last_clicks_;
		//! The last X position (wrt screen).
		int last_rootx_;
		//! The last Y position (wrt screen).
		int last_rooty_;
		//! Alt GR is currently pressed
		bool alt_gr_;
		//! field usage.
		field_app_t application_;
		//! Rows per page.
		int rows_per_page_;
		//! How the records are ordered.
		sort_order order_;   
		//! Field on which sort was done.
		std::string sorted_field_;
		//! The edit input.
		field_input* edit_input_;
		//! Edited row number.
		int edit_row_;
		//! Edited column number.
		unsigned int edit_col_;
		//! Font used.
		static Fl_Font font_;
		//! Font size used.
		static Fl_Fontsize fontsize_;
		//! Tooltip window for explaining field contents.
		Fl_Window* tip_window_;
		//! Tooltip X position (on screen).
		int tip_root_x_;
		//! Tooltip Y position (on screen).
		int tip_root_y_;
	};

	//! JSON serialisation for log_table::sort_order
	NLOHMANN_JSON_SERIALIZE_ENUM(log_table::sort_order, {
		{ log_table::FIRST_TO_LAST, "Chronological" },
		{ log_table::LAST_TO_FIRST, "Reverse Chronological" },
		{ log_table::SORTED_UP, "Alphabetical" },
		{ log_table::SORTED_DOWN, "Reverse Alphabetical" }
		}
	)


#endif
