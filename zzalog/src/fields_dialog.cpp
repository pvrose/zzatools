#include "fields_dialog.h"
#include "drawing.h"
#include "field_choice.h"
#include "intl_widgets.h"
#include "utils.h"
#include "callback.h"
#include "book.h"
#include "fields.h"

#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>

extern book* book_;
extern fields* fields_;

// Constructor
fields_table::fields_table(int X, int Y, int W, int H, const char* L) :
    Fl_Table_Row(X, Y, W, H, L),
    data_(nullptr),
    selected_row_(0)
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
        default:
            break;
    }
}

// Set the data
void fields_table::data(collection_t* d) {
    data_ = d;
    rows(data_->size() + 1);
    // Sometimes it seems we enter with a different font than expected
    Fl_Font save_font = fl_font();
    Fl_Fontsize save_size = fl_size();
    fl_font(0, FL_NORMAL_SIZE);
    row_height_all(fl_height() + 2);
    fl_font(save_font, save_size);
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
                        Fl_Widget* edit = nullptr;
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
                        if (edit) {
                            edit->resize(X, Y, W, H);
                            edit->user_data((void*)(intptr_t)row);
                            edit->show();
                        }
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
        default:
            break;
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
        info->width = 50;
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
    collection_ = fields_->coll_name(application_);
    linked_ = true;
    // use page_dialog default
    do_creation(X, Y);
    enable_widgets();
}

// Destructor
fields_dialog::~fields_dialog() {}

// Load data from settings
void fields_dialog::load_values() {
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
    w102->input()->when(FL_WHEN_ENTER_KEY);
    populate_coll(w102);
    ch_coll_ = w102;

    curr_x += w102->w() + GAP;
    Fl_Light_Button* w103 = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Linked");
    w103->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    w103->callback(cb_linked, &linked_);
    w103->tooltip("Changes to application and collection values are linked");
    w103->value(linked_);
    bn_linked_ = w103;

    curr_y += HBUTTON;
    Fl_Button* w113 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Delete");
    w113->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    w113->callback(cb_del_coll, nullptr);
    w113->tooltip("Delete the displayed collection");

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

    end();

}

void fields_dialog::save_values() {
    book_->selection(-1, HT_FORMAT);
}

// Enable widgets - update them per application 
void fields_dialog::enable_widgets() {
    // Update displayed data dependant on application
    ch_app_->value((int)application_);
    ch_coll_->value(collection_.c_str());
    ch_coll_->update_menubutton();
    collection_t* coll = fields_->collection(collection_);
    table_->data(coll);
    // Inhibit up or down buttons
    if (table_->selected_row() == 0) {
        bn_up_->deactivate();
    } else {
        bn_up_->activate();
    }
    if (table_->selected_row() == coll->size() - 1) {
        bn_down_->deactivate();
    } else {
        bn_down_->activate();
    }
    bn_linked_->value(linked_);
    char title[50];
    snprintf(title, sizeof(title), "Fields - %s", collection_.c_str());
    table_->copy_label(title);
    redraw();
}

// Application choice callback
// v is not used
void fields_dialog::cb_application(Fl_Widget* w, void* v) {
    Fl_Choice* ch = (Fl_Choice*)w;
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    // Set the application, get the associated collection and link them
    that->application_ = (field_app_t)ch->value();
    that->collection_ = fields_->coll_name(that->application_);
    that->linked_ = true;
    that->enable_widgets();
}

// Collection choice callback
// v is not used
void fields_dialog::cb_collection(Fl_Widget* w, void* v) {
    Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    if (ch->menubutton()->changed() == 0) {
        // New collection - initialise as copy of "Default"
        that->collection_ = ch->value();
        (void)fields_->collection(that->collection_, "Default");
    }
    else {
        char* temp = new char[32];
        ch->menubutton()->item_pathname(temp, 32);
        if (*temp == '/') that->collection_ = temp + 1;
        else that->collection_ = temp;
    }
    // Delete linkage
    that->linked_ = false;
    // Redraw 
    that->enable_widgets();
}

// Up/Don button callback
// v: true = up; false = down
void fields_dialog::cb_move(Fl_Widget* w, void* v) {
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    bool up = (bool)(intptr_t)v;
    that->navigate_table(up);
}

// Delete the displayed collection
void fields_dialog::cb_del_coll(Fl_Widget* w, void* v) {
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    if (fields_->delete_coll(that->collection_)) {
        that->collection_ = fields_->coll_name(that->application_);
        that->enable_widgets();
        that->populate_coll(that->ch_coll_);
    }
}

// Link the application
void fields_dialog::cb_linked(Fl_Widget* w, void* v) {
    // Gete the new value of the button
    cb_value<Fl_Light_Button, bool>(w, v);
    fields_dialog* that = ancestor_view<fields_dialog>(w);
    if (that->linked_) {
        fields_->link_app(that->application_, that->collection_);
    }
    that->enable_widgets();
   
}

// Implement the up/down
void fields_dialog::navigate_table(bool up) {
    collection_t* coll = fields_->collection(collection_);
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
    set<string> names = fields_->coll_names();
    for (auto it = names.begin(); it != names.end(); it++) {
        ch->add((*it).c_str());
    }
}


