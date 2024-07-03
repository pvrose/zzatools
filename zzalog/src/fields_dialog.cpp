#include "fields_dialog.h"
#include "drawing.h"
#include "field_choice.h"
#include "intl_widgets.h"
#include "utils.h"
#include "callback.h"
#include "book.h"

#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Button.H>

extern Fl_Preferences* settings_;
extern book* book_;

// Constructor
fields_table::fields_table(int X, int Y, int W, int H, const char* L) :
    Fl_Table_Row(X, Y, W, H, L) 
{
    col_header(1);
    col_resize(0);
    col_header_height(fl_height() + 2);
    row_header(0);
    row_resize(0);
    cols(3);
    col_width_all((W - Fl::scrollbar_size()) / 3);
    // Create the editing widgets
    ch_field_ = new field_choice(X + W/2, Y + H/2, 10, 10);
    ch_field_->set_dataset("Fields");
    ch_field_->callback(cb_field);
    ch_field_->hide();

    ip_header_ = new intl_input(X + W/2, Y + H/2, 10, 10);
    ip_header_->callback(cb_header);
    ip_header_->when(FL_WHEN_ENTER_KEY);
    ip_header_->hide();

    ip_width_ = new Fl_Int_Input(X + W/2, Y + H/2, 10, 10);
    ip_width_->callback(cb_width);
    ip_width_->when(FL_WHEN_ENTER_KEY);
    ip_width_->hide();

    edit_row_ = -1;
    edit_col_ = -1;

    callback(cb_table);
    end();
}

// Destructor
fields_table::~fields_table() {
}

// Draw the table - but not the widgets
void fields_table::draw_cell(TableContext context, int R, int C, int X, int Y,
    int W, int H)
{
    switch(context) {
        case CONTEXT_STARTPAGE: {
            fl_font(0, FL_NORMAL_SIZE);
            return;
        }
        case CONTEXT_COL_HEADER: {
            fl_push_clip(X, Y, W, H);
			fl_color(col_header_color());
			fl_rectf(X, Y, W, H);
			fl_color(FL_FOREGROUND_COLOR);
			fl_yxline(X, Y, Y + H - 1, X + W);
            

            switch(C) {
                case 0: {
                    fl_draw("Field", X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                }
                case 1: {
                    fl_draw("Header", X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                }
                case 2: {
                    fl_draw("Width", X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                }
            }
            fl_pop_clip();
            return;
        }
        case CONTEXT_CELL: {
            if (edit_row_ != R || edit_col_ != C) {
                fl_push_clip(X, Y, W, H);
                Fl_Color bg_colour = row_selected(R) ? FL_FOREGROUND_COLOR : FL_BACKGROUND_COLOR;
                Fl_Color fg_colour = row_selected(R) ? FL_BACKGROUND_COLOR : FL_FOREGROUND_COLOR;
                fl_color(bg_colour);
                fl_rectf(X, Y, W, H);
                fl_color(fg_colour);
                fl_yxline(X, Y, Y + H - 1, X + W);
                if (R < data_->size()) {
                    field_info_t& info = (*data_)[R];

                    switch(C) {
                        case 0: {
                            fl_draw(info.field.c_str(), X + 2, Y, W - 4, H, FL_ALIGN_LEFT);
                            break;
                        }
                        case 1: {
                            fl_draw(info.header.c_str(), X + 2, Y, W - 4, H, FL_ALIGN_LEFT);
                            break;
                        }
                        case 2: {
                            char t[10];
                            snprintf(t, sizeof(t), "%d", info.width);
                            fl_draw(t, X + 2, Y, W - 4, H, FL_ALIGN_RIGHT);
                            break;
                        }
                    }
                }
               fl_pop_clip();
            }
            return;
        }
    }
}

// Set the data
void fields_table::data(vector<field_info_t>* d) {
    data_ = d;
    rows(data_->size() + 1);
    row_height_all(fl_height() + 2);
    redraw();
}

// Click on the table
void fields_table::cb_table(Fl_Widget* w, void* v) {
    // Get the row and column
    fields_table* that = (fields_table*)w;
    int row = that->callback_row();
    int col = that->callback_col();
    // Get the position of the cell
    int X, Y, W, H;
    that->find_cell(CONTEXT_TABLE, row, col, X, Y, W, H);
    switch(that->callback_context()) {
        case CONTEXT_CELL: {
            switch (Fl::event_button()) {
                case FL_LEFT_MOUSE: {
                    // Hide any existing editor
                    that->ch_field_->hide();
                    that->ip_header_->hide();
                    that->ip_width_->hide();
                    that->edit_col_ = -1;
                    that->edit_row_ = -1;
                    if (Fl::event_clicks()) {
                        // Double clich left button - open edit widget
                        // get the data
                        if (row == that->data_->size()) {
                            field_info_t* newi = new field_info_t;
                            that->data_->push_back(*newi);
                        }
                        field_info_t& info = (*that->data_)[row];
                        // Which column
                        Fl_Widget* edit;
                        switch(col) {
                            case 0: {
                                edit = that->ch_field_;
                                that->ch_field_->value(info.field.c_str());
                                break;
                            }
                            case 1: {
                                edit = that->ip_header_;
                                that->ip_header_->value(info.header.c_str());
                                break;
                            }
                            case 2: {
                                edit = that->ip_width_;
                                char t[10];
                                snprintf(t, sizeof(t), "%d", info.width);
                                that->ip_width_->value(t);
                                break;
                            }
                        }
                        edit->resize(X, Y, W, H);
                        edit->user_data((void*)(intptr_t)row);
                        edit->show();
                        that->edit_row_ = row;
                        that->edit_col_ = col;
                    } else {
                        that->select_row(row);
                        that->selected_row_ = row;
                    }
                    ancestor_view<fields_dialog>(that)->enable_widgets();
                    that->damage(FL_DAMAGE_ALL);
                    that->redraw();
                }
            }
        }
    }
}

// Callback from field_choice
void fields_table::cb_field(Fl_Widget* w, void* v) {
    fields_table* that = ancestor_view<fields_table>(w);
    int row = (int)(intptr_t)v;
    field_choice* ch = (field_choice*)w;
    if (row == that->data_->size()) {
        // Adding a record - defaul header and width
        field_info_t newf = field_info_t();;
        newf.field = ch->value();
        newf.header = ch->value();
        newf.width = 50;
        that->data_->push_back(newf);
    } else if (strlen(ch->value()) == 0) {
        // Deleting a record
        auto it = that->data_->begin() + row;
        that->data_->erase(it);
    } else {
        // Renaming a record - default header
        field_info_t* info = &(*that->data_)[row];
        info->field = ch->value();
        info->header = ch->value();
    }
    w->hide();
    that->edit_row_ = -1;
    that->edit_col_ = -1;
    // Call the table's callback
    that->select_row(row);
    that->redraw();
}

// Callback from the header input
void fields_table::cb_header(Fl_Widget* w, void* v) {
    fields_table* that = ancestor_view<fields_table>(w);
    int row = (int)(intptr_t)v;
    string header;
    cb_value<intl_input, string>(w, &header);
    (*that->data_)[row].header = header;
    w->hide();
    that->edit_row_ = -1;
    that->edit_col_ = -1;
    // Call the table's callback
    that->select_row(row);
    that->redraw();
}


// Callback from the width input
void fields_table::cb_width(Fl_Widget* w, void* v) {
    fields_table* that = ancestor_view<fields_table>(w);
    int row = (int)(intptr_t)v;
    int width;
    cb_value_int<intl_input>(w, &width);
    (*that->data_)[row].width = width;
    w->hide();
    that->edit_row_ = -1;
    that->edit_col_ = -1;
    // Call the table's callback
    that->select_row(row);
    that->redraw();
}

// Return selected row
int fields_table::selected_row() {
    return selected_row_;
}

// Set selected row
void fields_table::select_row(int row) {
    Fl_Table_Row::select_row(selected_row_, false);
    Fl_Table_Row::select_row(row, true);
    selected_row_ = row;
}

// fields_dialog constructor
fields_dialog::fields_dialog(int X, int Y, int W, int H, const char* L) :
    page_dialog(X, Y, W, H, L) 
{
    ch_app_ = nullptr;
    ch_coll_ = nullptr;
    table_ = nullptr;
    application_ = FO_MAINLOG;
    collection_ = "";
    app_map_.clear();
    coll_map_.clear();
    // use page_dialog default
    do_creation(X, Y);
    enable_widgets();
}

// Destructor
fields_dialog::~fields_dialog() {}

// Load data from settings
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
		app_map_[(field_app_t)i] = string(temp);
		free(temp);
	}
	// Default the field set to display that for the active application
	collection_ = app_map_[application_];
    linked_ = true;
	// Delete the existing field data
	for (auto it = app_map_.begin(); it != app_map_.end(); it++) {
        if (coll_map_.find(it->second) != coll_map_.end()) {
            delete coll_map_.at(it->second);
        }
    }
	coll_map_.clear();
	// Read all the field field sets
	if (num_field_sets == 0) {
		// No field sets in setting, initialise a new one with default field set
		vector<field_info_t>* field_set = default_collection();
 		coll_map_["Default"] = field_set;
    } else {
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
			coll_map_[colln_name] = field_set;
		}
	}
}

void fields_dialog::create_form(int X, int Y) {
    begin();

    int curr_x = X + GAP;
    int curr_y = Y + GAP + HTEXT;

    // Application choice
    Fl_Choice* w101 = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Application");
    w101->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
    w101->callback(cb_application, nullptr);
    w101->tooltip("Select the application to edit");
    populate_app(w101);
    ch_app_ = w101;

    curr_x += w101->w() + GAP;

    // Collection choice
    Fl_Input_Choice* w102 = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Collection");
    w102->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
    w102->callback(cb_collection, nullptr);
    w102->tooltip("Select the collection to edit");
    populate_coll(w102);
    ch_coll_ = w102;

    curr_x += w102->w() + GAP;
    Fl_Button* w103 = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Linked");
    w103->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    w103->callback(cb_value<Fl_Light_Button, bool>, &linked_);
    w103->tooltip("Changes to application and collection values are linked");
    w103->value(linked_);

    curr_x = X + GAP;
    curr_y += HBUTTON + GAP + HTEXT;
    int curr_w = w102->x() + w102->w() - w101->x();

    fields_table* w200 = new fields_table(curr_x, curr_y, curr_w, 300, "Fields");
    w200->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
    w200->tooltip("Table defining the fields, and their headers and widths in the current collection");

    table_ = w200;

    curr_y += w200->h() + GAP;

    curr_x = X + GAP;

    Fl_Button* w301 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Up");
    w301->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    w301->callback(cb_move, (void*)(intptr_t)true);
    w301->tooltip("Move the selected field up the list");
    bn_up_ = w301;

    curr_x += w301->w() + GAP;
    Fl_Button* w302 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Down");
    w302->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    w302->callback(cb_move, (void*)(intptr_t)false);
    w302->tooltip("Move the selected field down the list");
    bn_down_ = w302;

    curr_x += w302->w() + GAP;
    Fl_Button* w303 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Restore");
    w303->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    w303->callback(cb_default, nullptr);
    w303->tooltip("Restore \"Default\" collection");
    bn_restore_ = w303;

    end();

}

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
		fields_settings.set(app_path, app_map_[(field_app_t)i].c_str());
	}
	settings_->flush();
	// For each field set
	auto it = coll_map_.begin();
	for (int i = 0; it != coll_map_.end(); i++, it++) {
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

// Enable widgets - update them per application 
void fields_dialog::enable_widgets() {
    // Update displayed data dependant on application
    ch_app_->value((int)application_);
    ch_coll_->value(collection_.c_str());
    table_->data(coll_map_[collection_]);
    // Inhibit up or down buttons
    if (table_->selected_row() == 0) {
        bn_up_->deactivate();
    } else {
        bn_up_->activate();
    }
    if (table_->selected_row() == coll_map_[collection_]->size() - 1) {
        bn_down_->deactivate();
    } else {
        bn_down_->activate();
    }
    if (collection_ == "Default") {
        bn_restore_->activate();
    } else {
        bn_restore_->deactivate();
    }
}

// Application choice callback
// v is not used
void fields_dialog::cb_application(Fl_Widget* w, void* v) {
    Fl_Choice* ch = (Fl_Choice*)w;
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    that->application_ = (field_app_t)ch->value();
    if (that->linked_) {
        that->collection_ = that->app_map_[that->application_];
    }
    that->enable_widgets();
}

// Collection choice callback
// v is not used
void fields_dialog::cb_collection(Fl_Widget* w, void* v) {
    Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    that->collection_ = ch->value();
    if (ch->menubutton()->changed() == 0) {
        // New collection - initialise as copy of "Default"
        vector<field_info_t>* newc = new vector<field_info_t>;
        *newc = *(that->coll_map_.at("Default"));
        that->coll_map_[that->collection_] = newc;
    }
    // Change application to this collection
    if (that->linked_) {
        that->app_map_[that->application_] = that->collection_;
    }
    that->enable_widgets();
}

// Up/Don button callback
// v: true = up; false = down
void fields_dialog::cb_move(Fl_Widget* w, void* v) {
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    bool up = (bool)(intptr_t)v;
    that->navigate_table(up);
}

// Restore default collection
void fields_dialog::cb_default(Fl_Widget* w, void* v) {
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    delete that->coll_map_.at("Default");
    that->coll_map_["Default"] = that->default_collection();
    that->table_->data(that->coll_map_.at("Default"));
}

// Implement the up/down
void fields_dialog::navigate_table(bool up) {
    vector<field_info_t>* coll = coll_map_.at(collection_);
    int row = table_->selected_row();
    field_info_t data = (*coll)[row];
    auto selected = coll->begin() + row;
    auto next = coll->erase(selected);
    auto target = up ? selected - 1 : next + 1;
    coll->insert(target, data);
    table_->select_row(up ? row -1 : row + 1);
    table_->redraw();
    enable_widgets();
}

// Populate the application choice
void fields_dialog::populate_app(Fl_Choice* ch) {
    ch->clear();
    for (auto it = APPLICATION_LABELS.begin(); it != APPLICATION_LABELS.end(); it++) {
        ch->add(it->second.c_str());
    }
}

// :Populate the collection choice
void fields_dialog::populate_coll(Fl_Input_Choice* ch) {
    ch->clear();
    for (auto it = coll_map_.begin(); it != coll_map_.end(); it++) {
        ch->add(it->first.c_str());
    }
}

// Create default collection
vector<field_info_t>* fields_dialog::default_collection() {
    vector<field_info_t>* result = new vector<field_info_t>;
    // For all fields in default field set add it to the new field set
    for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
        result->push_back(DEFAULT_FIELDS[i]);
    }
    return result;
}

