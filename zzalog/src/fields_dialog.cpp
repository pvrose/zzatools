#include "fields_dialog.h"
#include "book.h"
#include "spec_data.h"
#include "intl_widgets.h"
#include "drawing.h"

#include <set>
#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Preferences.H>

using namespace std;



extern Fl_Preferences* settings_;
extern spec_data* spec_data_;
extern book* book_;

// Table constructor
fields_table::fields_table(int X, int Y, int W, int H, const char* label) :
	Fl_Table_Row(X, Y, W, H, label)
	, fields_(nullptr)
{
	// Set table parameters
	cols(3);
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_GRAY);
	row_header(false);
	selection_color(FL_YELLOW);
	type(Fl_Table_Row::SELECT_SINGLE);
}

// Table destructor
fields_table::~fields_table() {

}

// inherited from Fl_Table_Row - provide the contents and formats when drawing cells
void fields_table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	string text;
	field_info_t* field;
	// Get the field that goes in the row
	if (fields_ == nullptr || fields_->size() == 0) {
		// There is none
		field = nullptr;
	}
	else {
		// Index into array by row number
		field = &(*fields_)[R];
	}

	switch (context) {

	case CONTEXT_STARTPAGE:
		// Set the table font
		fl_font(0, FL_NORMAL_SIZE);
		return;

	case CONTEXT_ENDPAGE:
		// Do nothing
		return;

	case CONTEXT_ROW_HEADER:
		// Do nothing
		return;

	case CONTEXT_COL_HEADER:
		// put field header text into header
		fl_push_clip(X, Y, W, H);
		fl_draw_box(FL_FLAT_BOX, X, Y, W, H, col_header_color());
		fl_color(FL_BLACK);
		switch (C) {
		case 0:
			text = "Column";
			break;
		case 1:
			text = "Width";
			break;
		case 2:
			text = "Header";
			break;
		}
		fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
		// Get content from fields[R].what C is
		fl_push_clip(X, Y, W, H);
		// BG COLOR
		fl_color(row_selected(R) ? selection_color() : FL_WHITE);
		fl_rectf(X, Y, W, H);

		// TEXT
		fl_color(FL_BLACK);
		if (field == nullptr) {
			// No field to display
			text = "";
		}
		else {
			switch (C) {
			case 0:
				// Field name
				text = field->field;
				break;
			case 1:
				// Field width
				text = to_string(field->width);
				break;
			case 2:
				// Text to use as header in the application view
				text = field->header;
				break;
			}
		}
		fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);

		// BORDER
		fl_color(FL_LIGHT1);
		// draw top and right edges only
		fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
		fl_pop_clip();
		return;
	}
}

// Set the fields to display in the table
void fields_table::fields(vector<field_info_t>* fields) {
	fields_ = fields;
	if (fields_ == nullptr) {
		// No fields to display - so no rows
		rows(0);
	}
	else {
		// Set the number of rows to the size of the array and set a common row height
		rows(fields_->size());
		row_height_all(ROW_HEIGHT);
	}
}

// returns the lowest number selected row (-1 if none)
int fields_table::row() {
	// As multiple rows can be selected in the generic Fl_Table_Row there isn't a simple way
	// Go through all the rows and return the number selected
	for (int i = 0; i < rows(); i++) {
		if (row_selected(i)) return i;
	}
	return -1;
}

// Dialog constructor
fields_dialog::fields_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)
{
	// Initialise all data
	selection_in_used_ = false;
	field_set_name_ = "Default";
	field_sets_.clear();
	unused_fields_.clear();
	application_ = FO_MAINLOG;
	field_set_by_app_.clear();
	// Create the dialog
	do_creation(X, Y);
}

// Dialog destructor
fields_dialog::~fields_dialog()
{
	// Remove all information about field sets and release memory
	for (auto it = field_sets_.begin(); it != field_sets_.end(); it++) {
		it->second->clear();
		delete it->second;
	}
	field_sets_.clear();
	unused_fields_.clear();
	field_set_by_app_.clear();

}

// Load data from settings
void fields_dialog::load_values() {
	// Read the field data from the settings
	Fl_Preferences display_settings(settings_, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	// Number of field sets (a field set is a set of fields and their ordering)
	int num_field_sets = fields_settings.groups();
	// First read the default application to display first
	fields_settings.get("Active Application", (int&)application_, FO_MAINLOG);
	// Then get the field sets for each
	char app_path[128];
	char* temp;
	// For each field ordering application
	for (int i = 0; i < FO_LAST; i++) {
		// Get the field set name
		sprintf(app_path, "App%d", i);
		fields_settings.get(app_path, temp, "Default");
		field_set_by_app_[(field_ordering_t)i] = string(temp);
		free(temp);
	}
	// Default the field set to display that for the active application
	field_set_name_ = field_set_by_app_[application_];
	// Delete the existing field data
	for (auto it = field_sets_.begin(); it != field_sets_.end(); it++) {
		it->second->clear();
		delete it->second;
	}
	field_sets_.clear();
	// Read all the field field sets
	if (num_field_sets == 0) {
		// No field sets in setting, initialise a new one with default field set
		vector<field_info_t>* field_set = new vector<field_info_t>;
		// For all fields in default field set add it to the new field set
		for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
			field_set->push_back(DEFAULT_FIELDS[i]);
		}
		field_sets_["Default"] = field_set;
	}
	else {
		// For all the field sets in the settings
		for (int i = 0; i < num_field_sets; i++) {
			vector<field_info_t>* field_set = new vector<field_info_t>;
			// Get the name of the i'th field set
			string colln_name = fields_settings.group(i);
			Fl_Preferences colln_settings(fields_settings, colln_name.c_str());
			// Create an initial array for the number of fields in the settings
			int num_fields = colln_settings.groups();
			field_set->resize(num_fields);
			// For each field in the field set
			for (int j = 0; j < num_fields; j++) {
				// Read the field info: name, width and heading from the settings
				field_info_t field;
				string field_id = colln_settings.group(j);
				int field_num = stoi(field_id.substr(6)); // "Field n"
				Fl_Preferences field_settings(colln_settings, field_id.c_str());
				field_settings.get("Width", (int&)field.width, 50);
				char * temp;
				field_settings.get("Header", temp, "");
				field.header = temp;
				free(temp);
				field_settings.get("Name", temp, "");
				field.field = temp;
				free(temp);
				(*field_set)[j] = field;
			}
			// Add the field set to the list of field sets
			field_sets_[colln_name] = field_set;
		}
	}
	// Generate the list of fields not used in the displayed field set.
	unused_fields();
}

// Used to create the form
void fields_dialog::create_form(int X, int Y) {
	begin();

	// positioning constants
	// Group 1 - Select set

	const int XG1 = X + EDGE;
	const int X1_1 = XG1 + GAP;
	const int X1_2 = X1_1 + WSMEDIT + GAP;
	const int X1_3 = X1_2 + WSMEDIT + GAP;
	const int X1_4 = X1_3 + WBUTTON;

	const int WG1 = X1_4 + WSMEDIT + GAP - XG1;
	const int YG1 = Y + EDGE;
	const int Y1 = YG1 + GAP + HTEXT;
	const int HG1 = Y1 + HTEXT + GAP - YG1;

	// Group 1 surround
	Fl_Group* gp1 = new Fl_Group(XG1, YG1, WG1, HG1, "Select field set");
	gp1->labelsize(FL_NORMAL_SIZE + 2);
	gp1->labelfont(FL_BOLD);
	gp1->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	gp1->box(FL_BORDER_BOX);

	// Choice - select name of field-set to use
	Fl_Choice* ch1_1 = new Fl_Choice(X1_1, Y1, WSMEDIT, HTEXT, "Select by name");
	// Add the name of each set of field descriptions to the SelectionName choice
	// set the value of the choice to the number associated with the new name
	auto it = field_sets_.begin();
	for (int i = 0; it != field_sets_.end(); it++, i++) {
		ch1_1->add(it->first.c_str());
		if (field_set_name_ == it->first) {
			ch1_1->value(i);
		}
	}
	ch1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ch1_1->tooltip("Select the name of the set of fields to be listed");
	ch1_1->callback(cb_ch_sel_col);
	ch1_1->when(FL_WHEN_RELEASE);
	name_choice_ = ch1_1;

	// Choice select field-set by an application
	Fl_Choice* ch1_2 = new Fl_Choice(X1_2, Y1, WSMEDIT, HTEXT, "Select by app");
	for (int i = 0; i < (int)FO_LAST; i++) {
		ch1_2->add(APP_NAMES[i].c_str());
		if ((field_ordering_t)i == application_) {
			ch1_2->value(i);
		}
	}
	ch1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ch1_2->tooltip("Select the name of the application for which to list the set of fields");
	ch1_2->callback(cb_ch_sel_app);
	ch1_2->when(FL_WHEN_RELEASE);
	app_choice_ = ch1_2;

	// New button
	Fl_Button* bn1_3 = new Fl_Button(X1_3, Y1, WBUTTON, HBUTTON, "New");
	bn1_3->callback(cb_bn_new);
	bn1_3->when(FL_WHEN_RELEASE);
	bn1_3->tooltip("Create a new set of fields from the default set");

	// Name of new field set
	Fl_Input* ip1_4 = new Fl_Input(X1_4, Y1, WSMEDIT, HTEXT, "Name of new set");
	ip1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	name_input_ = ip1_4;

	gp1->end();
	
	// Group 2 - Fields apply to
	const int XG2 = X + EDGE;
	int X2[FO_LAST];
	for (int i = 0; i < FO_LAST; i++) {
		X2[i] = XG2 + GAP + (i * WBUTTON);
	}
	const int WG2 = X2[FO_LAST - 1] + WBUTTON + GAP - XG2;
	const int YG2 = YG1 + HG1 + GAP;
	const int Y2 = YG2 + HTEXT;
	const int HG2 = Y2 + HBUTTON + GAP - YG2;

	// Allow the view that uses the field-set to be slected
	Fl_Group* gp2 = new Fl_Group(XG2, YG2, WG2, HG2);
	gp2->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	gp2->box(FL_BORDER_BOX);
	// Add the app buttons
	for (int i = 0; i < FO_LAST; i++) {
		Fl_Light_Button* bn2 = new Fl_Light_Button(X2[i], Y2, WBUTTON, HBUTTON, APP_NAMES[i].c_str());
		bn2->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
		bn2->tooltip("Select application to use the current set of fields in it");
		bn2->callback(cb_bn_use_app, (void*)i);
		bn2->when(FL_WHEN_RELEASE);
		bn2->selection_color(FL_BLUE);
		app_buttons_[i] = bn2;
	}
	gp2->end();
	app_group_ = gp2;

	// Group 3 - tables and controls - use whats left
	const int WG3 = w() - EDGE - EDGE;
	const int HG3 = h() - HG1 - HG2 - GAP - GAP - EDGE - EDGE;
	const int XG3 = X + EDGE;
	const int YG3 = YG2 + HG2 + GAP;
	// Allow one row for 2 input widget symmetric about centre line
	const int WG3_2 = WSMEDIT;
	const int XG3_2A = XG3 + (WG3 / 2) - WG3_2;
	const int XG3_2B = XG3 + (WG3 / 2) + WLABEL;
	const int HG3_2 = HTEXT;
	const int YG3_2 = YG3 + HG3 - GAP - HG3_2;
	// now one row with 2 tables and 4 buttons vertical between them
	const int YG3_1 = YG3 + HTEXT + HTEXT;
	const int HG3_1 = YG3_2 - YG3_1 - GAP;
	const int WG3_1B = HBUTTON; // make these buttons square
	const int XG3_1B = XG3 + ((WG3 - WG3_1B) / 2);
	const int HG3_1B = (4 * HBUTTON) + (3 * GAP);
	const int YG3_1B1 = YG3_1 + (HG3_1 / 2) - (HG3_1B / 2);
	const int YG3_1B2 = YG3_1B1 + HBUTTON + GAP;
	const int YG3_1B3 = YG3_1B2 + HBUTTON + GAP;
	const int YG3_1B4 = YG3_1B3 + HBUTTON + GAP;
	const int XG3_1A = XG3 + GAP;
	const int WG3_1A = XG3_1B - GAP - XG3_1A;
	const int XG3_1C = XG3_1B + WG3_1B + GAP;
	const int WG3_1C = WG3 - GAP - XG3_1C;
	char label3[128];
	sprintf(label3, "Columns for field set %s", field_set_name_.c_str());
	Fl_Group* gp3 = new Fl_Group(XG3, YG3, WG3, HG3);
	gp3->copy_label(label3);
	gp3->labelsize(FL_NORMAL_SIZE + 2);
	gp3->labelfont(FL_BOLD);
	gp3->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	gp3->box(FL_BORDER_BOX);
	// The used field table
	fields_table* tab3_1a = new fields_table(XG3_1A, YG3_1, WG3_1A, HG3_1, "Columns in field set");
	tab3_1a->align(FL_ALIGN_TOP);
	tab3_1a->callback(cb_tab_inuse);
	tab3_1a->when(FL_WHEN_RELEASE);
	tab3_1a->tooltip("Displays the fields used in the current selected application");
	used_table_ = tab3_1a;

	// Button - move unused field to used list
	Fl_Button* bn3_1b1 = new Fl_Button(XG3_1B, YG3_1B1, WG3_1B, HBUTTON, "@<-");
	bn3_1b1->tooltip("Move the selected field from the unused to the used list");
	bn3_1b1->callback(cb_bn_use);
	bn3_1b1->when(FL_WHEN_RELEASE);
	use_button_ = bn3_1b1;
	gp3->add(bn3_1b1);
	// Button - move used field to unused list
	Fl_Button* bn3_1b2 = new Fl_Button(XG3_1B, YG3_1B2, WG3_1B, HBUTTON, "@->");
	bn3_1b2->tooltip("Move the selected field from the used to the unused list");
	bn3_1b2->callback(cb_bn_disuse);
	bn3_1b2->when(FL_WHEN_RELEASE);
	disuse_button_ = bn3_1b2;
	gp3->add(bn3_1b2);
	// Button - move field up the used list
	Fl_Button* bn3_1b3 = new Fl_Button(XG3_1B, YG3_1B3, WG3_1B, HBUTTON, "@8->");
	bn3_1b3->tooltip("Move the selected field up one position in the used list");
	bn3_1b3->callback(cb_bn_up);
	bn3_1b3->when(FL_WHEN_RELEASE);
	up_button_ = bn3_1b3;
	gp3->add(bn3_1b3);
	// Button - move field down the used list
	Fl_Button* bn3_1b4 = new Fl_Button(XG3_1B, YG3_1B4, WG3_1B, HBUTTON, "@2->");
	bn3_1b4->tooltip("Move the selected field down one position in the used list");
	bn3_1b4->callback(cb_bn_down);
	bn3_1b4->when(FL_WHEN_RELEASE);
	down_button_ = bn3_1b4;
	gp3->add(bn3_1b4);

	// The unused list
	fields_table* tab3_1c = new fields_table(XG3_1C, YG3_1, WG3_1C, HG3_1, "Available fields");
	tab3_1c->align(FL_ALIGN_TOP);
	tab3_1c->callback(cb_tab_avail);
	tab3_1c->when(FL_WHEN_RELEASE);
	tab3_1c->tooltip("Displays the fields not used in the current application");
	avail_table_ = tab3_1c;
	gp3->add(tab3_1c);

	// Input - header text for the selected field
	intl_input* ip3_2a = new intl_input(XG3_2A, YG3_2, WG3_2, HG3_2, "Header");
	ip3_2a->align(FL_ALIGN_LEFT);
	ip3_2a->callback(cb_ip_header);
	ip3_2a->when(FL_WHEN_CHANGED);
	ip3_2a->tooltip("Type in the new value for the header of the selected field");
	header_input_ = ip3_2a;
	gp3->add(ip3_2a);
	// Input - width in pixels for the selected field
	Fl_Int_Input* ip3_2b = new Fl_Int_Input(XG3_2B, YG3_2, WG3_2, HG3_2, "Width");
	ip3_2b->align(FL_ALIGN_LEFT);
	ip3_2b->callback(cb_ip_width);
	ip3_2b->when(FL_WHEN_CHANGED);
	ip3_2b->tooltip("Type in the new value for the width of the selected field");
	width_input_ = ip3_2b;
	gp3->add(ip3_2b);
	table_group_ = gp3;

	gp3->end();

	Fl_Group::end();
	show();

	update_widgets();
}

// Used to write settings back
void fields_dialog::save_values() {
	// Read the field data from the registry
	Fl_Preferences display_settings(settings_, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	// Delete current settings
	fields_settings.clear();
	// First store the default application
	fields_settings.set("Active Application", application_);
	// Then get the field sets for the applications
	char app_path[128];
	for (int i = 0; i < FO_LAST; i++) {
		sprintf(app_path, "App%d", i);
		fields_settings.set(app_path, field_set_by_app_[(field_ordering_t)i].c_str());
	}
	// For each field set
	auto it = field_sets_.begin();
	for (int i = 0; it != field_sets_.end(); i++, it++) {
		int num_fields = it->second->size();
		Fl_Preferences colln_settings(fields_settings, it->first.c_str());
		// For each field in the set
		for (int j = 0; j < num_fields; j++) {
			char field_id[10];
			sprintf(field_id, "Field %d", j);
			Fl_Preferences field_settings(colln_settings, field_id);
			field_info_t field = (it->second)->at(j);
			field_settings.set("Name", field.field.c_str());
			field_settings.set("Width", (int)field.width);
			field_settings.set("Header", field.header.c_str());
		}
	}

	// Tell the views that formatting has changed
	book_->selection(-1, HT_FORMAT);
}

// Used to enable/disable specific widget - any widgets enabled must be attributes
void fields_dialog::enable_widgets() {}

// Get the fields that are not in the selected field set
void fields_dialog::unused_fields() {
	// Get all fields in the ADIF database
	set<string>* all_fields = spec_data_->sorted_fieldnames();
	set<string>* used_fields = new set<string>;
	vector<field_info_t>* field_set = field_sets_[field_set_name_];
	// get all the fields in the set
	for (auto it = field_set->begin(); it != field_set->end(); it++) {
		used_fields->insert(it->field);
	}
	// Clear any existing information
	unused_fields_.clear();
	// For each field in the ADIF database
	for (auto it = all_fields->begin(); it != all_fields->end(); it++) {
		// If the field is not in the selected set
		if (used_fields->find((*it)) == used_fields->end()) {
			// Add it to the unused set, with width = 5 and heading = field name
			field_info_t col_info = { (*it), (*it), 50 };
			unused_fields_.push_back(col_info);
		}
	}
	// Release menory
	used_fields->clear();
	delete used_fields;
}

// Update widgets based on current attribute
void fields_dialog::update_widgets(bool update_name /* = true */) {
	// Default the field set to display by the active application
	if (update_name) {
		// Selected field set in that for the current application - display it in the set choice widget
		field_set_name_ = field_set_by_app_[application_];
		int index = ((Fl_Choice*)name_choice_)->find_index(field_set_name_.c_str());
		((Fl_Choice*)name_choice_)->value(index);
	}
	// Select the applications that use this field_set
	for (int i = 0; i < FO_LAST; i++) {
		if (field_set_by_app_[(field_ordering_t)i] == field_set_name_) {
			((Fl_Light_Button*)app_buttons_[i])->value(true);
		} else {
			((Fl_Light_Button*)app_buttons_[i])->value(false);
		}
	}
	// Set the app choice
	((Fl_Choice*)app_choice_)->value((int)application_);
	// Get the list of unused fields and copy field set and unused set to tables
	unused_fields();
	((fields_table*)used_table_)->fields(field_sets_[field_set_name_]);
	((fields_table*)avail_table_)->fields(&unused_fields_);
	// Update labels
	char label[128];
	sprintf(label, "Current applications using %s", field_set_name_.c_str());
	app_group_->copy_label(label);
	sprintf(label, "Columns for field set %s", field_set_name_.c_str());
	table_group_->copy_label(label);

}

// Callbacks
// Select field set choice
// Make the named field set the selected one.
void fields_dialog::cb_ch_sel_col(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	that->field_set_name_ = ch->text();
	// Set default application
	bool found = false;
	for (int i = 0; i < FO_LAST && !found; i++) {
		if (that->field_set_name_ == that->field_set_by_app_[(field_ordering_t)i]) {
			that->application_ = (field_ordering_t)i;
			found = true;
		}
	}
	that->update_widgets(false);
}

// Select app choice
// Make the named field set that used by this application
void fields_dialog::cb_ch_sel_app(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	string app_name;
	cb_choice_text(w, &app_name);
	for (int i = 0; i < FO_LAST; i++) {
		if (that->APP_NAMES[i] == app_name) {
			that->application_ = (field_ordering_t)i;
		}
	}
	that->update_widgets(true);
}

// Create new field set from default fields
// Create a new field set with the default fields and make it selected field set
void fields_dialog::cb_bn_new(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	intl_input* ip = (intl_input*)that->name_input_;
	const char * new_name = ip->value();
	// If the name is not empty
	if (new_name[0] != 0) {
		that->field_set_name_ = string(new_name);
		// Create a default field set
		vector<field_info_t>* field_set = new vector<field_info_t>;
		for (int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
			field_set->push_back(DEFAULT_FIELDS[i]);
		}
		// Name it
		that->field_sets_[string(new_name)] = field_set;
		// Update tables etc.
		that->update_widgets(false);
		// Add field set name to name choice widget and select it
		int value = ((Fl_Choice*)that->name_choice_)->add(new_name);
		((Fl_Choice*)that->name_choice_)->value(value);
	}
}

// Save field set to use in selected application
void fields_dialog::cb_bn_use_app(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	// Change the field set used by selected application to the selected field set
	field_ordering_t app = (field_ordering_t)(long)v;
	that->field_set_by_app_[app] = that->field_set_name_;
	that->update_widgets();
}

// left-hand table clicked (fields in use)
// Enable width and header to be edited (or not)
void fields_dialog::cb_tab_inuse(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	fields_table* table = (fields_table*)w;
	// Get a reference to the field set being displayed
	vector<field_info_t>* field_set = that->field_sets_[that->field_set_name_];
	char temp[128];
	int row;
	field_info_t* field;
	switch (table->callback_context()) {
	case Fl_Table::CONTEXT_CELL:
		// Get the selected row
		row = table->callback_row();
		// Field selected at the row
		field = &(*field_set)[row];
		switch (table->callback_col()) {
		case 1:
			// width clicked - copy it to  width input and enable it
			sprintf(temp, "%d", field->width);
			((Fl_Int_Input*)that->width_input_)->value(temp);
			that->width_input_->activate();
			that->header_input_->deactivate();
			break;
		case 2:
			// header clicked - copy it to header input and enable it
			((intl_input*)that->header_input_)->value(field->header.c_str());
			that->width_input_->deactivate();
			that->header_input_->activate();
			break;
		default:
			// somewhere else clicked - deactivate both width and header inputs
			that->width_input_->deactivate();
			that->header_input_->deactivate();
			break;
		}
		break;
	default:
		// somewhere else clicked - deactivate both width and header inputs
		that->width_input_->deactivate();
		that->header_input_->deactivate();
		break;
	}
}

// right table clicked (fields not in use)
// Doesn't actually do anything
void fields_dialog::cb_tab_avail(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	fields_table* table = (fields_table*)w;
	int row;
	field_info_t* field;
	switch (table->callback_context()) {
	case Fl_Table::CONTEXT_CELL:
		// Get the selected row
		row = table->callback_row();
		field = &(that->unused_fields_)[row];
		break;
	}
}

// move field from available to use
void fields_dialog::cb_bn_use(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	// Get reference to field set in use
	vector<field_info_t>* field_set = that->field_sets_[that->field_set_name_];
	// Get the active row in the unused table
	int row = ((fields_table*)that->avail_table_)->row();
	// Add the field to the current field set
	field_info_t field = that->unused_fields_[row];
	field_set->push_back(field);
	// Update with the altered field set
	that->update_widgets();
	((fields_table*)that->used_table_)->select_row(field_set->size() - 1);
}

// move field from use to available
void fields_dialog::cb_bn_disuse(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	// Get reference to field set in use
	vector<field_info_t>* field_set = that->field_sets_[that->field_set_name_];
	// Remove the field at the active row from the current field set
	int row = ((fields_table*)that->used_table_)->row();
	field_set->erase(field_set->begin() + row);
	// Update with  the altered field set
	that->update_widgets();
}

// move field up in use list
void fields_dialog::cb_bn_up(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	// Reference to active field set
	vector<field_info_t>* field_set = that->field_sets_[that->field_set_name_];
	fields_table* table = (fields_table*)that->used_table_;
	int row = table->row();
	// If first row is not alreadu selected
	if (row > 0) {
		// Copy field info
		field_info_t field = (*field_set)[row];
		// Delete from the field set
		field_set->erase(field_set->begin() + row);
		// Insert the copy in the previous position in the set
		row--;
		field_set->insert(field_set->begin() + row, field);
		field = (*field_set)[row];
		table->select_row(row);
		// Redraw table
		that->used_table_->redraw();
	}
}

// move field down in use list
void fields_dialog::cb_bn_down(Fl_Widget* w, void * v) 
{
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	// Reference to active field set
	vector<field_info_t>* field_set = that->field_sets_[that->field_set_name_];
	fields_table* table = (fields_table*)that->used_table_;
	int row = table->row();
	// If the active row is not the last row
	if ((unsigned)row < field_set->size()) {
		// Copy field info
		field_info_t field = (*field_set)[row];
		// Delete from the set
		field_set->erase(field_set->begin() + row);
		// Insert the copy in the next position in the set
		row++;
		field_set->insert(field_set->begin() + row, field);
		field = (*field_set)[row];
		table->select_row(row);
		// Redraw table
		that->used_table_->redraw();
	}
}

// header input changed
void fields_dialog::cb_ip_header(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	intl_input* ip = (intl_input*)w;
	// Reference to active set
	vector<field_info_t>* field_set = that->field_sets_[that->field_set_name_];
	// Get the field at the active row in the table
	int row = ((fields_table*)that->used_table_)->row();
	field_info_t* field = &(*field_set)[row];
	// Set its header value
	field->header = ip->value();
	that->used_table_->redraw();
}

// width input changed
void fields_dialog::cb_ip_width(Fl_Widget* w, void* v) {
	fields_dialog* that = ancestor_view<fields_dialog>(w);
	Fl_Int_Input* ip = (Fl_Int_Input*)w;
	// Reference to active set
	vector<field_info_t>* field_set = that->field_sets_[that->field_set_name_];
	// Get the field at the active row in the table
	int row = ((fields_table*)that->used_table_)->row();
	field_info_t* field = &(*field_set)[row];
	// Set its width value
	try {
		field->width = stoi(ip->value());
	}
	catch (invalid_argument&) {
		// Do nothing
	}
	that->used_table_->redraw();
}
