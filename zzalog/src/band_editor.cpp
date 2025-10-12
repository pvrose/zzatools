#include "band_editor.h"

#include "band_data.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Input.H>

#include <map>
#include <string>

extern band_data* band_data_;

band_table::band_table(int X, int Y, int W, int H, const char* L) :
    Fl_Scroll(X, Y, W, H, L)
{
    end();
    draw_widgets();
}

band_table::~band_table() {
}

//! Draw all the internal widgets
void band_table::draw_widgets() {
    clear();
    int r = 0;
    begin();
    int cy = y();
    for (auto e : band_data_->get_entries()) {
        band_row* wr = new band_row(x(), cy, w(), HBUTTON);
        wr->data(e);
        cy += HBUTTON;
    }
    end();
}

//! Set selected entry
void band_table::selected(band_data::band_entry_t* e) {
    selected_entry_ = e;
}

//! Get the selected entry
band_data::band_entry_t* band_table::selected() {
    return selected_entry_;
}


//! Constructor

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
band_modechoice::band_modechoice(int X, int Y, int W, int H, const char* L) :
    Fl_Input_Choice(X, Y, W, H, L)
{
    input()->type(FL_NORMAL_OUTPUT);
    menubutton()->callback(cb_menu);
}

//! Desctructor
band_modechoice::~band_modechoice() {
}

//! Menu callback
void band_modechoice::cb_menu(Fl_Widget* w, void* v) {
    Fl_Menu_Button* menu = (Fl_Menu_Button*)w;
    // Evaluate the text string comprising all selected menu items
    std::string text = "";
    for (int ix = 0; ix < menu->size(); ix++) {
        if (menu->menu()[ix].value()) {
            text += menu->text(ix);
            text += ';';
        }
    }
    // And place it in the input
    ((Fl_Input_Choice*)w->parent())->input()->value(text.c_str());
}

//! Value - returns all selected modes
std::set < std::string > band_modechoice::value() {
    std::set<std::string> result;
    for (int ix = 0; ix < menubutton()->size(); ix++) {
        if (menubutton()->menu()[ix].value()) {
            result.insert(menubutton()->text(ix));
        }
    }
    return result;
}

//! Set value
void band_modechoice::value(std::set<std::string> v) {
    std::string text = "";
    for (int ix = 0; ix < menubutton()->size() - 1; ix++) {
        // Mark checked items that are in the supplied modes
        if (v.find(menubutton()->text(ix)) == v.end()) {
            menubutton()->mode(ix, menubutton()->mode(ix) & ~FL_MENU_VALUE);
        }
        else {
            menubutton()->mode(ix, menubutton()->mode(ix) | FL_MENU_VALUE);
            text += menubutton()->text(ix);
            text += ';';
        }
    }
    input()->value(text.c_str());
}


//! Constructor

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
band_row::band_row(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L) {

    const int WW = w() / 10;
    int cx = x();
    w_type_ = new Fl_Choice(cx, y(), WW * 2, h());
    w_type_->callback(cb_type);
    w_type_->tooltip("Select type of entry");

    populate_type(w_type_);

    cx += WW * 2;
    w_lower_ = new Fl_Float_Input(cx, y(), WW, h());
    w_lower_->callback(cb_lower);
    w_lower_->tooltip("Select lower frequency of entry");

    cx += WW;
    w_upper_ = new Fl_Float_Input(cx, y(), WW, h());
    w_upper_->callback(cb_upper);
    w_upper_->tooltip("Select upper frequency of entry");

    cx += WW;
    w_width_ = new Fl_Float_Input(cx, y(), WW, h());
    w_width_->callback(cb_width);
    w_width_->tooltip("Select bandwidth of entry");

    cx += WW;
    w_modes_ = new band_modechoice(cx, y(), WW * 2, h());
    w_modes_->callback(cb_modes);
    w_modes_->tooltip("Select multiple modes of entry");

    populate_mode(w_modes_);

    cx += WW * 2;
    w_description_ = new Fl_Input(cx, y(), WW * 3, h());
    w_description_->callback(cb_description);
    w_description_->tooltip("Select type of entry");

    end();
}

//! Destructor
band_row::~band_row() {
}

//! Set data
void band_row::data(band_data::band_entry_t* d) {
    entry_ = d;
    enable_widgets();
}

//! Enable widgets
void band_row::enable_widgets() {
    char text[32];
    w_type_->value(entry_->type);
    if (entry_->range.lower < 2000.0)
        snprintf(text, sizeof(text), "%0.4f", entry_->range.lower);
    else
        snprintf(text, sizeof(text), "%g", entry_->range.lower);
    w_lower_->value(text);
    if (entry_->range.upper < 2000.0)
        snprintf(text, sizeof(text), "%0.4f", entry_->range.upper);
    else
        snprintf(text, sizeof(text), "%g", entry_->range.upper);
    w_upper_->value(text);
    snprintf(text, sizeof(text), "%g", entry_->bandwidth);
    w_width_->value(text);
    w_modes_->value(entry_->modes);
    w_description_->value(entry_->summary.c_str());
    switch (entry_->type) {
    case band_data::UNKNOWN:
        w_type_->activate();
        w_lower_->deactivate();
        w_upper_->deactivate();
        w_width_->deactivate();
        w_modes_->deactivate();
        w_description_->deactivate();
        break;
    case band_data::BAND:
        w_type_->deactivate();
        w_lower_->deactivate();
        w_upper_->deactivate();
        w_width_->deactivate();
        w_modes_->deactivate();
        w_description_->deactivate();
        break;
    case band_data::SUB_BAND:
        w_type_->activate();
        w_lower_->activate();
        w_upper_->activate();
        w_width_->activate();
        w_modes_->activate();
        w_description_->activate();
        break;
    case band_data::SPOT_SET:
        w_type_->activate();
        w_lower_->activate();
        w_upper_->activate();
        w_width_->activate();
        w_modes_->activate();
        w_description_->activate();
        break;
    case band_data::SPOT:
        w_type_->activate();
        w_lower_->activate();
        w_upper_->deactivate();
        w_width_->activate();
        w_modes_->activate();
        w_description_->activate();
        break;
    case band_data::USER_SPOT:
        w_type_->activate();
        w_lower_->activate();
        w_upper_->deactivate();
        w_width_->activate();
        w_modes_->activate();
        w_description_->activate();
        break;
    }
    band_table* t = ancestor_view<band_table>(this);
    t->selected(entry_);
}

// MAp band_data::entry_t
const std::map<band_data::entry_t, std::string> TYPE_MAP = {
    { band_data::UNKNOWN, "None" },
    { band_data::BAND, "Band" },
    { band_data::SUB_BAND, "Sub-band" },
    { band_data::SPOT, "Spot frequency" },
    { band_data::SPOT_SET, "Set of spots" }
};

//! Populate the type choices
void band_row::populate_type(Fl_Choice* ch) {
    ch->clear();
    for (auto it : TYPE_MAP) {
        ch->add(it.second.c_str());
    }
}

//! Populate the mode choices
void band_row::populate_mode(band_modechoice* ch) {
    ch->clear();
    for (auto it : band_data_->get_modes()) {
        ch->menubutton()->add(it.c_str(), 0, nullptr, nullptr, FL_MENU_TOGGLE);
    }
}

//! Callback from the Type entry field.
void band_row::cb_type(Fl_Widget* w, void* v) {
    band_row* that = ancestor_view<band_row>(w);
    that->entry_->type = (band_data::entry_t)((Fl_Choice*)w)->value();
    that->enable_widgets();
}

//! Callback from the Upper entry field.
void band_row::cb_upper(Fl_Widget* w, void* v) {
    band_row* that = ancestor_view<band_row>(w);
    that->entry_->range.lower = atof(((Fl_Float_Input*)w)->value());
    that->enable_widgets();
}

//! Callback from the Lower entry field.
void band_row::cb_lower(Fl_Widget* w, void* v) {
    band_row* that = ancestor_view<band_row>(w);
    that->entry_->range.upper = atof(((Fl_Float_Input*)w)->value());
    that->enable_widgets();
}

//! Callback from the bandwidth entry field.
void band_row::cb_width(Fl_Widget* w, void* v) {
    band_row* that = ancestor_view<band_row>(w);
    that->entry_->bandwidth = atof(((Fl_Float_Input*)w)->value());
    that->enable_widgets();
}

//! Callback from the Modes entry field.
void band_row::cb_modes(Fl_Widget* w, void* v) {
    band_row* that = ancestor_view<band_row>(w);
    that->entry_->modes = ((band_modechoice*)w)->value();
    that->enable_widgets();
}

//! Callback from the Description entry field.
void band_row::cb_description(Fl_Widget* w, void* v) {
    band_row* that = ancestor_view<band_row>(w);
    that->entry_->summary = ((Fl_Input*)w)->value();
    that->enable_widgets();
}

//! Constructor

 //! \param X horizontal position within host window
 //! \param Y vertical position with hosr window
 //! \param W width 
 //! \param H height
 //! \param L label
band_editor::band_editor(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    scroll_freq_ = 0.0;

    int cx = x() + GAP;
    int cy = y() + GAP;
    int ht = h() - GAP - GAP - HBUTTON;
    int wt = w() - GAP - GAP;

    int WW = wt / 10;

    Fl_Box* ch1 = new Fl_Box(cx, cy, WW * 2, HBUTTON, "Type");
    ch1->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    ch1->box(FL_FLAT_BOX);
    cx += ch1->w();
    Fl_Box* ch2 = new Fl_Box(cx, cy, WW, HBUTTON, "Lower");
    ch2->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    ch2->box(FL_FLAT_BOX);
    cx += ch2->w();
    Fl_Box* ch3 = new Fl_Box(cx, cy, WW, HBUTTON, "Upper");
    ch3->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    ch3->box(FL_FLAT_BOX);
    cx += ch3->w();
    Fl_Box* ch4 = new Fl_Box(cx, cy, WW, HBUTTON, "Bandwidth");
    ch4->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    ch4->box(FL_FLAT_BOX);
    cx += ch4->w();
    Fl_Box* ch5 = new Fl_Box(cx, cy, WW * 2, HBUTTON, "Modes");
    ch5->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    ch5->box(FL_FLAT_BOX);
    cx += ch5->w();
    Fl_Box* ch6 = new Fl_Box(cx, cy, WW * 3, HBUTTON, "Summary");
    ch6->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    ch6->box(FL_FLAT_BOX);

    cx = x() + GAP;
    cy += HBUTTON;
    table_ = new band_table(cx, cy, wt, ht);
    
    cy += ht;
    bn_add_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Add");
    bn_add_->callback(cb_add);
    bn_add_->tooltip("Add a band entry at the same frequency as the selected");

    cx += WBUTTON + GAP;
    bn_delete_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Delete");
    bn_delete_->callback(cb_delete);
    bn_delete_->tooltip("Delete the selected band entry");

    end();

    resizable(table_);

    show();

}

//! Destructor
band_editor::~band_editor() 
{ }

//!Callback from add row button
void band_editor::cb_add(Fl_Widget* w, void* v) {
    band_editor* that = ancestor_view<band_editor>(w);
    band_data::band_entry_t* e = that->table_->selected();
    band_data::band_entry_t* new_e = new band_data::band_entry_t;
    new_e->type = band_data::UNKNOWN;
    new_e->range.lower = e->range.lower;
    band_data_->get_entries().insert(new_e);
    that->redraw();
}

//! Callback from delete row button
void band_editor::cb_delete(Fl_Widget* w, void* v) {
    band_editor* that = ancestor_view<band_editor>(w);
    band_data::band_entry_t* e = that->table_->selected();
    band_data_->get_entries().erase(e);
    that->redraw();
}

//! Select the band record containing \p frequency
void band_editor::value(double frequency) {
    int cy = 0;
    if (frequency != scroll_freq_) {
        scroll_freq_ = frequency;
        for (auto e : band_data_->get_entries()) {
            if (e->type == band_data::BAND) {
                if (e->range.lower <= frequency && e->range.upper >= frequency) {
                    table_->scroll_to(0, cy);
                    break;
                }
            }
            cy += HBUTTON;
        }
    }
}
