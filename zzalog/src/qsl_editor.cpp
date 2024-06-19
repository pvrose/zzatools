#include "qsl_editor.h"
#include "status.h"
#include "qso_manager.h"
#include "settings.h"
#include "utils.h"
#include "field_choice.h"
#include "intl_widgets.h"
#include "font_dialog.h"

#include <string>
#include <ctime>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Input.H>

using namespace std;

extern Fl_Preferences* settings_;
extern qso_manager* qso_manager_;
extern status* status_;

qsl_editor::qsl_editor(int X, int Y, int W, int H, const char* L) :
    page_dialog(X, Y, W, H, L),
	show_example_(false)
{

    load_values();
	create_display();

    display_->value(callsign_);
    display_->size(width_, height_, unit_, number_qsos_);
    display_->editable(true);

	if (active()) w_display_->show();
	else w_display_->hide();
 
    create_form(X, Y);

}

qsl_editor::~qsl_editor() {
	Fl::delete_widget(w_display_);
}

int qsl_editor::handle(int event) {
	switch(event) {
	case FL_DEACTIVATE: {
		if (display_) w_display_->hide();
		break;
	}
	case FL_ACTIVATE: {
		if (display_) w_display_->show();
		break;
	}
	}
	return page_dialog::handle(event);
}

void qsl_editor::load_values() {
    callsign_ = qso_manager_->get_default(qso_manager::CALLSIGN);

	Fl_Preferences qsl_settings(settings_, "QSL Design");
	Fl_Preferences call_settings(qsl_settings, callsign_.c_str());
		
	call_settings.get("Unit", (int&)unit_, (int)qsl_display::MILLIMETER);
	call_settings.get("Width", width_, 99.1);
	call_settings.get("Height", height_, 66.7);
	call_settings.get("Number Rows", num_rows_, 4);
	call_settings.get("Number Columns", num_cols_, 2);
	call_settings.get("Column Width", col_width_, 101.6);
	call_settings.get("Row Height", row_height_, 67.7);
	call_settings.get("First Row", row_top_, 12.9);
	call_settings.get("First Column", col_left_, 4.6);
	call_settings.get("Max QSOs per Card", number_qsos_, 1);
	call_settings.get("Date Format", (int&)date_format_, (int)qsl_display::FMT_Y4MD_ADIF);
	call_settings.get("Time Format", (int&)time_format_, (int)qsl_display::FMT_HMS_ADIF);
	char * temp;
	call_settings.get("Card Design", temp, "");
	filename_ = temp;

	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences qsl_win_settings(windows_settings, "QSL Design");
	qsl_win_settings.get("Top", win_y_, 10);
	qsl_win_settings.get("Left", win_x_, 10);

}

void qsl_editor::create_form(int X, int Y) {
	
    int curr_x = X + GAP;
    int curr_y = Y + GAP;
    int max_x;

    g_1_ = new Fl_Group(curr_x, curr_y, 100, 100, "Template File");
    g_1_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    g_1_->labelfont(FL_BOLD);
    g_1_->labelsize(FL_NORMAL_SIZE + 2);
    g_1_->box(FL_BORDER_BOX);

	curr_y += HTEXT;

    field_input* w101 = new field_input(curr_x + WLLABEL, curr_y, WSMEDIT, HBUTTON, "Callsign for QSL");
    w101->value(callsign_.c_str());
    w101->callback(cb_callsign, &callsign_);
    w101->field_name("STATION_CALLSIGN");
	w101->tooltip("Select the callsign to change QSL template parameters for");

    curr_y += HBUTTON;
    intl_input* w102 = new intl_input(curr_x + GAP, curr_y, WEDIT * 2, HBUTTON);
    w102->callback(cb_filename, &filename_);
    w102->when(FL_WHEN_CHANGED);
    w102->value(filename_.c_str());
    w102->tooltip("Please enter the QSL template");

    curr_x += w102->w() + GAP;

    Fl_Button* w103 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Browse");
    w103->align(FL_ALIGN_INSIDE);
    filedata_ = { "Please enter the QSL template", "TSV\t*.tsv", &filename_, nullptr, w102, nullptr};
    w103->callback(cb_bn_browsefile, &filedata_);
    w103->when(FL_WHEN_RELEASE);
	w103->tooltip("Opens a file browser to locate the QSL template file");

    curr_x += w103->w() + GAP;
    curr_y += w103->h() + GAP;
    g_1_->resizable(nullptr);
    g_1_->size(curr_x - g_1_->x(), curr_y - g_1_->y());
    g_1_->end();

    max_x = g_1_->x() + g_1_->w();

    curr_x = x() + GAP;

    g_2_ = new Fl_Group(curr_x, curr_y, 100, 100, "Size & format parameters");
    g_2_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    g_2_->labelfont(FL_BOLD);
    g_2_->labelsize(FL_NORMAL_SIZE + 2);
    g_2_->box(FL_BORDER_BOX);

	curr_y += HTEXT;

	// radio buttons to select size units
	curr_x += GAP;
	Fl_Group* g_201 = new Fl_Group(curr_x, curr_y, WBUTTON, HBUTTON * 3);
	g_201->box(FL_FLAT_BOX);
	// Radio to select inches
	Fl_Radio_Round_Button* w_20101 = new Fl_Radio_Round_Button(curr_x, curr_y, HBUTTON, HBUTTON, "inch");
	w_20101->value(unit_ == qsl_display::INCH);
	w_20101->selection_color(FL_RED);
	w_20101->align(FL_ALIGN_RIGHT);
	w_20101->callback(cb_radio_dim, (void*)qsl_display::INCH);
	w_20101->tooltip("Measurements in inches");
    curr_y += HBUTTON;
	// Radio to select millimetres
	Fl_Radio_Round_Button* w_20102 = new Fl_Radio_Round_Button(curr_x, curr_y, HBUTTON, HBUTTON, "mm");
	w_20102->value(unit_ == qsl_display::MILLIMETER);
	w_20102->selection_color(FL_RED);
	w_20102->align(FL_ALIGN_RIGHT);
	w_20102->callback(cb_radio_dim, (void*)qsl_display::MILLIMETER);
	w_20102->tooltip("Measurements in millimetres");
	// Radio to select points
    curr_y += HBUTTON;
	Fl_Radio_Round_Button* w_20103 = new Fl_Radio_Round_Button(curr_x, curr_y, HBUTTON, HBUTTON, "point");
	w_20103->value(unit_ == qsl_display::POINT);
	w_20103->selection_color(FL_RED);
	w_20103->align(FL_ALIGN_RIGHT);
	w_20103->callback(cb_radio_dim, (void*)qsl_display::POINT);
	w_20103->tooltip("Measurements in printer points");
	g_201->end();

    curr_x += g_201->w();
    curr_y = g_201->y();
    Fl_Group* g_202 = new Fl_Group(curr_x, curr_y, 4*(WBUTTON + WLABEL), 3*HBUTTON + HTEXT + GAP);
    g_202->box(FL_FLAT_BOX);

    curr_x += WLABEL;
    // Input to set number of columns to print
	Fl_Value_Input* w_20201 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Columns");
	w_20201->value(num_cols_);
	w_20201->align(FL_ALIGN_LEFT);
	w_20201->callback(cb_value<Fl_Value_Input, int>, &num_cols_);
//	w_20201->when(FL_WHEN_ENTER_KEY);
	w_20201->tooltip("Please specify the number of columns when printing");
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the width of the label
	Fl_Value_Input* w_20202 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Width");
	w_20202->value(width_);
	w_20202->align(FL_ALIGN_LEFT);
	w_20202->callback(cb_size_double, &width_);
	w_20202->when(FL_WHEN_ENTER_KEY);
	w_20202->tooltip("Please specify the width of the label in the selected unit");
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the position ofthe left edge of culmn 1
	Fl_Value_Input* w_20203 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Position");
	w_20203->value(col_left_);
	w_20203->align(FL_ALIGN_LEFT);
	w_20203->callback(cb_value<Fl_Value_Input, double>, &col_left_);
//	w_20203->when(FL_WHEN_ENTER_KEY);
	w_20203->tooltip("Please specify the position of the first column");
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the gap between columns
	Fl_Value_Input* w_20204 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Spacing");
	w_20204->value(col_width_);
	w_20204->align(FL_ALIGN_LEFT);
	w_20204->callback(cb_value<Fl_Value_Input, double>, &col_width_);
//	w_20204->when(FL_WHEN_ENTER_KEY);
	w_20204->tooltip("Please specify the spacing between columns");
    curr_x = g_202->x();
    curr_x += WLABEL;
    curr_y += HBUTTON;
	// Row 2A - height parameters
	// Input to specify the number of rows of labels to print
	Fl_Value_Input* w_20205 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Rows");
	w_20205->value(num_rows_);
	w_20205->align(FL_ALIGN_LEFT);
	w_20205->callback(cb_value<Fl_Value_Input, int>, &num_rows_);
//	w_20205->when(FL_WHEN_ENTER_KEY);
	w_20205->tooltip("Please specify the number of rows when printing");
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the height of each label
	Fl_Value_Input* w_20206 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Height");
	w_20206->value(height_);
	w_20206->align(FL_ALIGN_LEFT);
	w_20206->callback(cb_size_double, &height_);
	w_20206->when(FL_WHEN_ENTER_KEY);
	w_20206->tooltip("Please specify the width of the label in the selected label");
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the top of the first label
	Fl_Value_Input* w_20207 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Position");
	w_20207->value(row_top_);
	w_20207->align(FL_ALIGN_LEFT);
	w_20207->callback(cb_value<Fl_Value_Input, double>, &row_top_);
//	w_20207->when(FL_WHEN_ENTER_KEY);
	w_20207->tooltip("Please specify the position of the first row");
    curr_x += WLABEL + WBUTTON;;
	// Input to specify the gap between each row
	Fl_Value_Input* w_20208 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Spacing");
	w_20208->value(row_height_);
	w_20208->align(FL_ALIGN_LEFT);
	w_20208->callback(cb_value<Fl_Value_Input, double>, &row_height_);
//	w_20208->when(FL_WHEN_ENTER_KEY);
	w_20208->tooltip("Please specify the spacing between rows");
    curr_x = g_202->x();
	curr_x += WLABEL;
    curr_y += HBUTTON;
	// Input to specify the (max) number of QSOs per QSL
	Fl_Value_Input* w_20209 = new Fl_Value_Input(curr_x, curr_y, WBUTTON, HBUTTON, "QSOs/Card");
	w_20209->value(number_qsos_);
	w_20209->align(FL_ALIGN_LEFT);
	w_20209->callback(cb_value<Fl_Value_Input, int>, &number_qsos_);
//	w_20209->when(FL_WHEN_ENTER_KEY);
	w_20209->tooltip("Please specify the number of QSOs per Card");
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
	Fl_Group* g_203 = new Fl_Group(curr_x, curr_y, 2 * WSMEDIT + 2 * WLLABEL+ GAP, HBUTTON * 2);

	curr_x += WLLABEL;
	Fl_Choice* w_20301 = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Date Format");
	w_20301->align(FL_ALIGN_LEFT);
	w_20301->tooltip("Select the way date is output on the QSL card");
	populate_date(w_20301);
	w_20301->value(date_format_);
	w_20301->callback(cb_datetime<qsl_display::date_format>, (void*)&date_format_);

	curr_x += w_20301->w() + WLLABEL;
	Fl_Choice* w_20302 = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Time Format");
	w_20302->align(FL_ALIGN_LEFT);
	w_20302->tooltip("Select the way date is output on the QSL card");
	populate_time(w_20302);
	w_20302->value(time_format_);
	w_20302->callback(cb_datetime<qsl_display::time_format>, (void*)&time_format_);

	curr_x = x() + GAP + WLLABEL;
	curr_y += HBUTTON;

	Fl_Light_Button* w_20303 = new Fl_Light_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Use Example QSO");
	w_20303->align(FL_ALIGN_LEFT);
	w_20303->tooltip("Show an example QSO rather than the template");
	w_20303->value(show_example_);
	w_20303->callback(cb_example, (void*)&show_example_);

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

	g_4_ = new Fl_Group(curr_x, curr_y, 10, avail_height, "Design widgets");
    g_4_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    g_4_->labelfont(FL_BOLD);
    g_4_->labelsize(FL_NORMAL_SIZE + 2);
    g_4_->box(FL_BORDER_BOX);
	g_4_->end();

    create_items();

    end();

	resizable(nullptr);
	resize();

	show();
}

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
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
	Fl_Preferences call_settings(qsl_settings, callsign.c_str());

	call_settings.set("Unit", (int)unit_);
	call_settings.set("Width", width_);
	call_settings.set("Height", height_);
	call_settings.set("Number Rows", num_rows_);
	call_settings.set("Number Columns", num_cols_);
	call_settings.set("Column Width", col_width_);
	call_settings.set("Row Height", row_height_);
	call_settings.set("First Row", row_top_);
	call_settings.set("First Column", col_left_);
	call_settings.set("Max QSOs per Card", number_qsos_);
	call_settings.set("Card Design", filename_.c_str());
	call_settings.set("Date Format", (int)date_format_);
	call_settings.set("Time Format", (int)time_format_);

    display_->save_data();

	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences qsl_win_settings(windows_settings, "QSL Design");
	qsl_win_settings.set("Top", win_y_);
	qsl_win_settings.set("Left", win_x_);

	settings_->flush();
}

// Create item groups
void qsl_editor::create_items() {
	g_4_->clear();

	int curr_x = g_4_->x() + GAP;
	int save_x = curr_x;
	int curr_y = g_4_->y() + HTEXT;
	int max_x = curr_x;
	g_4_->begin();

	if (display_->data()->size()) {
		create_labels();
		curr_y += HBUTTON;
	}
	// Leave space for the New Item button
	int available_h = g_4_->y() + g_4_->h() - curr_y - HBUTTON - GAP - GAP;

	Fl_Scroll* gg_401 = new Fl_Scroll(curr_x, curr_y, 100, available_h);
	gg_401->type(Fl_Scroll::VERTICAL);
	
	for (int ix = 0; ix < display_->data()->size(); ix++) {
		qsl_display::item_def* item = (*display_->data())[ix];

		if (item->type != qsl_display::NONE) {
			Fl_Group* g_401 = new Fl_Group(save_x, curr_y, 100, HBUTTON);
			char id[5];
			snprintf(id, 5, "%2d", ix);
			g_401->copy_label(id);
			g_401->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
			g_401->box(FL_FLAT_BOX);

			curr_x = save_x + WLABEL / 2;
			Fl_Choice* w_40101 = new Fl_Choice(curr_x, curr_y, WBUTTON, HBUTTON);
			w_40101->tooltip("Select the item type - NONE to ignore");
			w_40101->callback(cb_ch_type, &item->type);
			populate_type(w_40101);
			w_40101->value((int)item->type);

			curr_x += w_40101->w() + GAP;

			// Now create the individual row
			switch(item->type) {
			case qsl_display::FIELD: {
				create_fparams(curr_x, curr_y, &item->field);
				break;
			}
			case qsl_display::TEXT: {
				create_tparams(curr_x, curr_y, &item->text);
				break;
			}
			case qsl_display::IMAGE: {
				create_iparams(curr_x, curr_y, &item->image);
				break;
			}
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
	Fl_Group* g_402 = new Fl_Group(save_x + WLABEL, curr_y, 100, HBUTTON);
	g_402->box(FL_FLAT_BOX);

	curr_x = save_x + WLABEL / 2;
	// Now the varioius widgets
	Fl_Choice* w_40201 = new Fl_Choice(curr_x, curr_y, WBUTTON, HBUTTON, "New");
	w_40201->tooltip("Select the new type to add");
	w_40201->callback(cb_new_item);
	populate_type(w_40201);
	w_40201->value((int)qsl_display::NONE);

	curr_y += HBUTTON + GAP;
	curr_x += w_40201->w();
	max_x = max(max_x, curr_x) + GAP + Fl::scrollbar_size();

	g_4_->resizable(nullptr);
	g_4_->size(max_x - g_4_->x(), g_4_->h());
	g_4_->end();

	redraw();
};

void qsl_editor::create_labels() {
	int curr_x = g_4_->x() + GAP + WLABEL / 2;
	int curr_y = g_4_->y() + HTEXT;
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

void qsl_editor::create_fparams(int& curr_x, int& curr_y, qsl_display::field_def* field) {
	char temp[100];

	Fl_Input* f_dx = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	f_dx->tooltip("Enter the X-coordinates of the item");
	f_dx->callback(cb_ip_int, &(field->dx));
	f_dx->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", field->dx);
	f_dx->value(temp);

	curr_x += f_dx->w();
	Fl_Input* f_dy = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	f_dy->tooltip("Enter the Y-coordinates of the item");
	f_dy->callback(cb_ip_int, &(field->dy));
	f_dy->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", field->dy);
	f_dy->value(temp);
	curr_x += f_dy->w() + GAP;

	Fl_Input* f_label = new Fl_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	f_label->tooltip("Enter the label to appear beside the item");
	f_label->value(field->label.c_str());
	f_label->callback(cb_ip_string, &(field->label));
	f_label->when(FL_WHEN_CHANGED);
	curr_x += f_label->w();

	Fl_Button* f_lstyle = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "S");
	f_lstyle->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
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
	// Now the varioius widgets
	const int WFIELD = WEDIT - WBUTTON - HBUTTON;
	field_choice* f_field = new field_choice(curr_x, curr_y, WFIELD, HBUTTON);
	f_field->set_dataset("Fields");
	f_field->tooltip("Select the field to display");
	f_field->value(field->field.c_str());
	f_field->callback(cb_ch_field, &field);

	curr_x += f_field->w();
	Fl_Button* f_tstyle = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "S");
	f_tstyle->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
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
	Fl_Check_Button* f_vert = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_vert->tooltip("Select vertical positioning of label and text");
	f_vert->callback(cb_ip_bool, &(field->vertical));
	f_vert->when(FL_WHEN_RELEASE);
	f_vert->value(field->vertical);

	curr_x += f_vert->w();
	Fl_Check_Button* f_mqso = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_mqso->tooltip("Select if multiple QSOs are displayed on separate lines");
	f_mqso->callback(cb_ip_bool, &(field->multi_qso));
	f_mqso->when(FL_WHEN_RELEASE);
	f_mqso->value(field->multi_qso);

	curr_x += f_mqso->w();
	Fl_Check_Button* f_box = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_box->tooltip("Select if the item is to be enclosed in a box");
	f_box->callback(cb_ip_bool, &(field->box));
	f_box->value(field->box);
	f_box->when(FL_WHEN_RELEASE);

	curr_x += f_box->w();
	Fl_Check_Button* f_ignore = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	f_ignore->tooltip("Select if the item is to be displayed if null value");
	f_ignore->callback(cb_ip_bool, (void*)&field->display_empty);
	f_ignore->value(field->display_empty);
	f_ignore->when(FL_WHEN_RELEASE);

	curr_x += f_ignore->w();
	curr_y += HBUTTON;
}




void qsl_editor::create_tparams(int& curr_x, int& curr_y, qsl_display::text_def* text) {
	char temp[100];

	Fl_Input* t_dx = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	t_dx->tooltip("Enter the X-coordinates of the item");
	t_dx->callback(cb_ip_int, &(text->dx));
	t_dx->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", text->dx);
	t_dx->value(temp);

	curr_x += t_dx->w();
	Fl_Input* t_dy = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	t_dy->tooltip("Enter the Y-coordinates of the item");
	t_dy->callback(cb_ip_int, &(text->dy));
	t_dy->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", text->dy);
	t_dy->value(temp);
	curr_x += t_dy->w() + GAP;

	Fl_Input* t_text = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON);
	t_text->tooltip("Please enter the text to display");
	t_text->callback(cb_ip_string, &text->text);
	t_text->when(FL_WHEN_CHANGED);
	t_text->value(text->text.c_str());
	curr_x += t_text->w();

	Fl_Button* t_tstyle = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "S");
	t_tstyle->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	t_tstyle->tooltip("Opens up a dialog to set font style and colour");
	t_tstyle->callback(cb_bn_style, (void*)&text->t_style);
	t_tstyle->when(FL_WHEN_RELEASE);
	t_tstyle->color(FL_WHITE);
	t_tstyle->labelcolor(text->t_style.colour);
	t_tstyle->labelfont(text->t_style.font);
	t_tstyle->labelsize(text->t_style.size);

	curr_x += t_tstyle->w();
	curr_y += HBUTTON;
}

void qsl_editor::create_iparams(int& curr_x, int& curr_y, qsl_display::image_def* image) {
	char temp[100];

	Fl_Input* i_dx = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	i_dx->tooltip("Enter the X-coordinates of the item");
	i_dx->callback(cb_ip_int, &(image->dx));
	i_dx->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", image->dx);
	i_dx->value(temp);

	curr_x += i_dx->w();
	Fl_Input* i_dy = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
	i_dy->tooltip("Enter the X-coordinates of the item");
	i_dy->callback(cb_ip_int, &(image->dy));
	i_dy->when(FL_WHEN_CHANGED);
	snprintf(temp, sizeof(temp), "%d", image->dy);
	i_dy->value(temp);
	curr_x += i_dy->w() + GAP;

	// Input - image filename
	Fl_Input* i_filename = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON);
	i_filename->callback(cb_value<intl_input, string>, &image->filename);
	i_filename->when(FL_WHEN_CHANGED);
	i_filename->value(image->filename.c_str());
	i_filename->tooltip("Location of TQSL executable");
	curr_x += i_filename->w();

	// Button - Opens file browser to locate executable
	Fl_Button* bn_browse_tqsl = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "?");
	bn_browse_tqsl->align(FL_ALIGN_INSIDE);
	bn_browse_tqsl->callback(cb_image, &image);
	bn_browse_tqsl->when(FL_WHEN_RELEASE);
	bn_browse_tqsl->tooltip("Opens a file browser to locate the image file");

	curr_x += bn_browse_tqsl->w();
	curr_y += HBUTTON;

}


void qsl_editor::redraw_display() {
	display_->size(width_, height_, unit_, number_qsos_);
	w_display_->size(display_->w(), display_->h());
	display_->formats(date_format_, time_format_);
	if (show_example_) {
		example_qso_ = qso_manager_->data()->current_qso();
		if (example_qso_ && example_qso_->item("STATION_CALLSIGN") == callsign_) {
			display_->example_qso(example_qso_);
		} else {
			char msg[100];
			snprintf(msg, sizeof(msg), "QSL DESIGNER: Current QSO does not match callsign %s",
				callsign_.c_str());
			status_->misc_status(ST_WARNING, msg);
			display_->example_qso(nullptr);
		} 
	} else {
		display_->example_qso(nullptr);
	}
	display_->redraw();
}

// Update teh size display
void qsl_editor::update_size() {
	char temp[64];
	int pw;
	int ph;
	switch(unit_) {
		case qsl_display::INCH: 
			pw = width_ * IN_TO_POINT;
			ph = height_ * IN_TO_POINT;
			break;
		case qsl_display::MILLIMETER:
			pw = width_ * MM_TO_POINT;
			ph = height_ * MM_TO_POINT;
			break;
		case qsl_display::POINT:
			pw = width_;
			ph = height_;
			break;
	}
	snprintf(temp, sizeof(temp), "%d x %d", pw, ph);
	op_size_->value(temp);
}

void qsl_editor::enable_widgets() {
	update_size();
	create_items();
	resize();
	redraw_display();
}

// Call back when a radio button is pressed - v indicates which button
void qsl_editor::cb_radio_dim(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->unit_ = ((qsl_display::dim_unit)(intptr_t)v);
    that->redraw_display();
}

// The design has been interactively edited
void qsl_editor::cb_design(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
    that->create_items();
    that->resize();
}

// Want to change the style of the text
void qsl_editor::cb_bn_style(Fl_Widget* w, void* v) {
   	qsl_editor* that = ancestor_view<qsl_editor>(w);
	qsl_display::style_def* style = (qsl_display::style_def*)v;
    font_dialog* d = new font_dialog(style->font, style->size, style->colour, "QSL Editor - text style chooser");
    button_t result = d->display();
    if (result == BN_OK) {
        style->font = d->font();
        style->size = d->font_size();
        style->colour = d->colour();
        that->redraw_display();
		that->create_items();
    }
    Fl::delete_widget(d);
}

// Want to change a size parameter (widgth or height of label)
void qsl_editor::cb_size_double(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<Fl_Value_Input, double>(w, v);
    that->redraw_display();
}

// String input
void qsl_editor::cb_ip_string(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<Fl_Input, string>(w, v);
    that->redraw_display();  
	// that->create_items();
}

// integer input
void qsl_editor::cb_ip_int(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value_int<Fl_Int_Input>(w, v);
    that->redraw_display();  
	// that->create_items();
}

// Bool input
void qsl_editor::cb_ip_bool(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<Fl_Check_Button, bool>(w, v);
    that->redraw_display();  
	// that->create_items();
}

// Callsign
void qsl_editor::cb_callsign(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<field_input, string>(w, v);
    that->redraw_display();  
	that->create_items();
}

// Field
void qsl_editor::cb_ch_field(Fl_Widget* w, void* v) { 
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	field_choice* ch = (field_choice*)w;
	const char* field = ch->value();
	string* item_field = (string*)v;
	*item_field = field;
	that->redraw_display();
	// that->create_items();
}

// Filename changed so update display
void qsl_editor::cb_filename(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	cb_value<intl_input, string>(w, v);
	that->save_values();
	that->redraw_display();
}

// Type choice
void qsl_editor::cb_ch_type(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	Fl_Choice* ch = (Fl_Choice*)w;
	qsl_display::item_type* type = (qsl_display::item_type*)v;
	*type = (qsl_display::item_type)ch->value();
	that->redraw_display();
	that->create_items();
}

// New item
void qsl_editor::cb_new_item(Fl_Widget* w, void* v) {
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	Fl_Choice* ch = (Fl_Choice*)w;
	qsl_display::item_type* type = (qsl_display::item_type*)v;
	qsl_display::item_def* item = new qsl_display::item_def();
	item->type = (qsl_display::item_type)ch->value();
	that->display_->data()->push_back(item);
	that->redraw_display();
	that->create_items();
}

// Image
void qsl_editor::cb_image(Fl_Widget* w, void* v) {
	qsl_display::image_def& image = *(qsl_display::image_def*)v;
	browser_data_t bd = {
		"Please select image file", "*.png|*.bmp|*.jpg", &image.filename, nullptr, nullptr, nullptr };
	
	cb_bn_browsefile(w, (void*)&bd);
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	image.image = that->display_->get_image(image.filename);
	that->redraw_display();
	that->create_items();
}

// Date time
template<class ENUM>
void qsl_editor::cb_datetime(Fl_Widget* w, void* v) {
	cb_value<Fl_Choice, ENUM>(w, v);
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->save_values();
	that->redraw_display();
}

// Show example
void qsl_editor::cb_example(Fl_Widget* w, void* v) {
	cb_value<Fl_Light_Button, bool>(w, v);
	qsl_editor* that = ancestor_view<qsl_editor>(w);
	that->redraw_display();
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
	display_ = new qsl_display(0, 0, 100, 100);

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

