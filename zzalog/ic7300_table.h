#ifndef __MEMORY_TABLE__
#define __MEMORY_TABLE__

#include "view.h"
#include "fields.h"

#include <FL/Fl_Table_Row.H>

namespace zzalog {

	enum view_type {
		VT_MEMORIES,
		VT_SCOPE_BANDS,
		VT_USER_BANDS,
		VT_CW_MESSAGES,
		VT_RTTY_MESSAGES,
		VT_UNCHANGED
	};

	class ic7300_table : public Fl_Table_Row, public view
	{
	public:
		ic7300_table(int X, int Y, int W, int H, const char* label = nullptr, field_ordering_t order = FO_LAST);
		~ic7300_table();

		// inherited from view
		virtual void update(hint_t hint, unsigned int record_num_1, unsigned int record_num_2 = 0);
		// inherited from Fl_Table_Row
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);
		// inherited from Fl_Widget
		virtual int handle(int event);

		// Get/Set type
		void type(view_type t);
		view_type type();

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