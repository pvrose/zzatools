#include "qsl_editor.h"
#include "status.h"
#include "qso_manager.h"
#include "settings.h"
#include "utils.h"
#include "field_choice.h"
#include "intl_widgets.h"
#include "font_dialog.h"

#include <string>

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
    page_dialog(X, Y, W, H, L)
{
    load_values();
	create_display();

    display_->value(callsign_);
    display_->size(width_, height_, unit_);
    display_->editable(true);
 
    create_form(X, Y);

}

qsl_editor::~qsl_editor() {
	Fl::delete_widget(w_display_);
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

    g_2_ = new Fl_Group(curr_x, curr_y, 100, 100, "Size parameters");
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
	Fl_Output* w_20210 = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON, "Size in pt.");
	w_20210->align(FL_ALIGN_LEFT);
	w_20210->tooltip("Displays the size of the label in points");
	op_size_ = w_20210;
	update_size();

    g_202->end();
    
    g_2_->resizable(nullptr);
    g_2_->size(g_201->w() + g_202->w() + GAP + GAP, max(g_201->h(), g_202->h()) + GAP);
    g_2_->end();

    max_x = max(max_x, g_2_->x() + g_2_->w());

    curr_x = x() + GAP;
    curr_y = g_2_->y() + g_2_->h() + GAP;

    curr_x = x() + GAP;

	g_4_ = new Fl_Group(curr_x, curr_y, 10, 10, "Design widgets");
    g_4_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
    g_4_->labelfont(FL_BOLD);
    g_4_->labelsize(FL_NORMAL_SIZE + 2);
    g_4_->box(FL_BORDER_BOX);
	g_4_->end();

    draw_items();

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

	call_settings.set("Unit", (int&)unit_);
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

    display_->save_data();

	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences qsl_win_settings(windows_settings, "QSL Design");
	qsl_win_settings.set("Top", win_y_);
	qsl_win_settings.set("Left", win_x_);

	settings_->flush();
}

// Add data item - append it to the list of data items
void qsl_editor::add_item(string field) {
	qsl_display::item_def* item = new qsl_display::item_def;
	item->field = field;
	display_->data()->push_back(item);
	// Redraw the group
	redraw_display();
	draw_items();
	resize();
}

// Delete data item
void qsl_editor::delete_item(int number) {
	display_->data()->erase(display_->data()->begin() + number);
	// Redraw the group
	draw_items();
	resize();
}

// Create item groups
void qsl_editor::draw_items() {
	int curr_x = g_4_->x() + GAP;
	int save_x = curr_x;
	int curr_y = g_4_->y() + HBUTTON + GAP;
	int top_y = g_4_->y() + GAP;
	char temp[16];
	g_4_->clear();
	Fl_Group* save = Fl_Group::current();
	g_4_->begin();
	for (int ix = 0; ix < display_->data()->size(); ix++) {
		Fl_Group* g_401 = new Fl_Group(save_x + WLABEL, curr_y, 100, HBUTTON);
		char id[5];
		snprintf(id, 5, "%2d", ix);
		g_401->copy_label(id);
		g_401->align(FL_ALIGN_LEFT);
		g_401->box(FL_FLAT_BOX);

		curr_x = save_x + WLABEL;
		qsl_display::item_def* item = (*display_->data())[ix];
		// Now the varioius widgets
		field_choice* w_40101 = new field_choice(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON);
		w_40101->set_dataset("Fields");
		w_40101->tooltip("Select the field to display");
		w_40101->value(item->field.c_str());
		w_40101->callback(cb_ch_field, (void*)(intptr_t)ix);

		curr_x += w_40101->w();
		Fl_Input* w_40102 = new Fl_Input(curr_x, curr_y, WBUTTON, HBUTTON);
		w_40102->tooltip("Enter the label to appear beside the item");
		w_40102->value(item->label.c_str());
		w_40102->callback(cb_ip_string, &(item->label));
		w_40102->when(FL_WHEN_ENTER_KEY);
		w_40102->textcolor(item->l_style.colour);
		w_40102->textfont(item->l_style.font);
		w_40102->textsize(item->l_style.size);
		w_40102->color(FL_WHITE);

		curr_x += w_40102->w();
		Fl_Button* w_40102a = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Value");
		w_40102a->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
		w_40102a->tooltip("Opens up a dialog to set font style and colour");
		w_40102a->callback(cb_bn_style, (void*)&item->l_style);
		w_40102a->when(FL_WHEN_RELEASE);
		w_40102a->color(FL_WHITE);
		w_40102a->labelcolor(item->l_style.colour);
		w_40102a->labelfont(item->l_style.font);
		w_40102a->labelsize(item->l_style.size);

		curr_x += w_40102a->w();
		Fl_Button* w_40103 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Value");
		w_40103->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
		w_40103->tooltip("Opens up a dialog to set font style and colour");
		w_40103->callback(cb_bn_style, (void*)&item->t_style);
		w_40103->when(FL_WHEN_RELEASE);
		w_40103->color(FL_WHITE);
		w_40103->labelcolor(item->t_style.colour);
		w_40103->labelfont(item->t_style.font);
		w_40103->labelsize(item->t_style.size);

		curr_x += w_40103->w();
		Fl_Input* w_40104 = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
		w_40104->tooltip("Enter the X-coordinates of the item");
		w_40104->callback(cb_ip_int, &(item->dx));
		w_40104->when(FL_WHEN_ENTER_KEY);
		snprintf(temp, sizeof(temp), "%d", item->dx);
		w_40104->value(temp);

		curr_x += w_40104->w();
		Fl_Input* w_40105 = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
		w_40105->tooltip("Enter the X-coordinates of the item");
		w_40105->callback(cb_ip_int, &(item->dy));
		w_40105->when(FL_WHEN_ENTER_KEY);
		snprintf(temp, sizeof(temp), "%d", item->dy);
		w_40105->value(temp);

		curr_x += w_40105->w();
		Fl_Input* w_40106 = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
		w_40106->tooltip("Enter the X-coordinates of the item");
		w_40106->callback(cb_ip_int, &(item->dw));
		w_40106->when(FL_WHEN_ENTER_KEY);
		snprintf(temp, sizeof(temp), "%d", item->dw);
		w_40106->value(temp);

		curr_x += w_40106->w();
		Fl_Input* w_40107 = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON);
		w_40107->tooltip("Enter the X-coordinates of the item");
		w_40107->callback(cb_ip_int, &(item->dh));
		w_40107->when(FL_WHEN_ENTER_KEY);
		snprintf(temp, sizeof(temp), "%d", item->dh);
		w_40107->value(temp);

		curr_x += w_40107->w();
		Fl_Button* w_40108 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON);
		w_40108->tooltip("Select the position of the label to the item: Above or Left");
		w_40108->callback(cb_bn_align, (void*)(intptr_t)ix);
		w_40108->type(FL_TOGGLE_BUTTON);
		w_40108->when(FL_WHEN_RELEASE);
		switch(item->align) {
			case FL_ALIGN_LEFT: 
				strcpy(temp, "Left");
				w_40108->value(false);
				break;
			case FL_ALIGN_TOP:
				strcpy(temp, "Above");
				w_40108->value(true);
				break;
			default:
				strcpy(temp, "Invalid");
				break;
		}
		w_40108->copy_label(temp);

		curr_x += w_40108->w();
		Fl_Check_Button* w_40109 = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
		w_40109->tooltip("Select if multiple QSOs are displayed on separate lines");
		w_40109->callback(cb_ip_bool, &(item->multi_qso));
		w_40109->when(FL_WHEN_RELEASE);
		w_40109->value(item->multi_qso);

		curr_x += w_40109->w();
		Fl_Check_Button* w_40110 = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
		w_40110->tooltip("Select if the item is to be enclosed in a box");
		w_40110->callback(cb_bn_box, (void*)(intptr_t)ix);
		w_40110->value(item->box == FL_BORDER_BOX);
		w_40110->when(FL_WHEN_RELEASE);

		curr_x += w_40110->w();
		Fl_Check_Button* w_40111 = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
		w_40111->tooltip("Select if the item is to be displayed if null value");
		w_40111->callback(cb_ip_bool, (void*)&item->display_empty);
		w_40111->value(item->display_empty);
		w_40111->when(FL_WHEN_RELEASE);

		curr_x += w_40110->w();
		g_401->resizable(nullptr);
		g_401->size(curr_x - g_401->x(), g_401->h());			
		g_401->end();

		if (ix == 0) {
			Fl_Box* b1 = new Fl_Box(w_40101->x(), top_y, w_40101->w(), HBUTTON, "Field");
			b1->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b2 = new Fl_Box(w_40102->x(), top_y, w_40102->w(), HBUTTON, "Label");
			b2->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b2a = new Fl_Box(w_40102a->x(), top_y, w_40102a->w(), HBUTTON, "St");
			b2a->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b3 = new Fl_Box(w_40103->x(), top_y, w_40103->w(), HBUTTON, "Style");
			b3->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b4 = new Fl_Box(w_40104->x(), top_y, w_40104->w(), HBUTTON, "X");
			b4->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b5 = new Fl_Box(w_40105->x(), top_y, w_40105->w(), HBUTTON, "Y");
			b5->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b6 = new Fl_Box(w_40106->x(), top_y, w_40106->w(), HBUTTON, "W");
			b6->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b7 = new Fl_Box(w_40107->x(), top_y, w_40107->w(), HBUTTON, "H");
			b7->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b8 = new Fl_Box(w_40108->x(), top_y, w_40108->w(), HBUTTON, "Align");
			b8->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b9 = new Fl_Box(w_40109->x(), top_y, w_40109->w(), HBUTTON, "*1");
			b9->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b10 = new Fl_Box(w_40110->x(), top_y, w_40110->w(), HBUTTON, "*2");
			b10->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
			Fl_Box* b11 = new Fl_Box(w_40111->x(), top_y, w_40111->w(), HBUTTON, "*3");
			b11->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
		}

		curr_y += HBUTTON;
		
		curr_x += GAP;

	}
	int max_x = curr_x;

	Fl_Group* g_402 = new Fl_Group(save_x + WLABEL, curr_y, 100, HBUTTON, "New item");
	g_402->align(FL_ALIGN_LEFT);
	g_402->box(FL_FLAT_BOX);

	curr_x = save_x + WLABEL;
	// Now the varioius widgets
	field_choice* w_40201 = new field_choice(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON);
	w_40201->set_dataset("Fields");
	w_40201->tooltip("Select the field to add");
	w_40201->value("");
	w_40201->callback(cb_new_field);

	if (display_->data()->size()) {
		curr_x += w_40201->w() + GAP;
		Fl_Box* b20 = new Fl_Box(curr_x, curr_y, WEDIT, HBUTTON, "*1 Multiple QSOs, *2 Show box, *3 Show if empty");
		b20->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		max_x = max(max_x, b20->x() + b20->w()) + GAP;
	} else {
		max_x = max(max_x, w_40201->x() + w_40201->w()) + GAP;

	}

	curr_y += HBUTTON + GAP;

	g_4_->resizable(nullptr);
	g_4_->size(max_x - g_4_->x(), curr_y - g_4_->y());
	Fl_Group::current(save);

	redraw();

}

void qsl_editor::redraw_display() {
	display_->size(width_, height_, unit_);
	w_display_->size(display_->w(), display_->h());
	display_->create_form();
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
	draw_items();
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
    that->draw_items();
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
		that->draw_items();
    }
    Fl::delete_widget(d);
}

// Want to change the alignment of accompanying text
void qsl_editor::cb_bn_align(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    int ix = (intptr_t)(v);
    bool value;
    cb_value<Fl_Check_Button, bool>(w, (void*)&value);
    qsl_display::item_def* item = (*that->display_->data())[ix];
    item->align = value ? FL_ALIGN_TOP : FL_ALIGN_LEFT;
    that->redraw_display();
	that->draw_items();
}

// Want to change the box style 
void qsl_editor::cb_bn_box(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    int ix = (intptr_t)(v);
    bool value;
    cb_value<Fl_Check_Button, bool>(w, (void*)&value);
    qsl_display::item_def* item = (*that->display_->data())[ix];
    item->box = value ? FL_BORDER_BOX : FL_FLAT_BOX;
    that->redraw_display();
	that->draw_items();
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
	that->draw_items();
}

// integer input
void qsl_editor::cb_ip_int(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value_int<Fl_Int_Input>(w, v);
    that->redraw_display();  
	that->draw_items();
}

// Bool input
void qsl_editor::cb_ip_bool(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<Fl_Check_Button, bool>(w, v);
    that->redraw_display();  
	that->draw_items();
}

// Callsign
void qsl_editor::cb_callsign(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
    cb_value<field_input, string>(w, v);
    that->redraw_display();  
	that->draw_items();
}

// Field
void qsl_editor::cb_ch_field(Fl_Widget* w, void* v) { 
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	field_choice* ch = (field_choice*)w;
	const char* field = ch->value();
	int ix = (int)(intptr_t)v;
	if (!strlen(field)) {
		that->delete_item(ix);
	}
}

// New field
void qsl_editor::cb_new_field(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	field_choice* ch = (field_choice*)w;
	const char* field = ch->value();
	int ix = (int)(intptr_t)v;
	if (strlen(field)) {
		that->add_item(string(field));
	}

}

// Filename changed so update display
void qsl_editor::cb_filename(Fl_Widget* w, void* v) {
    qsl_editor* that = ancestor_view<qsl_editor>(w);
	cb_value<intl_input, string>(w, v);
	that->save_values();
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

