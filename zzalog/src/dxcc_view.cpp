#include "dxcc_view.h"

#include "dxcc_table.h"
#include "utils.h"

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Preferences.H>

extern Fl_Preferences* settings_;

dxcc_view::dxcc_view(int X, int Y, int W, int H, const char* L, field_app_t fo) :
	view(),
	Fl_Group(X, Y, W, H, L)
{
    load_data();
	create_form();
	enable_widgets();
}

dxcc_view::~dxcc_view() {
    store_data();
}

void dxcc_view::create_form() {
    int h_table = h() - GAP - HBUTTON - GAP;
    table_ = new dxcc_table(x(), y(), w(), h_table);
    table_->display_type(display_type_);
    table_->confirm_type(confirm_type_);

    int curr_y = y() + h_table + GAP;
    int curr_x = x() + GAP;
    rg_display_ = new Fl_Group(curr_x, curr_y, WBUTTON * 4, HBUTTON);
    rg_display_->box(FL_FLAT_BOX);

    bn_total_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Total");
    bn_total_->type(FL_RADIO_BUTTON);
    bn_total_->value(display_type_ == dxcc_table::TOTAL);
    bn_total_->callback(cb_display, (void*)(intptr_t)dxcc_table::TOTAL);
    bn_total_->tooltip("Displays a count of all QSOs worked for that entity");

    curr_x += WBUTTON;
    bn_bands_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "By Band");
    bn_bands_->type(FL_RADIO_BUTTON);
    bn_bands_->value(display_type_ == dxcc_table::BANDS);
    bn_bands_->callback(cb_display, (void*)(intptr_t)dxcc_table::BANDS);
    bn_bands_->tooltip("Displays a count of all QSOs worked for that entity on each band");

    curr_x += WBUTTON;
    bn_modes_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "By Mode");
    bn_modes_->type(FL_RADIO_BUTTON);
    bn_modes_->value(display_type_ == dxcc_table::MODES);
    bn_modes_->callback(cb_display, (void*)(intptr_t)dxcc_table::MODES);
    bn_modes_->tooltip("Displays a count of all QSOs worked for that entity on each mode");

    curr_x += WBUTTON;
    bn_dmodes_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "By Class");
    bn_dmodes_->type(FL_RADIO_BUTTON);
    bn_dmodes_->value(display_type_ == dxcc_table::DXCC_MODES);
    bn_dmodes_->callback(cb_display, (void*)(intptr_t)dxcc_table::DXCC_MODES);
    bn_dmodes_->tooltip("Displays a count of all QSOs worked for that entity on that class of modes");

    rg_display_->end();
    rg_display_->show();
    curr_x += WBUTTON + GAP;

    bg_confirm_ = new Fl_Group(curr_x, curr_y, WBUTTON * 3, HBUTTON);

    bn_eqsl_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "eQSL");
    bn_eqsl_->value(confirm_type_ & dxcc_table::EQSL);
    bn_eqsl_->callback(cb_confirm, (void*)(intptr_t)dxcc_table::EQSL);
    bn_eqsl_->tooltip("Counts QSOs confirmed by eQSL");

    curr_x += WBUTTON;
    bn_lotw_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "LotW");
    bn_lotw_->value(confirm_type_ & dxcc_table::LOTW);
    bn_lotw_->callback(cb_confirm, (void*)(intptr_t)dxcc_table::LOTW);
    bn_lotw_->tooltip("Counts QSOs confirmed by LotW");

    curr_x += WBUTTON;
    bn_card_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Card");
    bn_card_->value(confirm_type_ & dxcc_table::CARD);
    bn_card_->callback(cb_confirm, (void*)(intptr_t)dxcc_table::CARD);
    bn_card_->tooltip("Counts QSOs confirmed by card");

    bg_confirm_->end();
    bg_confirm_->show();
    curr_x += WBUTTON + GAP;

    min_w_ = curr_x - x();
    min_h_ = table_->min_h() + GAP + HBUTTON + GAP;

    end();
    resizable(table_);
    show();
}

// Don't think there's anything to do
void dxcc_view::enable_widgets() {}

// Display choice
void dxcc_view::cb_display(Fl_Widget* w, void* v) {
    dxcc_view* that = ancestor_view<dxcc_view>(w);
    that->display_type_ = (dxcc_table::display_t)(intptr_t)v;
    that->table_->display_type(that->display_type_);
}

// Confirmation choice
void dxcc_view::cb_confirm(Fl_Widget* w, void* v) {
    dxcc_view* that = ancestor_view<dxcc_view>(w);
    Fl_Check_Button* bn = (Fl_Check_Button*)w;
    if (bn->value()) {
        (char&)that->confirm_type_ |= (char)(intptr_t)v;
    }
    else {
        (char&)that->confirm_type_ &= ~(char)(intptr_t)v;
    }
    that->table_->confirm_type(that->confirm_type_);
}

// Get previously saved values for display and confirmation types
void dxcc_view::load_data() {
    Fl_Preferences user_settings(settings_, "User Settings");
    Fl_Preferences dxcc_settings(user_settings, "DXCC Table");
    int temp;
    dxcc_settings.get("Display Type", temp, (int)dxcc_table::BANDS);
    display_type_ = (dxcc_table::display_t)temp;
    dxcc_settings.get("Confirmation", temp, (int)dxcc_table::LOTW);
    confirm_type_ = (dxcc_table::confirm_t)temp;
}

// Remember values for display and confirmation types
void dxcc_view::store_data() {
    Fl_Preferences user_settings(settings_, "User Settings");
    Fl_Preferences dxcc_settings(user_settings, "DXCC Table");
    dxcc_settings.set("Display Type", (int)display_type_);
    dxcc_settings.set("Confirmation", (int)confirm_type_);
}

// something has changed in the book - usually record 1 is to be selected, record_2 usage per view
void dxcc_view::update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2) {
    // Redraw the display
    table_->display_type(display_type_);
}
