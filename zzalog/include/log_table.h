#ifndef __LOG_TABLE__
#define __LOG_TABLE__


#include "view.h"
#include "fields.h"
//#include "edit_input.h"
#include "field_choice.h"

#include <string>
#include <vector>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Button.H>

using namespace std;

class book;
class field_choice;


	// This class implements a tabular view of a book. It inherits the tabular view from Fl_Table_Row
	// and the necessary book view features from view
	class log_table : public Fl_Table_Row, public view

	{
		// Constructor and Destructor
	public:
		log_table(int X, int Y, int W, int H, const char* label, field_app_t app);
		virtual ~log_table();

		// Public methods
	public:
		// inherited from view
		virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0);
		// inherited from Fl_Table_Row
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);
		// inherited from Fl_Widget
		virtual int handle(int event);

		// Get the edit_input to pass save events
		void edit_save(field_input::exit_reason_t exit_type);
		// Set font and fontsize
		static void set_font(Fl_Font font, Fl_Fontsize size);

		// Returns the name and attributes of the fields currently being displayed
		collection_t& fields();

		// Protected methods
	protected:
		// Call back to handle click on the table
		static void cb_tab_log(Fl_Widget* w, void* v);
		// Call back to handle edit input
		static void cb_input(Fl_Widget* w, void* v);

		// Read fields
		void get_fields();
		// Open an editor and change fields
		void edit_cell(int R, int C);
		// Finish edit
		void done_edit(bool keep_row);
		// Open a tooltip to describe the cells content
		void describe_cell(int item, int C);
		// Handle a double click on the column header
		void dbl_click_column(int C);
		// Handle a drag on the column header
		void drag_column(int C);
		// Select and go to item
		void display_current();
		// Adjust row height and header width to fit font
		void adjust_row_sizes();

		// Protected attributes:
	protected:
		// Column definition
		collection_t* log_fields_;
		// current selected item
		unsigned int current_item_num_;
		// last event
		int last_event_;
		// Mouse Button clicked for last event
		int last_button_;
		// Was it multiple clicks?
		int last_clicks_;
		// The last X position (wrt screen)
		int last_rootx_;
		// The last Y position (wrt screen)
		int last_rooty_;
		// Alt GR is currently pressed
		bool alt_gr_;
		// field aplication
		field_app_t application_;
		// Rows per page
		int rows_per_page_;
		// Inverse display order
		enum {FIRST_TO_LAST, LAST_TO_FIRST, SORTED_UP, SORTED_DOWN} order_;
		// Field on which sort was done
		string sorted_field_;
		// The edit input
		//edit_input* edit_input_;
		field_input* edit_input_;
		// Edit row
		unsigned int edit_row_;
		unsigned int edit_col_;
		// Table font and size
		static Fl_Font font_;
		static Fl_Fontsize fontsize_;
		// Tooltip window for explaining field contents
		Fl_Window* tip_window_;
		// Tooltip position
		int tip_root_x_;
		int tip_root_y_;
	};
#endif
