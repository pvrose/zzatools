#ifndef __VIEW73__
#define __VIEW73__

#include <string>

#include <FL/Fl_Table_Row.H>

using namespace std;

namespace zza7300 {
	enum view_type {
		VT_MEMORIES,
		VT_SCOPE_BANDS,
		VT_USER_BANDS,
		VT_CW_MESSAGES,
		VT_RTTY_MESSAGES,
		VT_UNCHANGED
	};

	class view73 : public Fl_Table_Row
	{
	public:
		view73(int X, int Y, int W, int H, const char* label = nullptr);
		~view73();
		
	public:
		// Get/Set type
		void type(view_type t);
		view_type type();

		// inherited from Fl_Table_Row
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);

	protected:
		// Open one of the views
		void open_view(view_type t);
		void draw_memory_view();
		void draw_scope_bands_view();
		void draw_user_bands_view();
		void draw_message_view(bool cw);
		// Resize columns
		void resize_cols();
		// Delete the current items
		void delete_items();
		// Convert int to BCD
		string int_to_bcd(int value, int size, bool least_first);
		// Convert BCD to int
		int bcd_to_int(string, bool least_first);
		// Convert BCD tp Float
		double bcd_to_double(string, int decimals, bool least_first);
		// Convert string to hex
		string string_to_hex(string, bool escape = false);
		// Convert string to hex
		string hex_to_string(string);
		// Send command
		string send_command(unsigned char command, string sub_command, bool& ok);
		string send_command(unsigned char command, string sub_command, string data, bool& ok);
		// Define structure to use with each view
		// The specific view type
		view_type type_;
		// Items to display
		string** items_;
		// Headers
		string* headers_;
		// Row headers
		string* row_headers_;
		// Column widths
		int* col_widths_;
		// Items valid
		bool items_valid_;
	};

}

#endif

