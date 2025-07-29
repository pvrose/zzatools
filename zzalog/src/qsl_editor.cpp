#include "qsl_editor.h"
#include "qsl_dataset.h"
#include "qsl_display.h"
#include "qsl_widget.h"
#include "status.h"
#include "qso_manager.h"
#include "qso_data.h"
#include "config.h"
#include "utils.h"
#include "field_choice.h"
#include "intl_widgets.h"
#include "font_dialog.h"
#include "record.h"
#include "filename_input.h"

#include <string>
#include <ctime>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Output.H>
#include <Fl/Fl_Check_Button.H>

using namespace std;

extern qso_manager* qso_manager_;
extern qsl_dataset* qsl_dataset_;
extern status* status_;
extern string VENDOR;
extern string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;

// Constructor
qsl_editor::qsl_editor(int X, int Y, int W, int H, const char* L) :
    page_dialog(X, Y, W, H, L),
	show_example_(false),
	load_tsv_(false)
{
	// Default callback
	callback(cb_bn_ok);
	// Load settings
    load_values();
	// Create the qsl_display instance
	create_display();
	qsl_ = display_->display();
	// Load the display settings for this callsign
	data_ = qsl_dataset_->get_card(callsign_, qsl_type_, true);
	// If data does not exist 
	// Set data into QSL Display
	qsl_->set_card(data_);

	// Only show the dipslay window if this dialog is active
	w_display_->hide();
	// Add all the widgets in this dialog
    create_form(X, Y);

	if (active()) redraw_display(false);

}

// Destructor
qsl_editor::~qsl_editor() {
	// delete the display window
	Fl::delete_widget(w_display_);
}

// Overload of handle() to intercept ACTIVATE and DEACTIVATE events
int qsl_editor::handle(int event) {
	switch(event) {
	case FL_DEACTIVATE:
	case FL_HIDE: {
		// This is being deactivated, so hide the display window
		if (display_) w_display_->hide();
		break;
	}
	case FL_ACTIVATE: 
	case FL_SHOW: {
		// This is being activated so show the display window
		if (display_) w_display_->show();
		create_items();
		break;
	}
	}
	// Pass all events to page_dialog
	return page_dialog::handle(event);
}

// Load the settings
void qsl_editor::load_values() {
	// Get the current or default callsign to use initially
	if (qso_manager_) {
		callsign_ = qso_manager_->get_default(qso_manager::CALLSIGN);
	}
	qsl_type_ = qsl_data::LABEL;
	// Get the display window position
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences windows_settings(settings, "Windows");
	Fl_Preferences qsl_win_settings(windows_settings, "QSL Design");
	qsl_win_settings.get("Top", win_y_, 10);
	qsl_win_settings.get("Left", win_x_, 10);

}

// Instantiate all widgets
void qsl_editor::create_form(int X, int Y) {
	
    int curr_x = X + GAP;
    int curr_y = Y + GAP;
    int max_x;

	// Group 1: Template file
    g_1_ = new Fl_Group(curr_x, curr_y, 100, 100, "Template File");
    g_1_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    g_1_->labelfont(FL_BOLD);
    g_1_->labelsize(FL_NORMAL_SIZE + 2);
    g_1_->box(FL_BORDER_BOX);

	curr_y += HTEXT;

	// Input choice to select callsign to use
    field_input* w101 = new field_input(curr_x + WLLABEL, curr_y, WSMEDIT, HBUTTON, "Callsign for QSL");
    w101->value(callsign_.c_str());
    w101->callback(cb_callsign, &callsign_);
    w101->field_name("STATION_CALLSIGN");
	w101->tooltip("Select the callsign to change QSL template parameters for");
	ip_callsign_ = w101;

	curr_x += w101->x() + w101->w() + GAP + WLABEL;
	Fl_Choice* w10101 = new Fl_Choice(curr_x, curr_y, WBUTTON, HBUTTON, "QSL Type");
	w10101->align(FL_ALIGN_LEFT);
	w10101->callback(cb_qsl_type, &qsl_type_);
	populate_qsl_type(w10101);
	w10101->value((int)qsl_type_);
	w10101->tooltip("Select the type of QSL display you want, eg for a label for printing or a file for e-mailing");

	curr_x += w10101->w() + GAP;
	Fl_Check_Button* w10102 = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Load TSV");
	w10102->align(FL_ALIGN_RIGHT);
	w10102->callback(cb_bn_loadtsv, &load_tsv_);
	w10102->value(load_tsv_);
	w10102->tooltip("Select to enable loading from TSV file");
	bn_loadtsv_ = w10102;

	max_x = w10102->x() + w10102->w() + WLABEL + GAP;
    curr_y += HBUTTON;
	curr_x = g_1_->x() + GAP;

	// Input field to specify filename
    filename_input* w102 = new filename_input(curr_x, curr_y, WEDIT * 2, HBUTTON);
    w102->callback(cb_filename, &data_->filename);
    w102->when(FL_WHEN_CHANGED);
    w102->value(data_->filename.c_str());
    w102->tooltip("Please enter the QSL template");
	w102->type(filename_input::FILE);
	w102->title("Please select QSL Template");
	w102->pattern("TSV\t*.tsv" );
	ip_filename_ = w102;

    curr_x += w102->w() + GAP;
	curr_y += w102->h() + GAP;

	max_x = max(max_x, curr_x);

    g_1_->resizable(nullptr);
    g_1_->size(max_x - g_1_->x(), curr_y - g_1_->y());
    g_1_->end();

    max_x = g_1_->x() + g_1_->w();

    curr_x = x() + GAP;

	// Group 2: Size and Format parameters
    g_2_ = new Fl_Group(curr_x, curr_y, 100, 100, "Size & format parameters");
    g_2_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    g_2_->labelfont(FL_BOLD);
    g_2_->labelsize(FL_NORMAL_SIZE + 2);
    g_2_->box(FL_BORDER_BOX);

	curr_y += HTEXT;

	curr_x += GAP;
	// Group 2.1 - radio buttons for units
	Fl_Group* g_201 = new Fl_Group(curr_x, curr_y, WBUTTON, HBUTTON * 3);
	g_201->box(FL_FLAT_BOX);
	g_dim_ = g_201;
	// Radio to select inches
	Fl_Radio_Round_Button* w_20101 = new Fl_Radio_Round_Button(curr_x, curr_y, HBUTTON, HBUTTON, "inch");
	w_20101->value(data_->unit == qsl_data::INCH);
	w_20101->selection_color(FL_RED);
	w_20101->align(FL_ALIGN_RIGHT);
	w_20101->callback(cb_radio_dim, (void*)qsl_data::INCH);
	w_20101->tooltip("Measurements in inches");
	bn_inch_ = w_20101;
    curr_y += HBUTTON;
	// Radio to select millimetres
	Fl_Radio_Round_Button* w_20102 = new Fl_Radio_Round_Button(curr_x, curr_y, HBUTTON, HBUTTON, "mm");
	w_20102->value(data_->unit == qsl_data::MILLIMETER);
	w_20102->selection_color(FL_RED);
	w_20102->align(FL_ALIGN_RIGHT);
	w_20102->callback(cb_radio_dim, (void*)qsl_data::MILLIMETER);
	w_20102->tooltip("Measurements in millimetres");
	bn_mm_ = w_20102;
	// Radio to select points
    curr_y += HBUTTON;
	Fl_Radio_Round_Button* w_20103 = new Fl_Radio_Round_Button(curr_x, curr_y, HBUTTON, HBUTTON, "point");
	w_20103->value(data_->unit == qsl_data::POINT);
	w_20103->selection_color(FL_RED);
	w_20103->align(FL_ALIGN_RIGHT);
	w_20103->callback(cb_radio_dim, (void*)qsl_data::POINT);
	w_20103->tooltip("Measurements in printer points");
	bn_point_ = w_20103;
	g_201->end();

    curr_x += g_201->w();
    curr_y = g_201->y();
	// Group 2.2 - All the size buttons
    Fl_Group* g_202 = new Fl_Group(curr_x, curr_y, 4*(WBUTTON + WLABEL), 3*HBUTTON + HTEXT + GAP);
    g_202->box(FL_FLAT_BOX);

    curr_x += WLABEL;
    // Input to set number of columns to print
	Fl_Value_Input* w_20201 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Columns");
	w_20201->value(data_->columns);
	w_20201->align(FL_ALIGN_LEFT);
	w_20201->callback(cb_value<Fl_Value_Input, int>, &data_->columns);
	w_20201->when(FL_WHEN_CHANGED);
	w_20201->tooltip("Please specify the number of columns when printing");
	ip_cols_ = w_20201;
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the width of the label
	Fl_Value_Input* w_20202 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Width");
	w_20202->value(data_->width);
	w_20202->align(FL_ALIGN_LEFT);
	w_20202->callback(cb_size_double, &data_->width);
	w_20202->when(FL_WHEN_CHANGED);
	w_20202->tooltip("Please specify the width of the label in the selected unit");
	ip_width_ = w_20202;
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the position ofthe left edge of culmn 1
	Fl_Value_Input* w_20203 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Position");
	w_20203->value(data_->col_left);
	w_20203->align(FL_ALIGN_LEFT);
	w_20203->callback(cb_value<Fl_Value_Input, double>, &data_->col_left);
	w_20203->when(FL_WHEN_CHANGED);
	w_20203->tooltip("Please specify the position of the first column");
	ip_cpos_ = w_20203;
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the spacing between columns (between left edges)
	Fl_Value_Input* w_20204 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Spacing");
	w_20204->value(data_->col_width);
	w_20204->align(FL_ALIGN_LEFT);
	w_20204->callback(cb_value<Fl_Value_Input, double>, &data_->col_width);
	w_20204->when(FL_WHEN_CHANGED);
	w_20204->tooltip("Please specify the spacing between columns");
	ip_cspace_ = w_20204;
    curr_x = g_202->x();
    curr_x += WLABEL;
    curr_y += HBUTTON;
	// Row 2A - height parameters
	// Input to specify the number of rows of labels to print
	Fl_Value_Input* w_20205 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Rows");
	w_20205->value(data_->rows);
	w_20205->align(FL_ALIGN_LEFT);
	w_20205->callback(cb_value<Fl_Value_Input, int>, &data_->rows);
	w_20205->when(FL_WHEN_CHANGED);
	w_20205->tooltip("Please specify the number of rows when printing");
	ip_rows_ = w_20205;
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the height of each label
	Fl_Value_Input* w_20206 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Height");
	w_20206->value(data_->height);
	w_20206->align(FL_ALIGN_LEFT);
	w_20206->callback(cb_size_double, &data_->height);
	w_20206->when(FL_WHEN_CHANGED);
	w_20206->tooltip("Please specify the width of the label in the selected label");
	ip_height_ = w_20206;
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the top of the first label
	Fl_Value_Input* w_20207 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Position");
	w_20207->value(data_->row_top);
	w_20207->align(FL_ALIGN_LEFT);
	w_20207->callback(cb_value<Fl_Value_Input, double>, &data_->row_top);
	w_20207->when(FL_WHEN_CHANGED);
	w_20207->tooltip("Please specify the position of the first row");
	ip_rpos_ = w_20207;
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the gap between each row
	Fl_Value_Input* w_20208 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Spacing");
	w_20208->value(data_->row_height);
	w_20208->align(FL_ALIGN_LEFT);
	w_20208->callback(cb_value<Fl_Value_Input, double>, &data_->row_height);
	w_20208->when(FL_WHEN_CHANGED);
	w_20208->tooltip("Please specify the spacing between rows");
	ip_rspace_ = w_20208;
    curr_x = g_202->x();
	curr_x += WLABEL;
    curr_y += HBUTTON;
	// Input to specify the (max) number of QSOs per QSL
	Fl_Value_Input* w_20209 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "QSOs/Card");
	w_20209->value(data_->max_qsos);
	w_20209->align(FL_ALIGN_LEFT);
	w_20209->callback(cb_value<Fl_Value_Input, int>, &data_->max_qsos);
	w_20209->when(FL_WHEN_CHANGED);
	w_20209->tooltip("Please specify the number of QSOs per Card");
	ip_qsos_ = w_20209;
	// Output to display size in points
	curr_x += WBUTTON + WLABEL;
	Fl_Output* w_20210 = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Size in pt.");
	w_20210->align(FL_ALIGN_LEFT);
	w_20210->tooltip("Displays the size of the label in points");
	op_size_ = w_20210;
	update_size();
	curr_y += HBUTTON;

    g_202->end();

	curr_x = x() + GAP;
	curr_y += GAP;
	// Group 2.3 - date and time formats
	Fl_Group* g_203 = new Fl_Group(curr_x, curr_y, 2 * WSMEDIT + 2 * WLLABEL+ GAP, HBUTTON * 2);

	curr_x += WLLABEL;
	// Date format choice
	Fl_Choice* w_20301 = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Date Format");
	w_20301->align(FL_ALIGN_LEFT);
	w_20301->tooltip("Select the way date is output on the QSL card");
	populate_date(w_20301);
	w_20301->value(data_->f_date);
	w_20301->callback(cb_datetime<qsl_data::date_format>, (void*)&data_->f_date);
	ch_data_ = w_20301;

	curr_x += w_20301->w() + WLLABEL;
	// Time format choice
	Fl_Choice* w_20302 = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Time Format");
	w_20302->align(FL_ALIGN_LEFT);
	w_20302->tooltip("Select the way time is output on the QSL card");
	populate_time(w_20302);
	w_20302->value(data_->f_time);
	w_20302->callback(cb_datetime<qsl_data::time_format>, (void*)&data_->f_time);
	ch_time_ = w_20302;

	curr_x = x() + GAP + WLLABEL;
	curr_y += HBUTTON;
	// A button to disable/enable using the current QSO as an example QSO
	Fl_Light_Button* w_20303 = new Fl_Light_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Use Example QSO");
	w_20303->align(FL_ALIGN_LEFT);
	w_20303->tooltip("Show an example QSO rather than the template");
	w_20303->value(show_example_);
	w_20303->callback(cb_example, (void*)&show_example_);

	curr_x += w_20303->w() + GAP;
	// A button to restore image to unscaled size
	Fl_Button* w_20304 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Actual size");
	w_20304->tooltip("Restore the displayed image to actual size");
	w_20304->callback(cb_descale);

	curr_y += HBUTTON;

	g_203->end();
  
    g_2_->resizable(nullptr);
	int w = max(g_201->w() + g_202->w(), g_203->w()) + GAP;
	int h = max(g_201->h(), g_202->h()) + g_203->h();
    g_2_->size(w + GAP, h + GAP);
    g_2_->end();

    max_x = max(max_x, g_2_->x() + g_2_->w());

    curr_x = x() + GAP;
    curr_y = g_2_->y() + g_2_->h();

    curr_x = x() + GAP;

	int avail_height = Fl_Group::h() - (curr_y - y());

	// There is no Group 3

	// Group 4 - show the drawing item data
	g_4_ = new Fl_Group(curr_x, curr_y, 10, avail_height, "Design items");
    g_4_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    g_4_->labelfont(FL_BOLD);
    g_4_->labelsize(FL_NORMAL_SIZE + 2);
    g_4_->box(FL_BORDER_BOX);
	g_4_->end();

	// Add all the items to Group 4
    create_items();

    end();

	resizable(nullptr);
	resize();

	show();
}

// Added as page_dialog only has an abstract
void qsl_editor::enable_widgets() {}

// Resize qsl_editor to fit all three groups
void qsl_editor::resize() {
    // Find size of display and adjust g_3_
    int w = display_->w();
    int h = display_->h();
    w_display_->size(w, h);

    w = max(g_1_->w(), g_2_->w());
    w = max(w, g_4_->w());

    h = g_4_->y() + g_4_->h();
    size(w, h);

	redraw();
}
    
// Save the new design
void qsl_editor::save_values() {

	// Sabe the display window position
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences windows_settings(settings, "Windows");
	Fl_Preferences qsl_win_settings(windows_settings, "QSL Design");
	qsl_win_settings.set("Top", win_y_);
	qsl_win_settings.set("Left", win_x_);

	if (data_->filename_valid && data_->filename.length() == 0) {
		char msg[128];
		snprintf(msg, sizeof(msg), "QSL: No filename specified to save drawing items for %s %s - data may be lost",
			callsign_.c_str(),
			QSL_TYPES[qsl_type_].c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	qsl_dataset_->save_data();
}
// Create the card data item data widgets
void qsl_editor::create_items() {

	// Remove all existing widgets - hide them first 
	g_4_->hide();
	g_4_->clear();
	g_4_->show();
	g_4_->redraw();

	int curr_x = g_4_->x() + GAP;
	int save_x = curr_x;
	int curr_y = g_4_->y() + HTEXT;
	int max_x = curr_x;
	
	Fl_Group* saveg = Fl_Group::current();
	Fl_Group::current(g_4_);

	// Group 4.2 - New item (NB group ordering has changes 4.2 is before 4.1)
	Fl_Group* g_402 = new Fl_Group(save_x + WLABEL, curr_y, 100, HBUTTON);
	g_402->box(FL_FLAT_BOX);

	curr_x = save_x + WLABEL / 2;
	// Nrew item choice selects item type
	Fl_Choice* w_40201 = new Fl_Choice(curr_x, curr_y, WBUTTON, HBUTTON, "New");
	w_40201->tooltip("Select the new type to add");
	w_40201->callback(cb_new_item);
	populate_type(w_40201);
	w_40201->value((int)qsl_data::NONE);

	curr_y += HBUTTON + GAP;
	curr_x += w_40201->w();
	max_x = curr_x;
	g_402->end();

	// Only display column labels if there is data to display
	if (data_->items.size()) {
		create_labels(curr_y);
		curr_y += HBUTTON;
	}
	// Use the remainder of the space for an Fl_Scroll containing the items
	int available_h = g_4_->y() + g_4_->h() - curr_y - GAP;
	curr_x = g_4_->x() + GAP;

	// Fl_Scroll encapsulates a set of Group 4.1.x
	Fl_Scroll* gg_401 = new Fl_Scroll(curr_x, curr_y, 100, available_h);
	gg_401->type(Fl_Scroll::VERTICAL);
	
	// For each item...
	for (int ix = 0; ix < data_->items.size(); ix++) {
		// Get the item
		qsl_data::item_def* item = data_->items[ix];

		if (item->type != qsl_data::NONE) {
			// Group 4.1.x (1 per item) - The item design data for editing
			Fl_Group* g_401 = new Fl_Group(save_x, curr_y, 100, HBUTTON);
			char id[5];
			snprintf(id, 5, "%2d", ix);
			g_401->copy_label(id);
			g_401->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
			g_401->box(FL_FLAT_BOX);

			curr_x = save_x + WLABEL / 2;
			// Choice to allow item type to be changed (NONE effectively deletes it)
			Fl_Choice* w_40101 = new Fl_Choice(curr_x, curr_y, WBUTTON, HBUTTON);
			w_40101->tooltip("Select the item type - NONE to ignore");
			w_40101->callback(cb_ch_type, &item->type);
			populate_type(w_40101);
			w_40101->value((int)item->type);

			curr_x += w_40101->w() + GAP;

			// Now create the individual row
			switch(item->type) {
			case qsl_data::FIELD: {
				// The widgets to edit a field item
				create_fparams(curr_x, curr_y, &item->field);
				break;
			}
			case qsl_data::TEXT: {
				// The widgets to edit a text item
				create_tparams(curr_x, curr_y, &item->text);
				break;
			}
			case qsl_data::IMAGE: {
				// The widgets to edit an image item
				create_iparams(curr_x, curr_y, &item->image);
				break;
			}
			default:
				break;
			}
			g_401->resizable(nullptr);
			g_401->size(curr_x - g_401->x(), g_401->h());
			g_401->end();
			max_x = max(max_x, curr_x);
		}
	}
	gg_401->resizable(nullptr);
	gg_401->size(max_x - gg_401->x() + GAP +  Fl::scrollbar_size(), gg_401->h());
	gg_401->end();
	max_x = max(max_x, gg_401->x() + gg_401->w());

	curr_y = gg_401->y() + available_h + GAP;
	max_x = max(max_x, curr_x) + GAP + Fl::scrollbar_size();

	g_4_->resizable(nullptr);
	g_4_->size(max_x - g_4_->x(), g_4_->h());

	Fl_Group::current(saveg);

	redraw();
};

// Create the boxes containing column headers
void qsl_editor::create_labels(int curr_y) {
	int curr_x = g_4_->x() + GAP + WLABEL / 2;
	Fl_Box* b_type = new Fl_Box(curr_x, curr_y, WBUTTON, HBUTTON, "Type");
	b_type->tooltip("Select the item type - NONE to ignore");
	curr_x += b_type->w() + GAP;
	Fl_Box* b_x = new Fl_Box(curr_x, curr_y, WBUTTON / 2, HBUTTON, "X");
	b_x->tooltip("Enter the X-coordinates of the item");
	curr_x += b_x->w();
	Fl_Box* b_y = new Fl_Box(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Y");
	b_y->tooltip("Enter the Y-coordinates of the item");
	curr_x += b_y->w() + GAP;
	Fl_Box* b_data = new Fl_Box(curr_x, curr_y, WEDIT + HBUTTON, HBUTTON, "Contents data");
	b_data->tooltip("Enter contents:\nField: label and field and their style\n"
		"Text: Text to be displayed and its style\n"
		"Image: Filename of image");
	curr_x += b_data->w();
	Fl_Box* b_vp = new Fl_Box(curr_x, curr_y, HBUTTON, HBUTTON, "V");
	b_vp->tooltip("Select vertical positioning of label and text");
	curr_x += b_vp->w();
	Fl_Box* b_mq = new Fl_Box(curr_x, curr_y, HBUTTON, HBUTTON, "MQ");
	b_mq->tooltip("Select if multiple QSOs are displayed on separate lines");
	curr_x += b_mq->w();
	Fl_Box* b_box = new Fl_Box(curr_x, curr_y, HBUTTON, HBUTTON, "B");
	b_box->tooltip("Select if the item is to be enclosed in a box");
	curr_x += b_box->w();
	Fl_Box* b_dn = new Fl_Box(curr_x, curr_y, HBUTTON, HBUTTON, "DE");
	b_dn->tooltip("Select if the item is to be displayed if null value");
}

// Create the widgets to edit field item values
void qsl_editor::create_fparams(int& curr_x, int& curr_y, qsl_data::field_def* field) {
	char temp[100];

	// Item position (X-coordinate relative to display)
	Fl_Input* f_dx = new Fl_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	f_dx->tooltip("Enter the X-coordinates of the item");
	f_dx->callback(cb_ip_int, &(field->dx));
	f_dx->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", field->dx);
	f_dx->value(temp);

	curr_x += f_dx->w();
	// Item position (Y-coordinate relative to display)
	Fl_Input* f_dy = new Fl_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	f_dy->tooltip("Enter the Y-coordinates of the item");
	f_dy->callback(cb_ip_int, &(field->dy));
	f_dy->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", field->dy);
	f_dy->value(temp);
	curr_x += f_dy->w() + GAP;

	// Label to add to the left or above the item value
	Fl_Input* f_label = new Fl_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	f_label->tooltip("Enter the label to appear beside the item");
	f_label->value(field->label.c_str());
	f_label->callback(cb_ip_string, &(field->label));
	f_label->when(FL_WHEN_CHANGED);
	curr_x += f_label->w();

	// Button opens a font, size and colour dialog for the label
	// The button's label will be displayed in the selected font, size and colour
	Fl_Button* f_lstyle = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "S");
	f_lstyle->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_CLIP);
	f_lstyle->callback(cb_bn_style, (void*)&field->l_style);
	f_lstyle->when(FL_WHEN_RELEASE);
	f_lstyle->color(FL_WHITE);
	f_lstyle->labelcolor(field->l_style.colour);
	f_lstyle->labelfont(field->l_style.font);
	f_lstyle->labelsize(field->l_style.size);
	snprintf(temp, sizeof(temp), "%s: %d pt. Click to change", 
		Fl::get_font_name(Fl_Font(field->l_style.font), nullptr), 
		field->l_style.size);
	f_lstyle->copy_tooltip(temp);

	curr_x += f_lstyle->w();
	const int WFIELD = WEDIT - WBUTTON - HBUTTON;
	// A choice to allow the ADIF field to use
	field_choice* f_field = new field_choice(curr_x, curr_y, WFIELD, HBUTTON);
	f_field->set_dataset("Fields");
	f_field->tooltip("Select the field to display");
	f_field->value(field->field.c_str());
	f_field->callback(cb_ch_field, &field->field);

	curr_x += f_field->w();
	// Button opens a font, size and colour dialog for the value
	// The button's label will be displayed in the selected font, size and colour
	Fl_Button* f_tstyle = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "S");
	f_tstyle->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_CLIP);
	f_tstyle->callback(cb_bn_style, (void*)&field->t_style);
	f_tstyle->when(FL_WHEN_RELEASE);
	f_tstyle->color(FL_WHITE);
	f_tstyle->labelcolor(field->t_style.colour);
	f_tstyle->labelfont(field->t_style.font);
	f_tstyle->labelsize(field->t_style.size);
	snprintf(temp, sizeof(temp), "%s: %d pt. Click to change", 
		Fl::get_font_name(Fl_Font(field->t_style.font), nullptr), 
		field->t_style.size);
	f_tstyle->copy_tooltip(temp);

	curr_x += f_tstyle->w();
	// Button to select vertical (label above value) or horizontal (label left of value)
	Fl_Check_Button* f_vert = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_vert->tooltip("Select vertical positioning of label and text");
	f_vert->callback(cb_ip_bool, &(field->vertical));
	f_vert->when(FL_WHEN_RELEASE);
	f_vert->value(field->vertical);

	curr_x += f_vert->w();
	// Button to select how values from multiple QSOs are displayed
	Fl_Check_Button* f_mqso = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_mqso->tooltip("Select if multiple QSOs are displayed on separate lines");
	f_mqso->callback(cb_ip_bool, &(field->multi_qso));
	f_mqso->when(FL_WHEN_RELEASE);
	f_mqso->value(field->multi_qso);

	curr_x += f_mqso->w();
	// Button to select whether a box is displayed around the item value
	Fl_Check_Button* f_box = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_box->tooltip("Select if the item is to be enclosed in a box");
	f_box->callback(cb_ip_bool, &(field->box));
	f_box->value(field->box);
	f_box->when(FL_WHEN_RELEASE);

	curr_x += f_box->w();
	// Button to select whether the label and box are displayed if the value is an empty string
	Fl_Check_Button* f_ignore = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_ignore->tooltip("Select if the item is to be displayed if null value");
	f_ignore->callback(cb_ip_bool, (void*)&field->display_empty);
	f_ignore->value(field->display_empty);
	f_ignore->when(FL_WHEN_RELEASE);

	curr_x += f_ignore->w();
	curr_y += HBUTTON;
}

// Craete the widgets to edit a text item
void qsl_editor::create_tparams(int& curr_x, int& curr_y, qsl_data::text_def* text) {
	char temp[100];

	// X-position of the item relative to the display
	Fl_Input* t_dx = new Fl_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	t_dx->tooltip("Enter the X-coordinates of the item");
	t_dx->callback(cb_ip_int, &(text->dx));
	t_dx->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", text->dx);
	t_dx->value(temp);

	curr_x += t_dx->w();
	// Y-position of the item relativ eto the display window
	Fl_Input* t_dy = new Fl_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	t_dy->tooltip("Enter the Y-coordinates of the item");
	t_dy->callback(cb_ip_int, &(text->dy));
	t_dy->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", text->dy);
	t_dy->value(temp);
	curr_x += t_dy->w() + GAP;

	// Input to display text to wroite on card
	Fl_Input* t_text = new intl_input(curr_x, curr_y, WEDIT, HBUTTON);
	t_text->tooltip("Please enter the text to display");
	t_text->callback(cb_ip_string, &text->text);
	t_text->when(FL_WHEN_CHANGED);
	t_text->value(text->text.c_str());
	curr_x += t_text->w();

	// Button to open a font, size and colour dialog for the text value
	// The button's label will be displayed in the selected font, size and colour
	Fl_Button* t_tstyle = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "S");
	t_tstyle->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_CLIP);
	t_tstyle->callback(cb_bn_style, (void*)&text->t_style);
	t_tstyle->when(FL_WHEN_RELEASE);
	t_tstyle->color(FL_WHITE);
	t_tstyle->labelcolor(text->t_style.colour);
	t_tstyle->labelfont(text->t_style.font);
	t_tstyle->labelsize(text->t_style.size);
	snprintf(temp, sizeof(temp), "%s: %d pt. Click to change", 
		Fl::get_font_name(Fl_Font(text->t_style.font), nullptr), 
		text->t_style.size);
	t_tstyle->copy_tooltip(temp);

	curr_x += t_tstyle->w();

	// Button to select vertical (label above value) or horizontal (label left of value)
	Fl_Check_Button* t_vert = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	t_vert->tooltip("Select for vertical alignment of text item to next");
	t_vert->callback(cb_ip_bool, &(text->vertical));
	t_vert->when(FL_WHEN_RELEASE);
	t_vert->value(text->vertical);

	curr_x += t_vert->w();
	curr_y += HBUTTON;
}

// Create the widgets to edit image items
void qsl_editor::create_iparams(int& curr_x, int& curr_y, qsl_data::image_def* image) {
	char temp[100];

	// X-position of the image relative to the display window
	Fl_Input* i_dx = new Fl_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	i_dx->tooltip("Enter the X-coordinates of the item");
	i_dx->callback(cb_ip_int, &(image->dx));
	i_dx->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", image->dx);
	i_dx->value(temp);

	curr_x += i_dx->w();
	// Y-position of the image relative to the display window
	Fl_Input* i_dy = new Fl_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	i_dy->tooltip("Enter the X-coordinates of the item");
	i_dy->callback(cb_ip_int, &(image->dy));
	i_dy->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", image->dy);
	i_dy->value(temp);
	curr_x += i_dy->w() + GAP;

	// Input - image filename
	filename_input* i_filename = new filename_input(curr_x, curr_y, WEDIT + HBUTTON, HBUTTON);
	i_filename->callback(cb_image, image);
	i_filename->when(FL_WHEN_CHANGED);
	i_filename->value(image->filename.c_str());
	i_filename->tooltip("Location of image file");
	i_filename->type(filename_input::FILE);
	i_filename->title("Please select image file");
	i_filename->pattern("Image Files\t*.{png,bmp,jpg}");

	curr_x += i_filename->w();
	curr_y += HBUTTON;

}

// After editing the item data, redraw the window
void qsl_editor::redraw_display(bool dirty) {
	// Mark data dirty if done as a result of edit
	// if (dirty) {
	// 	qsl_dataset_->dirty(data_);
	// }
	// Update qsl_display
	if (data_->items.size()) {
		load_tsv_ = false;
		bn_loadtsv_->deactivate();		
	} else {
		bn_loadtsv_->activate();
	}
	if (show_example_) {
		example_qso_ = qso_manager_->data()->current_qso();
		if (example_qso_ && example_qso_->item("STATION_CALLSIGN") == callsign_) {
			// Showing the example qSO as it exists and matches the callsign 
			qsl_->set_qsos(&example_qso_, 1);
		} else if (callsign_.length() == 0) {
			callsign_ = qso_manager_->get_default(qso_manager::CALLSIGN);
			ip_callsign_->value(callsign_.c_str());
			// Load the display settings for this callsign
			data_ = qsl_dataset_->get_card(callsign_, qsl_type_, true);
			if (data_->filename_valid || load_tsv_) {
				ip_filename_->value(data_->filename.c_str());
				ip_filename_->activate();
			} else {
				ip_filename_->deactivate();
			}
			qsl_->set_qsos(&example_qso_, 1);
		} else {
			// Either there is no qSO ipor a different callsign was used
			char msg[100];
			snprintf(msg, sizeof(msg), "QSL: Current QSO does not match callsign %s",
				callsign_.c_str());
			status_->misc_status(ST_WARNING, msg);
			qsl_->set_qsos(nullptr, 0);
		}
	} else {
		//if (qso_manager_) {
		//	callsign_ = qso_manager_->get_default(qso_manager::CALLSIGN);
		//	ip_callsign_->value(callsign_.c_str());
		//	// Load the display settings for this callsign
		//	data_ = qsl_dataset_->get_card(callsign_, qsl_type_, true);
		//	ip_filename_->value(data_->filename.c_str());
		//}
		// Do not show an example QSO
		if (data_->filename_valid || load_tsv_) {
			ip_filename_->value(data_->filename.c_str());
			ip_filename_->activate();
		} else {
			ip_filename_->deactivate();
		}
		qsl_->set_qsos(nullptr, 0);
	}
	// We may have changed the size
	qsl_->set_card(data_);
	if (!dirty) {
		// Do not restore size of editing
		int w, h;
		qsl_->get_size(w, h);
		w_display_->size(w, h);
		w_display_->redraw();
	}
	else {
		w_display_->redraw();
	}

}

// Update the size widget
void qsl_editor::update_size() {
	char temp[64];
	int pw;
	int ph;
	// Convert supplied data to pixels
	switch(data_->unit) {
		case qsl_data::INCH: 
			pw = data_->width * IN_TO_POINT;
			ph = data_->height * IN_TO_POINT;
			break;
		case qsl_data::MILLIMETER:
			pw = data_->width * MM_TO_POINT;
			ph = data_->height * MM_TO_POINT;
			break;
		case qsl_data::POINT:
			pw = data_->width;
			ph = data_->height;
			break;
		default:
			pw = 0;
			ph = 0;
			break;
	}
	snprintf(temp, sizeof(temp), "%d x %d", pw, ph);
	op_size_->value(temp);
}

// Restore user_data fpor all widgets thatrequire access to data
void qsl_editor::update_dimensions() {
	switch (data_->unit) {
	case qsl_data::INCH:
		bn_inch_->value(true);
		bn_mm_->value(false);
		bn_point_->value(false);
		break;
	case qsl_data::MILLIMETER:
		bn_inch_->value(false);
		bn_mm_->value(true);
		bn_point_->value(false);
		break;
	case qsl_data::POINT:
		bn_inch_->value(false);
		bn_mm_->value(false);
		bn_point_->value(true);
		break;
	}
	ip_cols_->user_data(&data_->columns);
	ip_cols_->value(data_->columns);
	ip_width_->user_data(&data_->width);
	ip_width_->value(data_->width);
	ip_cpos_->user_data(&data_->col_left);
	ip_cpos_->value(data_->col_left);
	ip_cspace_->user_data(&data_->col_width);
	ip_cspace_->value(data_->col_width);
	ip_rows_->user_data(&data_->rows);
	ip_rows_->value(data_->rows);
	ip_height_->user_data(&data_->height);
	ip_height_->value(data_->height);
	ip_rpos_->user_data(&data_->row_top);
	ip_rpos_->value(data_->row_top);
	ip_rspace_->user_data(&data_->row_height);
	ip_rspace_->value(data_->row_height);
	ip_qsos_->user_data(&data_->max_qsos);
	ip_qsos_->value(data_->max_qsos);
	ch_data_->user_data(&data_->f_date);
	ch_data_->value(data_->f_date);
	ch_time_->user_data(&data_->f_time);
	ch_time_->value(data_->f_time);
	update_size();
}

// Call back when a radio button is pressed - v indicates which button
void qsl_editor::cb_radio_dim(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->data_->unit = ((qsl_data::dim_unit)(intptr_t)v);
	that->update_size();
	that->redraw_display(true);
}

// Want to change the style (font, size and colour) of the text
void qsl_editor::cb_bn_style(Fl_Widget* w, void* v) {
	// Get the current style
   	qsl_editor* that = ancestor_view<qsl_editor>(w);
	qsl_data::style_def* style = (qsl_data::style_def*)v;
	// Open a font, size and colour dialog
    font_dialog* d = new font_dialog(style->font, style->size, style->colour, "QSL Editor - text style chooser");
    button_t result = d->display();
    if (result == BN_OK) {
        style->font = d->font();
        style->size = d->font_size();
        style->colour = d->colour();
        that->redraw_display(true);
		that->create_items();
    }
    Fl::delete_widget(d);
}

// Want to change a size parameter (widgth or height of label)
void qsl_editor::cb_size_double(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<Fl_Value_Input, double>(w, v);
	that->update_size();
    that->redraw_display(true);
}

// String input
void qsl_editor::cb_ip_string(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<intl_input, string>(w, v);
    that->redraw_display(true);  
	// that->create_items();
}

// integer input
void qsl_editor::cb_ip_int(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value_int<Fl_Input>(w, v);
	that->update_size();
	that->redraw_display(true);  
	// that->create_items();
}

// Bool input
void qsl_editor::cb_ip_bool(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<Fl_Check_Button, bool>(w, v);
    that->redraw_display(true);  
	// that->create_items();
}

// Callsign 
void qsl_editor::cb_callsign(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<field_input, string>(w, v);
	that->data_ = qsl_dataset_->get_card(that->callsign_, that->qsl_type_, true);
	that->qsl_->set_card(that->data_);
	if (that->data_->filename_valid) {
		that->filedata_.filename = &(that->data_->filename);
		that->ip_filename_->value(that->data_->filename.c_str());
		that->ip_filename_->user_data(&that->data_->filename);
	}
	that->update_dimensions();
	// Only using existing data - not edited
    that->redraw_display(false);  
	that->create_items();
	that->redraw();
}

// Load TSV
void qsl_editor::cb_bn_loadtsv(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	cb_value<Fl_Check_Button, bool>(w, v);
	that->redraw_display(false);
}

// Field
void qsl_editor::cb_ch_field(Fl_Widget* w, void* v) { 
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	field_choice* ch = (field_choice*)w;
	const char* field = ch->value();
	string* item_field = (string*)v;
	*item_field = field;
	that->redraw_display(true);
}

// Filename changed so update display
void qsl_editor::cb_filename(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	cb_value<Fl_Input, string>(w, &(that->data_->filename));
	qsl_dataset_->load_items(that->data_);
	that->redraw_display(true);
	that->create_items();
	that->redraw();
}

// Type choice
void qsl_editor::cb_ch_type(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	Fl_Choice* ch = (Fl_Choice*)w;
	// v points to the type field of that item
	qsl_data::item_type* type = (qsl_data::item_type*)v;
	*type = (qsl_data::item_type)ch->value();
	that->redraw_display(true);
	that->create_items();
}

// New item
void qsl_editor::cb_new_item(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	Fl_Choice* ch = (Fl_Choice*)w;
	qsl_data::item_def* item = new qsl_data::item_def();
	item->type = (qsl_data::item_type)ch->value();
	if (item->type == qsl_data::IMAGE) {
		item->image.filename = that->data_->filename;
	}
	that->data_->items.push_back(item);
	that->redraw_display(true);
	that->create_items();
}

// Image
void qsl_editor::cb_image(Fl_Widget* w, void* v) {
	qsl_data::image_def& image = *(qsl_data::image_def*)v;
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	cb_value<Fl_Input, string>(w, &image.filename);

	if (!that->relative_filename(image.filename)) {
		char msg[256];
		snprintf(msg, sizeof(msg), "QSL: Unable to create a path relative to %s for %s",
			that->data_->filename.c_str(), image.filename.c_str());
		status_->misc_status(ST_WARNING, msg);
	}
	that->redraw_display(true);
	that->create_items();
}

// Select date or time formats
template<class ENUM>
void qsl_editor::cb_datetime(Fl_Widget* w, void* v) {
	cb_value<Fl_Choice, ENUM>(w, v);
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->redraw_display(true);
}

// Use example qSO in display
void qsl_editor::cb_example(Fl_Widget* w, void* v) {
	cb_value<Fl_Light_Button, bool>(w, v);
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->redraw_display(true);
}

// Seelect different QSL type
void qsl_editor::cb_qsl_type(Fl_Widget* w, void* v) {
	cb_value<Fl_Choice, int>(w, v);
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->data_ = qsl_dataset_->get_card(that->callsign_, that->qsl_type_, true);
	that->qsl_->set_card(that->data_);
	if (that->data_->filename_valid) {
		that->filedata_.filename = &(that->data_->filename);
		that->ip_filename_->value(that->data_->filename.c_str());
		that->ip_filename_->user_data(&that->data_->filename);
	}
	that->update_dimensions();
	// Only using existing data - not edited
	that->redraw_display(false);
	that->create_items();
	that->redraw();
}

// Restore the drawing size
void qsl_editor::cb_descale(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->redraw_display(false);
}

void qsl_editor::create_display() {
	Fl_Group* save = Fl_Group::current();
	Fl_Group::current(nullptr);
	w_display_ = new Fl_Window(100, 100);
	w_display_->position(win_x_, win_y_);
	char l[128];
	snprintf(l, sizeof(l), "QSL Card Design: %s", callsign_.c_str());
	w_display_->copy_label(l);
	w_display_->border(true);
	display_ = new qsl_widget(0, 0, w_display_->w(), w_display_->h());
	display_->box(FL_FLAT_BOX);
	w_display_->resizable(display_);
	w_display_->end();
	w_display_->show();
	Fl_Group::current(save);
}

// Populate the type choices
void qsl_editor::populate_type(Fl_Choice* ch) {
	ch->clear();
	ch->add("NONE");
	ch->add("Field");
	ch->add("Text");
	ch->add("Image");
}

// Populate the date choice
void qsl_editor::populate_date(Fl_Choice* ch) {
	ch->clear();
	time_t now = time(nullptr);
	tm* utc = gmtime(&now);
	// 
	char value[25];
	// YYYMMDD
	strftime(value, sizeof(value), "%Y%m%d", utc);
	ch->add(value);
	// YYYY/MM/DD
	strftime(value, sizeof(value), "%Y\\/%m\\/%d", utc);
	ch->add(value);
	// YY/MM/DD
	strftime(value, sizeof(value), "%y\\/%m\\/%d", utc);
	ch->add(value);
	// DD/MM/YY
	strftime(value, sizeof(value), "%d\\/%m\\/%y", utc);
	ch->add(value);
	// MM/DD/YY
	strftime(value, sizeof(value), "%m\\/%d\\/%y", utc);
	ch->add(value);
}

// Populate the time choice
void qsl_editor::populate_time(Fl_Choice* ch) {
	ch->clear();
	time_t now = time(nullptr);
	tm* utc = gmtime(&now);
	// 
	char value[25];
	// HHMMSS
	strftime(value, sizeof(value), "%H%M%S", utc);
	ch->add(value);
	// HH:MM:SS
	strftime(value, sizeof(value), "%H:%M:%S", utc);
	ch->add(value);
	// HHMM
	strftime(value, sizeof(value), "%H%M", utc);
	ch->add(value);
	// HH:MM
	strftime(value, sizeof(value), "%H:%M", utc);
	ch->add(value);
}

void qsl_editor::populate_qsl_type(Fl_Choice* ch) {
	ch->clear();
	for (int ix = 0; ix < qsl_data::MAX_TYPE; ix++) {
		ch->add(QSL_TYPES[ix].c_str());
	}
}

// Update example call
void qsl_editor::update() {
	redraw_display(false);
}

// Remove the full path name - leave what's left after the QSL Data path name
bool qsl_editor::relative_filename(string& filename) {
	string path = qsl_dataset_->get_path();
	// Make both names portable
	forward_slash(path);
	forward_slash(filename);
	string result;
	char msg[256];
	// First check it's directly under the path
	if (filename.substr(0, path.length()) == path) {
		result = filename.substr(path.length());
		// Remove any leading stroke character
		if (result[0] == '/' || result[0] == '\\') {
			result = result.substr(1);
		}
		filename = result;
		return true;
	}
	// else - not directly in path
	bool found = false;
	size_t len;
	string save_path = path;
	// Remove any trainling searator from path
	if (path.back() == '/' || path.back() == '\\') {
		path = path.substr(0, path.length() - 1);
	}
	// Now one character ata  time compare the strings
	for (len = 0; !found && len < path.length(); len++) {
		if (filename.compare(0, len + 1, path) <= 0) {
			found = true;
		}
	}
	// len should now have the number of characters that match
	if (len == 0) {
		snprintf(msg, sizeof(msg), "QSL: No common path between %s and %s", filename.c_str(), save_path.c_str());
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	// Now behead the filename by the length of match
	result = filename.substr(len);
	// Scan the remainder of path for the number of separator and prepend "../" for each one
	size_t pos = len;
	while (pos != string::npos) {
		pos = path.find_first_of("/\\", pos);
		result = "../" + result;
	}
	snprintf(msg, sizeof(msg), "QSL: Common path found - setting filename to %s", result.c_str());
	status_->misc_status(ST_NOTE, msg);
	filename = result;
	return true;
}

