#ifndef __RECORD_VIEW__
#define __RECORD_VIEW__

#include "record_table.h"
#include "view.h"
#include "record.h"
#include "spec_data.h"
#include "fields.h"
#include "intl_widgets.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Output.H>	
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Menu_Item.H>

namespace zzalog {
	// This class is one of the views in the main window. It displays a table with all
	// the fields in the record, plus a selection of widgets that allow the capture and
	// editing of some of the more important fields. It also allows the user to 
	// select whether two records are equivalent and select which record's value of a 
	// a field should be used.
	class record_form :
		public Fl_Group,
		public view
	{
		// Use mode of the form
		enum use_mode_t {
			UM_DISPLAY,         // Displaying a record (i.e. before an edit is requested)
			UM_MODIFIED,        // Displaying a modified record
			UM_QSO,             // Interactive QSO (capturing a contact on-air)
			UM_QUERY,           // Querying an import (possible match and import record displayed)
			UM_DUPEQUERY,       // Querying a duplicate (the two records are displated)
			UM_MERGEDETAILS,    // Allowing user to merge additional data from another database
		};

		// What the source of the data in record is.
		enum edit_mode_t {
			EM_ORIGINAL,        // Original log record
			EM_EDIT,            // Supplied by editing
			EM_QUERY            // Supplied by query record
		};

		// The specific image required - determines the file location of the image file
		enum image_t {
			QI_EQSL,            // Downloaded eQSL image
			QI_CARD_FRONT,      // Scanned-in front of a card
			QI_CARD_BACK,       // Scanned-in back of a card
			QI_GEN_CARD         // Generated QSL card
		};


		// Constructors and destructor
	public:
		// Constructor initialises both the Fl_Group and view
		record_form(int X, int Y, int W, int H, const char* label, field_ordering_t app);
		~record_form();

		// Public methods
		// inherited from view
		virtual void update(hint_t hint, record_num_t record_num_1, record_num_t record_num_2 = 0);
		// Access to selected record (for use in callback)
		void set_selected_image(image_t image);
		// initialise view

		// call-back methods
	protected:
		// Card radio button
		static void cb_rad_card(Fl_Widget* w, void* v);
		// Table entry clicked
		static void cb_tab_record(Fl_Widget* w, void* v);
		// Field choice has been clicked
		static void cb_ch_field(Fl_Widget* w, void* v);
		// enum choice has been changed
		static void cb_ch_enum(Fl_Widget* w, void* v);
		// use button has been clicked
		static void cb_bn_use(Fl_Widget* w, void* v);
		// Clear button has been clicked
		static void cb_bn_clear(Fl_Widget* w, void* v);
		// reject/merge button has been clicked
		static void cb_bn_edit(Fl_Widget* w, long v);
		// quick button has been clicked
		static void cb_bn_quick(Fl_Widget* w, void* v);
		// Modify button has been clicked
		static void cb_bn_modify(Fl_Widget* w, void* v);
		// Fetch eQSL button has been clicked
		static void cb_bn_fetch(Fl_Widget* w, void* v);
		// Stretcch/Scale button has been clicked
		static void cb_bn_scale(Fl_Widget*, void* v);
		// Log Card button has been clicked
		static void cb_bn_log_card(Fl_Widget* w, void* v);
		// find possible matches
		static void cb_bn_find(Fl_Widget* w, void* v);
		// get next and update
		static void cb_bn_next(Fl_Widget* w, void* v);
		// get previous and update
		static void cb_bn_prev(Fl_Widget* w, void* v);
		// all or limited fields
		static void cb_bn_all_fields(Fl_Widget* w, void* v);

		// methods
		// update the form from the provided data
		void update_form();
		// set the image data
		void set_image();
		// set the image radio buttons
		void set_image_buttons();
		// draw the image
		void draw_image();
		// enable the widgets according to current use mode
		void enable_widgets();
		// set the editting widgets
		void set_edit_widgets(string field, string text);
		// set enumeration value
		void set_enum_choice(string enumeration_type, string text);
		// explain the enumeration value
		void explain_enum(spec_dataset* dataset, string enumeration_value);

	protected:
		// widgets
		// Card image and info
		Fl_Group * card_display_;
		Fl_Box* card_filename_out_;
		Fl_Light_Button* keep_bn_;
		// Card controls
		Fl_Group* card_type_grp_;
		Fl_Radio_Round_Button* eqsl_radio_;
		Fl_Radio_Round_Button* card_front_radio_;
		Fl_Radio_Round_Button* card_back_radio_;
		Fl_Radio_Round_Button* gen_card_radio_;
		Fl_Button* fetch_bn_;
		Fl_Button* log_card_bn_;
		Fl_Light_Button* scale_bn_;
		// Record table and ifo
		record_table* record_table_;
		Fl_Box* question_out_;
		// QSL messages
		Fl_Group* message_grp_;
		intl_input* qsl_message_in_;
		intl_input* swl_message_in_;
		Fl_Button* modify_message_bn_;
		// Editing info 
		Fl_Group* editting_grp_;
		Fl_Check_Button* all_fields_bn_;
		Fl_Choice* field_choice_;
		intl_editor* value_in_;
		Fl_Choice* enum_choice_;
		// Quick entry buttons
		Fl_Group* quick_grp_;
		Fl_Button* call_bn_;
		Fl_Button* name_bn_;
		Fl_Button* qth_bn_;
		Fl_Button* rst_rcvd_bn_;
		Fl_Button* grid_bn_;
		Fl_Button* freq_bn_;
		Fl_Button* mode_bn_;
		Fl_Button* power_bn_;
		Fl_Button* rst_sent_bn_;
		// Editting controls
		Fl_Button* use_bn_;
		Fl_Button* clear_bn_;
		Fl_Button* edit1_bn_;
		Fl_Button* edit2_bn_;
		Fl_Button* edit3_bn_;
		Fl_Button* edit4_bn_;
		// query controls
		Fl_Button* find_bn_;
		Fl_Button* previous_bn_;
		Fl_Button* next_bn_;

		// attributes
		// the first record displayed
		record* record_1_;
		// the second image displayed
		record* record_2_;
		// saved record
		record* saved_record_;
		// the first record number
		int item_num_1_;
		// the second record number
		int item_num_2_;
		// the use mode of the form
		use_mode_t use_mode_;
		// Manually modifying a field
		bool modifying_;
		// the message to display for a merge, dupe check or validation query
		string query_message_;
		// the field being edited is an enumeration
		bool is_enumeration_;
		// current field name
		string current_enum_type_;
		// the source of the displayed image
		image_t selected_image_;
		// the image file
		Fl_Image* image_;
		// Scale or stretch the image
		bool scaling_image_;
		// What the current edit value of a field is:
		edit_mode_t edit_mode_;
		// Current field
		string current_field_;
		// Display all fields
		bool display_all_fields_;
	};

}
#endif
