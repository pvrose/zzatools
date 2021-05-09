// This class uses windows specific inter-app communication
#ifdef _WIN32

#define _AFXDLL
// #include <afx.h>

#include "dxa_if.h"

#include "../zzalib/callback.h"
#include "book.h"
#include "extract_data.h"
#include "status.h"
#include "tabbed_forms.h"
#include "spec_data.h"
#include "drawing.h"
#include "pfx_data.h"
#include "menu.h"

#include <set>
#include <iostream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.h>
#include <FL/Fl.H>

#include <atlbase.h>
#include <atlcom.h>

using namespace zzalog;
using namespace zzalib;

// Top-level data items
extern Fl_Preferences* settings_;
extern book* book_;
extern extract_data* extract_records_;
extern extract_data* dxatlas_records_;
extern book* navigation_book_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern spec_data* spec_data_;
extern time_t session_start_;
extern menu* menu_;
extern bool in_current_session(record*);

// Constructor
dxa_if::dxa_if() :
	Fl_Window(10, 10, "DxAtlas Control")
	, qso_display_(AQ_NONE)
	, atlas_colour_(AC_NONE)
	, atlas_(nullptr)
	, home_lat_(0.0)
	, home_long_(0.0)
	, centre_lat_(0.0)
	, centre_long_(0.0)
	, atlas_width_(0)
	, atlas_height_(0)
	, atlas_left_(0)
	, atlas_top_(0)
	, call_layer_(nullptr)
	, is_my_change_(false)
	, locator_("")
	, most_recent_count_(0)
	, location_name_("")
	, home_lat_dms_("")
	, home_long_dms_("")
	, centre_mode_(HOME)
	, prefixes_(false)
	, cq_zones_(false)
	, itu_zones_(false)
	, grid_squares_(true)
	, lat_lon_grid_(false)
	, bearing_distance_(false)
	, projection_(DxAtlas::PRJ_RECTANGULAR)
	, pin_size_(3)
{
	records_to_display_.clear();
	colours_used_.clear();
	colour_bns_.clear();
	colour_enables_.clear();
	locations_.clear();
	button_map_.clear();

	load_values();
	// Create the form
	create_form();
	enable_widgets();
	// Connect to DXATLAS:
	connect_dxatlas();
	// If successful
	if (atlas_) {
		// Get map
		DxAtlas::IDxMapPtr map = atlas_->GetMap();
		//delete all existing layers - set call_layer_ to nullptr to prevent callbacks from using it
		call_layer_ = nullptr;
		DxAtlas::ICustomLayersPtr pin_layers = map->GetCustomLayers();
		pin_layers->Clear();
		// Find home location
		if (book_->size()) {
			// Get home location from a record in the book
			selected_locn_ = book_->get_record()->location(false);
		}
		else {
			// Default to 0N 0E
			selected_locn_ = { 0.0, 0.0 };
		}
		enable_widgets();
		get_records();
		get_colours(false);
		draw_pins();
	}
	// Set position 
	position(window_left_, window_top_);
	resizable(nullptr);

}

// Destructor
dxa_if::~dxa_if()
{
	// Remove and delete all other widtgets
	clear();
	colour_bns_.clear();
	colour_enables_.clear();
	// Clear the various data sets
	records_to_display_.clear();
	colours_used_.clear();
	// Disconnect DxAtlas
	disconnect_dxatlas(false);
}

// Handle FL_HIDE and FL_SHOW to get menu to update itself
int dxa_if::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		menu_->update_windows_items();
		return true;
	}

	return Fl_Window::handle(event);
}


// Public methods

// Load values from settings_
void dxa_if::load_values() {
	// Get Configuration
	Fl_Preferences dxatlas_settings(settings_, "DXATLAS:");
	dxatlas_settings.get("QSOs Displayed", (int&)qso_display_, AQ_NONE);
	dxatlas_settings.get("Most Recent Count", (int&)most_recent_count_, 1);
	dxatlas_settings.get("Coloured By", (int&)atlas_colour_, AC_BANDS);
	dxatlas_settings.get("Centre Latitude", centre_lat_, 0.0);
	dxatlas_settings.get("Centre Longitude", centre_long_, 0.0);
	dxatlas_settings.get("Atlas Left", atlas_left_, 0);
	dxatlas_settings.get("Atlas Top", atlas_top_, 0);
	dxatlas_settings.get("Atlas Width", atlas_width_, 0);
	dxatlas_settings.get("Atlas Height", atlas_height_, 0);
	dxatlas_settings.get("Window Left", window_left_, 100);
	dxatlas_settings.get("Window Top", window_top_, 100);
	dxatlas_settings.get("Include SWLs", (int&)include_swl_, false);
	dxatlas_settings.get("Zoom Value", zoom_value_, 1.0);
	dxatlas_settings.get("Centre Mode", (int&)centre_mode_, HOME);
	dxatlas_settings.get("Prefixes", (int&)prefixes_, false);
	dxatlas_settings.get("CQ Zones", (int&)cq_zones_, false);
	dxatlas_settings.get("ITU Zones", (int&)itu_zones_, false);
	dxatlas_settings.get("Gridsquares", (int&)grid_squares_, true);
	dxatlas_settings.get("Latitude Longitude", (int&)lat_lon_grid_, false);
	dxatlas_settings.get("Bearing Distance", (int&)bearing_distance_, false);
	dxatlas_settings.get("Projection", (int&)projection_, DxAtlas::PRJ_RECTANGULAR);
	dxatlas_settings.get("Pin Size", pin_size_, 3);

	// Get stations details - QTH locations used to mark home location on map
	Fl_Preferences stations_settings(settings_, "Stations");
	char* temp;
	// All QTH settings
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	qths_settings.get("Current", temp, "");
	location_name_ = temp;
	Fl_Preferences qth_settings(qths_settings, temp);
	free(temp);
	qth_settings.get("Locator", temp, "RR73TU");
	locator_ = temp;
	free(temp);
	int num_qths = qths_settings.groups();
	for (int i = 0; i < num_qths; i++) {
		string location = qths_settings.group(i);
		locations_.push_back(location);
	}
	// Get home location from gridsquare in float and string form
	home_location();

}

// Used to create the form
void dxa_if::create_form() {

	// now create the groups

	// Group 1 - DxAtlas controls
	Fl_Group* group1 = new Fl_Group(EDGE, EDGE, 10, 10);
	group1->box(FL_THIN_DOWN_BOX);
	group1->labelfont(FONT);
	group1->labelsize(FONT_SIZE);
	// Choice - which QSOs to display
	Fl_Choice* ch11 = new Fl_Choice(group1->x() + GAP, group1->y() + HTEXT, WSMEDIT, HTEXT, "QSOs showing");
	ch11->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	ch11->labelfont(FONT);
	ch11->labelsize(FONT_SIZE);
	ch11->textfont(FONT);
	ch11->textsize(FONT_SIZE);
	ch11->callback(cb_ch_qsos);
	ch11->when(FL_WHEN_RELEASE);
	ch11->clear();
	ch11->add("None");
	ch11->add("Selected QSO");
	ch11->add("All QSOs");
	ch11->add("Extracted QSOs");
	ch11->add("QSOs on recent days");
	ch11->add("Recent QSOs");
	ch11->add("Year to date");
	ch11->add("This session");
	ch11->value((int)qso_display_);
	ch11->tooltip("Select which QSOs to display");
	qso_count_ = ch11;
	// Input - Number of days or QSOs
	Fl_Int_Input* ip11 = new Fl_Int_Input(ch11->x(), ch11->y() + ch11->h(), ch11->w(), HTEXT);
	ip11->textfont(FONT);
	ip11->textsize(FONT_SIZE);
	ip11->callback(cb_ip_number);
	ip11->when(FL_WHEN_ENTER_KEY);
	ip11->value(to_string(most_recent_count_).c_str());
	ip11->tooltip("Enter the number of QSOs or days");
	most_recent_ip_ = ip11;
	// Check button - display SWLs
	Fl_Check_Button* bn11 = new Fl_Check_Button(ch11->x(), ip11->y() + ip11->h(), WRADIO, HRADIO, "Include SWLs");
	bn11->labelfont(FONT);
	bn11->labelsize(FONT_SIZE);
	bn11->align(FL_ALIGN_RIGHT);
	bn11->callback(cb_ch_swlen);
	bn11->when(FL_WHEN_RELEASE);
	bn11->tooltip("Include SWLs in display");
	// Choice - What to colour by
	Fl_Choice* ch12 = new Fl_Choice(ch11->x() + ch11->w() + GAP, ch11->y(), ch11->w(), ch11->h(), "Colour pins by");
	ch12->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	ch12->labelfont(FONT);
	ch12->labelsize(FONT_SIZE);
	ch12->textfont(FONT);
	ch12->textsize(FONT_SIZE);
	ch12->callback(cb_ch_colour);
	ch12->clear();
	ch12->add("All black");
	ch12->add("By band");
	ch12->add("By logged mode");
	ch12->add("By award mode");
	ch12->add("By distance");
	ch12->add("By date");
	ch12->value((int)atlas_colour_);
	// Choice - where to centre the map
	Fl_Choice* ch13 = new Fl_Choice(ch12->x(), ch12->y() + ch12->h() + HTEXT, ch12->w(), ch12->h(), "Centre on...");
	ch13->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	ch13->labelfont(FONT);
	ch13->labelsize(FONT_SIZE);
	ch13->textfont(FONT);
	ch13->textsize(FONT_SIZE);
	ch13->callback(cb_ch_centre);
	ch13->clear();
	ch13->add("Home Location");
	ch13->add("Selected QSO");
	ch13->add("In group");
	ch13->add("At 0°N 0°E");
	ch13->add("Continent/Europe");
	ch13->add("Continent/Asia");
	ch13->add("Continent/Africa");
	ch13->add("Continent/North America");
	ch13->add("Continent/South America");
	ch13->add("Continent/Oceania");
	ch13->add("Continent/Antarctica");
	ch13->value((int)centre_mode_);
	centre_ch_ = ch13;
	// Slider - pin size
	Fl_Slider* sl10 = new Fl_Slider(ch11->x(), ch13->y() + ch13->h() + GAP, ch11->w() - HBUTTON, HBUTTON, "Pin size");
	sl10->type(FL_HORIZONTAL);
	sl10->labelfont(FONT);
	sl10->labelsize(FONT_SIZE);
	sl10->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	sl10->callback(cb_sl_pinsize, &pin_size_);
	sl10->when(FL_WHEN_RELEASE);
	sl10->range(3.0, HBUTTON / 2);
	sl10->step(1);
	sl10->value(pin_size_);
	// Pinsize display
	pz_widget* pz11 = new pz_widget(sl10->x() + sl10->w(), sl10->y(), HBUTTON, HBUTTON);
	pz11->value(pin_size_);
	pz11->labelfont(FONT);
	pz11->labelsize(FONT_SIZE);
	pz11->copy_label(to_string(pin_size_).c_str());
	pz11->color(FL_WHITE);
	pz11->align(FL_ALIGN_TOP);
	pz_w_ = pz11;

	// Button - Start or stop
	Fl_Button* bn12 = new Fl_Button(ch12->x(), ch13->y() + ch13->h() + GAP, WBUTTON, HBUTTON, "Start");
	bn12->labelfont(FONT);
	bn12->labelsize(FONT_SIZE);
	bn12->callback(cb_bn_start_stop);
	bn12->color(FL_GREEN);
	bn12->tooltip("Start or Stop current DxAtlas session");
	start_stop_bn_ = bn12;
	// Button - Recentre
	Fl_Button* bn13 = new Fl_Button(bn12->x() + bn12->w(), bn12->y(), WBUTTON, HBUTTON, "Re-centre");
	bn13->labelfont(FONT);
	bn13->labelsize(FONT_SIZE);
	bn13->callback(cb_bn_centre);
	bn13->color(FL_YELLOW);
	bn13->tooltip("Recentre the DxAtlas window");

	int next_y = max(bn11->y() + bn11->h(), bn12->y() + bn12->h());
	// Choice - provides all user's locations (in settings)
	Fl_Choice* ch21 = new Fl_Choice(ch11->x(), next_y + HTEXT, WSMEDIT, HBUTTON, "Location");
	ch21->labelfont(FONT);
	ch21->labelsize(FONT_SIZE);
	ch21->textfont(FONT);
	ch21->textsize(FONT_SIZE);
	ch21->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	ch21->callback(cb_ch_locn, &location_name_);
	ch21->when(FL_WHEN_RELEASE);
	ch21->clear();
	for (size_t i = 0; i < locations_.size(); i++) {
		int index = ch21->add(locations_[i].c_str());
		if (locations_[i] == location_name_) {
			ch21->value(index);
		}
	}
	ch21->tooltip("Specify the location to use as home location");
	// Output - the locator grid square for the selected location
	Fl_Output* op21 = new Fl_Output(ch21->x(), ch21->y() + ch21->h(), WSMEDIT, HBUTTON);
	op21->labelfont(FONT);
	op21->labelsize(FONT_SIZE);
	op21->textfont(FONT);
	op21->textsize(FONT_SIZE);
	op21->value(locator_.c_str());
	op21->box(FL_FLAT_BOX);
	op21->color(group1->color());
	op21->tooltip("The grid-square of the current home location");
	locator_op_ = op21;
	// Output - the latitude of the location
	Fl_Output* op22 = new Fl_Output(ch12->x(), ch21->y(), WSMEDIT, HBUTTON);
	op22->labelfont(FONT);
	op22->labelsize(FONT_SIZE);
	op22->textfont(FONT);
	op22->textsize(FONT_SIZE);
	op22->value(home_lat_dms_.c_str());
	op22->box(FL_FLAT_BOX);
	op22->color(group1->color());
	op22->tooltip("The latitude of the current home location");
	lat_dms_op_ = op22;
	// Output - the longitude of the location
	Fl_Output* op23 = new Fl_Output(op22->x(), op22->y() + op22->h(), WSMEDIT, HBUTTON);
	op23->labelfont(FONT);
	op23->labelsize(FONT_SIZE);
	op23->textfont(FONT);
	op23->textsize(FONT_SIZE);
	op23->value(home_long_dms_.c_str());
	op23->box(FL_FLAT_BOX);
	op23->color(group1->color());
	op23->tooltip("The longitude of the current home location");
	lon_dms_op_ = op23;

	// Resize group by size of choices
	const int WGRP_1 = ch12->x() + ch12->w() + GAP;
	const int HGRP_1 = max(op21->y() + op21->h(), op23->y() + op23->h()) + GAP;
	group1->resizable(nullptr);
	group1->size(WGRP_1, HGRP_1);
	group1->end();
	// Size of window
	const int WWIN = EDGE + WGRP_1 + EDGE;

	// Group to contain the buttons displaying the colours
	colour_grp_ = new Fl_Group(EDGE, HGRP_1 + GAP, WGRP_1, HTEXT);
	label_colour_grp();
	colour_grp_->box(FL_THIN_DOWN_BOX);
	colour_grp_->labelfont(FONT);
	colour_grp_->labelsize(FONT_SIZE);
	colour_grp_->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	
	Fl_Button* bn_set_all = new Fl_Button(EDGE + GAP+ WBUTTON + WBUTTON, colour_grp_->y() + HTEXT, WBUTTON, HBUTTON, "Set all");
	bn_set_all->labelfont(FONT);
	bn_set_all->labelsize(FONT_SIZE);
	bn_set_all->align(FL_ALIGN_CENTER);
	bn_set_all->callback(cb_bn_all, (void*)true);

	Fl_Button* bn_clr_all = new Fl_Button(bn_set_all->x() + bn_set_all->w() , bn_set_all->y(), WBUTTON, HBUTTON, "Clear all");
	bn_clr_all->labelfont(FONT);
	bn_clr_all->labelsize(FONT_SIZE);
	bn_clr_all->align(FL_ALIGN_CENTER);
	bn_clr_all->callback(cb_bn_all, (void*)false);

	colour_grp_->end();

	resizable(nullptr);
	resize(window_left_, window_top_, WWIN, colour_grp_->y() + colour_grp_->h() + EDGE);

	// Add all the colour buttons

	get_colours(true);
	this->position(window_left_, window_top_);
	this->show();
};

// Used to write settings back
void dxa_if::save_values() {
	// Get Configuration
	Fl_Preferences dxatlas_settings(settings_, "DXATLAS:");
	dxatlas_settings.set("QSOs Displayed", qso_display_);
	dxatlas_settings.set("Most Recent Count", (signed)most_recent_count_);
	dxatlas_settings.set("Coloured By", atlas_colour_);
	dxatlas_settings.set("Centre Latitude", centre_lat_);
	dxatlas_settings.set("Centre Longitude", centre_long_);
	dxatlas_settings.set("Atlas Left", atlas_left_);
	dxatlas_settings.set("Atlas Top", atlas_top_);
	dxatlas_settings.set("Atlas Width", atlas_width_);
	dxatlas_settings.set("Atlas Height", atlas_height_);
	dxatlas_settings.set("Window Left", x_root());
	dxatlas_settings.set("Window Top", y_root());
	dxatlas_settings.set("Include SWLs", include_swl_);
	dxatlas_settings.set("Centre Mode", centre_mode_);
	dxatlas_settings.set("Prefixes", (int&)prefixes_);
	dxatlas_settings.set("CQ Zones", (int&)cq_zones_);
	dxatlas_settings.set("ITU Zones", (int&)itu_zones_);
	dxatlas_settings.set("Gridsquares", (int&)grid_squares_);
	dxatlas_settings.set("Latitude Longitude", (int&)lat_lon_grid_);
	dxatlas_settings.set("Bearing Distance", (int&)bearing_distance_);
	dxatlas_settings.set("Projection", (int&)projection_);
	dxatlas_settings.set("Pin Size", pin_size_);
	dxatlas_settings.set("Zoom Value", zoom_value_);
}

// Used to enable/disable specific widget - any widgets enabled must be attributes
void dxa_if::enable_widgets() {
	// The number of most-recent items input
	if (qso_display_ == AQ_DAYS || qso_display_ == AQ_QSOS) {
		most_recent_ip_->activate();
	}
	else {
		most_recent_ip_->deactivate();
	}
	// Start or stop button
	if (atlas_ == nullptr) {
		start_stop_bn_->label("Start");
		start_stop_bn_->color(FL_GREEN);
	}
	else {
		start_stop_bn_->label("Stop");
		start_stop_bn_->color(FL_RED);
	}
	start_stop_bn_->labelcolor(fl_contrast(FL_BLACK, start_stop_bn_->color()));
	// Centre choice - enable if rectangular projection
	if (atlas_) {
		DxAtlas::IDxMapPtr map = atlas_->GetMap();
		Fl_Choice* ch = (Fl_Choice*)centre_ch_;
		switch (map->GetProjection()) {
		case DxAtlas::PRJ_RECTANGULAR:
			// Set all choice items active
			ch->activate();
			for (int ix = 0; ix < ch->size() - 1; ix++) {
				ch->mode(ix, ch->mode(ix) & ~FL_MENU_INACTIVE);
			}
			break;
		case DxAtlas::PRJ_AZIMUTHAL:
			// Set only HOME (Zoom to group) or ZERO (zoom out)
			ch->activate();
			for (int ix = 0; ix < ch->size() - 1; ix++) {
				switch ((centre_t)ix) {
				case HOME:
				case ZERO:
					ch->mode(ix, ch->mode(ix) & ~FL_MENU_INACTIVE);
					break;
				default:
					ch->mode(ix, ch->mode(ix) | FL_MENU_INACTIVE);
					break;
				}
			}
			break;
		default:
			ch->deactivate();
			break;
		}
	}
	// Enable colour buttons
	for (size_t i = 0; i < colour_bns_.size(); i++) {
		if (colour_enables_[i]) {
			colour_bns_[i]->color(button_colour(i));
			colour_bns_[i]->labelcolor(fl_contrast(FL_BLACK, button_colour(i)));
		}
		else {
			Fl_Color inactive_colour = fl_color_average(button_colour(i), FL_WHITE, 0.5f);
			colour_bns_[i]->color(inactive_colour);
			colour_bns_[i]->labelcolor(fl_contrast(FL_BLACK, inactive_colour));
		}
	}

	redraw();
}

// Update location widgets
void dxa_if::update_loc_widgets() {
	((Fl_Output*)locator_op_)->value(locator_.c_str());
	((Fl_Output*)lat_dms_op_)->value(home_lat_dms_.c_str());
	((Fl_Output*)lon_dms_op_)->value(home_long_dms_.c_str());
}


// Callbacks

// Choice for QSOs
void dxa_if::cb_ch_qsos(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	// get value of radio button
	cb_value<Fl_Choice, int>(w, (void*)&that->qso_display_);
	// Redraw the data
	that->enable_widgets();
	that->get_records();
	that->get_colours(false);
	that->draw_pins();
}

// Input for most recent count
void dxa_if::cb_ip_number(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	cb_value_int<Fl_Int_Input>(w, (void*)& that->most_recent_count_);
	// Redraw the data
	that->enable_widgets();
	that->get_records();
	that->get_colours(false);
	that->draw_pins();
}

// Colour choice value selected - redraw pins using new colour spec
void dxa_if::cb_ch_colour(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	// get value
	cb_value<Fl_Choice, int>(w, &that->atlas_colour_);
	// Redraw data
	that->label_colour_grp();
	that->get_records();
	that->get_colours(true);
	that->draw_pins();
}

// Include SWL reports
void dxa_if::cb_ch_swlen(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	// get value
	cb_value<Fl_Check_Button, bool>(w, &that->include_swl_);
	// Redraw the data
	that->enable_widgets();
	that->get_records();
	that->get_colours(false);
	that->draw_pins();
}

// Open or Close DxAtlas
void dxa_if::cb_bn_start_stop(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	// Connect to DXATLAS: if not already
	if (that->atlas_ == nullptr) {
		that->connect_dxatlas();

		// and get new position of DXATLAS:
		that->atlas_left_ = that->atlas_->GetLeft();
		that->atlas_width_ = that->atlas_->GetWidth();
		that->atlas_top_ = that->atlas_->GetTop();
		that->atlas_height_ = that->atlas_->GetHeight();
		that->save_values();

		// Enable the controls, update configuration, and initialise the map
		that->enable_widgets();
		// Get the items to display
		that->get_records();
		that->get_colours(true);

		that->draw_pins();
	}
	else {
		that->disconnect_dxatlas(false);
		that->enable_widgets();
	}
}

// Location choice widget selected
void dxa_if::cb_ch_locn(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	// Get the location selected
	that->location_name_ = ((Fl_Choice*)w)->mvalue()->label();
	// Get the details of the new location from settings
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	Fl_Preferences qth_settings(qths_settings, that->location_name_.c_str());
	char * temp;
	qth_settings.get("Locator", temp, "RR73TU");
	that->locator_ = temp;
	free(temp);
	// Get home location from gridsquare in string form
	that->home_location();
	// Update the output widgets with the values of the new location
	that->update_loc_widgets();
	that->draw_home_flag();
}

// Map centre choice widget selected
void dxa_if::cb_ch_centre(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	if (that->atlas_) {
		DxAtlas::IDxMapPtr map = that->atlas_->GetMap();
		if (map->GetProjection() == DxAtlas::PRJ_RECTANGULAR) {
			cb_value<Fl_Choice, int>(w, &that->centre_mode_);
			that->centre_map();
		}
	}
}

// Re-centre map button selected
void dxa_if::cb_bn_centre(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	if (that->atlas_) {
		DxAtlas::IDxMapPtr map = that->atlas_->GetMap();
		if (map->GetProjection() == DxAtlas::PRJ_RECTANGULAR) {
			that->centre_map();
		}
	}
}

// One of the colour buttons selected - enable/disable that colour
void dxa_if::cb_bn_colour(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	int number = (int)v;
	if (that->colour_enables_[number]) {
		that->colour_enables_[number] = false;
	}
	else {
		that->colour_enables_[number] = true;
	}
	that->enable_widgets();
	that->draw_pins();
}

// Set all or clear all selected
void dxa_if::cb_bn_all(Fl_Widget* w, void* v) {
	dxa_if* that = ancestor_view<dxa_if>(w);
	bool enable = (long)v;
	for (size_t i = 0; i < that->colour_enables_.size(); i++) {
		that->colour_enables_[i] = enable;
	}
	that->enable_widgets();
	that->draw_pins();
}

// Pinsize slider
void dxa_if::cb_sl_pinsize(Fl_Widget* w, void* v) {
	cb_value<Fl_Slider, int>(w, v);
	dxa_if* that = ancestor_view<dxa_if>(w);
	((pz_widget*)that->pz_w_)->value(*(int*)v);
	that->pz_w_->copy_label(to_string(*(int*)v).c_str());
	that->enable_widgets();
	that->draw_pins();
}

// Call back from DXATLAS: when the mouse has been clicked on the map - show selected QSO or QSOs
// If we are displaying extracted records then only show the last record in the record view.
// If we are not, then copy all records within the tolerance to the extracted records.
HRESULT dxa_if::cb_map_clicked(float latitude, float longitude) {
	int num_found = 0;
	record_num_t record_num;
	record_num_t display_num = 0;
	float tolerance = (float)5.0 / zoom_value_;
	float best_diff = tolerance;
	record* disp_record = nullptr;
	if (qso_display_ != AQ_SEARCH && extract_records_) {
		// Copy all records within the specified tolerance of the click point in the extracted records
		// view. So clear any existing criteria and generate header 
		extract_records_->clear_criteria();
		record* header = new record;
		char format[] = "DxAtlas generated extract.\nContacts near %s %s (%s)\n";
		string s_lat = degrees_to_dms(latitude, true);
		string s_lon = degrees_to_dms(longitude, false);
		string gridsquare = latlong_to_grid({ latitude, longitude }, 6);
		char* text = new char[strlen(format) + s_lat.length() + s_lon.length() + 20];
		sprintf(text, format, s_lat.c_str(), s_lon.c_str(), gridsquare.c_str());
		header->header(string(text));
		extract_records_->header(header);
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
	}
	for (auto it = records_displayed_.begin(); it != records_displayed_.end(); it++) {
		// For all records beging displayed - get those that are within a small distance 
		// from the click point and add them to the list control
		record_num = *it;
		disp_record = book_->get_record(record_num, false);
		lat_long_t lat_long = disp_record->location(false);
		float d_lat = (float)lat_long.latitude - latitude;
		float d_long = (float)lat_long.longitude - longitude;
		float diff = sqrtf((d_lat * d_lat) + (d_long * d_long));
		if (diff < tolerance) {
			// within a small tolerance from the click point
			// Set the selected record to each record in turn
			if (qso_display_ != AQ_SEARCH && extract_records_) {
				extract_records_->add_record(record_num);
			}
			num_found++;
			// Set the record to display to the closest
			if (diff < best_diff) {
				best_diff = diff;
				display_num = record_num;
			}
		}
	}
	if (!num_found) {
		// Do nothing except update status
		status_->misc_status(ST_WARNING, "DXATLAS: No stations found");
	}
	else {
		// Send how many found to the status
		char message[256];
		sprintf(message, "DXATLAS: %d stations found", num_found);
		status_->misc_status(ST_NOTE, message);
		book_->selection(display_num);
		if (tabbed_forms_) {
			// Open the appropriate view
			if (qso_display_ != AQ_SEARCH) {
				tabbed_forms_->activate_pane(OT_EXTRACT, true);
			}
			else {
				tabbed_forms_->activate_pane(OT_RECORD, true);
			}
		}
	}
	return S_OK;
}

// Callback from DXATLAS: that the mouse position has changed
HRESULT dxa_if::cb_mouse_moved(float latitude, float longitude) {
	if (call_layer_ != nullptr && !is_my_change_) {
		// 
		_variant_t pt_lat, pt_long, pt_text;
		_variant_t label, labels;

		pt_lat.ChangeType(VT_R4);
		pt_long.ChangeType(VT_R4);

		label.vt = VT_ARRAY | VT_VARIANT;
		labels.vt = VT_ARRAY | VT_VARIANT;

		DxAtlas::IDxMapPtr map = atlas_->GetMap();
		// We are going to change the map - let callbacks know and start building bulk change.
		is_my_change_ = true;
		map->BeginUpdate();

		//create array of labels - 1 dimensional
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = 1;

		// Create an array of data (3 data items, longitude, latitude and callsign
		SAFEARRAY * points = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
		if (points == nullptr) {
			status_->misc_status(ST_SEVERE, "DXATLAS: Fatal error in callback");
			return E_ABORT;
		}
		rgsabound[0].cElements = 3;
		SAFEARRAY* point = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
		if (point == nullptr) {
			status_->misc_status(ST_SEVERE, "DXATLAS: Fatal error in callback");
			return E_ABORT;
		}
		// Default data will not display anything - 
		// if mouseover is not enabled or no record found close enough
		pt_long = (float)0.0;
		pt_lat = (float)0.0;
		pt_text.SetString(" ");

		// Start by having ridiculous closest point (As far away as we can consider)
		record_num_t closest_num = -1;
		float closest_diff = 360.0;
		float allowed_diff = (float)5.0 / zoom_value_;
		for (auto it = records_to_display_.begin(); it != records_to_display_.end(); it++ ) {
			// For all records being displayed
			record_num_t record_num = *it;
			// Get the location of the record
			record* record = book_->get_record(record_num, false);
			if (record) {
				lat_long_t lat_long = record->location(false);
				// If this is closer than previous closest
				float diff_lat = (float)(lat_long.latitude) - latitude;
				float diff_long = (float)(lat_long.longitude) - longitude;
				// Use sum of squares to get a qualitative figure of distance
				float this_diff = sqrtf((diff_lat*diff_lat) + (diff_long*diff_long));
				if (this_diff < closest_diff && this_diff < allowed_diff) {
					// Check whether it gets displayed
					if (is_displayed(record_num)) {
						closest_diff = this_diff;
						closest_num = record_num;
					}
				}
			}
		}
		// We now have the closest
		if (closest_num != -1) {
			record* record = book_->get_record(closest_num, false);
			if (record != nullptr) {
				lat_long_t lat_long = record->location(false);

				// Only add if the location is valid
				if (!isnan(lat_long.latitude) && !isnan(lat_long.longitude)) {
					//calculate attributes
					pt_long = (float)lat_long.longitude;
					pt_lat = (float)lat_long.latitude;
					pt_text.SetString(record->item("CALL").c_str());
				}
			}
		}

		//set attributes for point
		long ix = 0;
		(void)SafeArrayPutElement(point, &ix, &pt_long);
		ix = 1; (void)SafeArrayPutElement(point, &ix, &pt_lat);
		ix = 2; (void)SafeArrayPutElement(point, &ix, &pt_text);
		//add point to the array
		ix = 0; label.parray = point;
		(void)SafeArrayPutElement(points, &ix, &label);
		//put data into the layer
		labels.parray = points;
		try {
			call_layer_->SetData(labels);
		}
		catch (_com_error* e) {
			char error[256];
			sprintf(error, "DXATLAS: Got error displaying data : %s", e->ErrorMessage());
			status_->misc_status(ST_ERROR, error);
		}
		//now allow repainting
		map->EndUpdate();
		is_my_change_ = false;
	}
	return S_OK;
}

// Map changed callback - if projection changed it will affect which widgets are enabled
HRESULT dxa_if::cb_map_changed(DxAtlas::EnumMapChange change) {
	if (change == DxAtlas::EnumMapChange::MC_PROJECTION && atlas_ && !is_my_change_) {
		projection_ = atlas_->GetMap()->GetProjection();
		enable_widgets();
		if (projection_ == DxAtlas::EnumProjection::PRJ_RECTANGULAR) {
			// Rectangular projection so center the map appropriately
			centre_map();
		}
	}
	return S_OK;
}

// DxAtlas closing - disconnect from DxAtlas and tidy it up.
HRESULT dxa_if::cb_exit_requested() {
	disconnect_dxatlas(true);
	return S_OK;
}

// Get the colour for a particular button number
Fl_Color dxa_if::button_colour(int button_num) {
	return ZLG_PALETTE[button_num];
}

// Get details of the home location for the selected record
void dxa_if::home_location() {
	// Get home location for the current active location
	lat_long_t home_lat_long = grid_to_latlong(locator_);
	// Convert to single precision explicitly
	home_lat_ = home_lat_long.latitude;
	home_long_ = home_lat_long.longitude;
	// Convert to ° ' " N/E/S/W format
	home_lat_dms_ = degrees_to_dms((float)home_lat_, true);
	home_long_dms_ = degrees_to_dms((float)home_long_, false);
	// Add uncertainty
	switch (locator_.length()) {
	case 0:
		home_lat_dms_ += " ±90°";
		home_long_dms_ += " ±180°";
		break;
	case 2:
		home_lat_dms_ += " ±5°";
		home_long_dms_ += " ±10°";
		break;
	case 4:
		home_lat_dms_ += " ±30'";
		home_long_dms_ += " ±1°";
		break;
	case 6:
		home_lat_dms_ += " ±1'15\"";
		home_long_dms_ += " ±2'30\"";
		break;
	case 8:
		home_lat_dms_ += " ±7.5\"";
		home_long_dms_ += " ±15\"";
		break;
	}
}

// Connect to DXATLAS:
bool dxa_if::connect_dxatlas() {
	bool opened_ok = true;

	if (atlas_ == nullptr) {
		// We aren't connected already
		try {
			// First try to connect to DX Atlas if it is running
			HRESULT hAtlas = atlas_.GetActiveObject("DxAtlas.Atlas");
			if (FAILED(hAtlas)) _com_issue_error(hAtlas);
		}
		catch (_com_error& /*e*/) {
			try {
				// If that fails, try to start a new instance of DX Atlas
				HRESULT hAtlas = atlas_.CreateInstance("DxAtlas.Atlas", nullptr, CLSCTX_ALL);
				if (FAILED(hAtlas)) _com_issue_error(hAtlas);
			}
			catch (_com_error& /*e*/) {
				// And if that fails, say oops and hide this view
				status_->misc_status(ST_ERROR, "DXATLAS: Unable to connect to DxAtlas.");
				opened_ok = false;
			}
		}

		if (opened_ok) {
			try {
				// Tell the connection we are implementing callbacks
				IDispEventSimpleImpl<2, dxa_if, &__uuidof(DxAtlas::IDxAtlasEvents)>::DispEventAdvise(atlas_);
			} 
			catch (exception&) {
				// And if that fails, say oops and hide this view
				status_->misc_status(ST_ERROR, "DXATLAS: Exception in DxAtlas.");
				opened_ok = false;
			}
		}
	}
	if (opened_ok) {
		// Initialise the map and make DXATLAS: visible
		enable_widgets();
		initialise_map();
		atlas_->PutVisible(true);
		// Allow mouse click and override active labels
		atlas_->GetMap()->PutMouseMode(DxAtlas::MM_USER);
		atlas_->GetMap()->PutActiveLabels(false);
		status_->misc_status(ST_OK, "DXATLAS: Connected");
	}
	else {
		status_->misc_status(ST_ERROR, "DXATLAS: failed to open");
	}

	return opened_ok;

}

// Disconnect from DXATLAS:
void dxa_if::disconnect_dxatlas(bool dxatlas_exit) {
	if (atlas_) {
		// Read all the data we want to save in settings.
		DxAtlas::IDxMapPtr map = atlas_->GetMap();
		atlas_left_ = atlas_->GetLeft();
		atlas_width_ = atlas_->GetWidth();
		atlas_top_ = atlas_->GetTop();
		atlas_height_ = atlas_->GetHeight();
		zoom_value_ = map->GetZoom();
		centre_lat_ = map->GetCenterLatitude();
		centre_long_ = map->GetCenterLongitude();
		prefixes_ = map->GetPrefixesVisible();
		cq_zones_ = map->GetCqZonesVisible();
		itu_zones_ = map->GetItuZonesVisible();
		grid_squares_ = map->GetGridSquaresVisible();
		lat_lon_grid_ = map->GetLatLonGridVisible();
		bearing_distance_ = map->GetBearingDistanceGridVisible();
		projection_ = map->GetProjection();
		save_values();
		// Ignore any callbacks from DXATLAS: while we disconnect it.
		is_my_change_ = true;
		map->BeginUpdate();
		map->GetCustomLayers()->Clear();
		map->EndUpdate();
		call_layer_ = nullptr;
		// Tell the interface we no longer support the callbacks
		IDispEventSimpleImpl<2, dxa_if, & __uuidof(DxAtlas::IDxAtlasEvents)>::DispEventUnadvise(atlas_);
		atlas_ = nullptr;
		is_my_change_ = false;
		status_->misc_status(ST_WARNING, "DXATLAS: Disconnected");
		if (dxatlas_exit) {
			enable_widgets();
		}
	}
}

// Initialise DXATLAS: with the current configuration
void dxa_if::initialise_map() {
	DxAtlas::IDxMapPtr map = atlas_->GetMap();

	// Don't redraw until we have set all the properties
	is_my_change_ = true;
	map->BeginUpdate();

	// Find the screen that intersects with the saved dimensions - Note this returns zero if there is none.
	int screen = Fl::screen_num(atlas_left_, atlas_top_, atlas_width_, atlas_height_);
	int screen_x;
	int screen_y;
	int screen_w;
	int screen_h;
	// now see if it really does intersect and by how much
	Fl::screen_work_area(screen_x, screen_y, screen_w, screen_h, screen);
	int w;
	int h;
	if (atlas_left_ > screen_x) {
		// Want to place it to the right of the left screen edge
		if (atlas_left_ > screen_x + screen_w) {
			// It is wholly to theright of the screen
			w = 0;
		}
		else if (atlas_left_ + atlas_width_ > screen_x + screen_w) {
			// It overhangs the right edge of the screen
			w = screen_w - (atlas_left_ - screen_x);
		}
		else {
			// It is wholly within the screen
			w = atlas_width_;
		}
	}
	else {
		// Want to place it to the left of the left edge
		if (atlas_left_ + atlas_width_ > screen_x) {
			// It overhangs the left edge
			w = atlas_width_ - (screen_x - atlas_left_);
		}
		else {
			// It is wholly off the left of the screen
			w = 0;
		}
	}
	if (atlas_top_ > screen_y) {
		// Want to place it below the top edge
		if (atlas_top_ > screen_y + screen_h) {
			// It is wholly below the screen
			h = 0;
		} 
		else if (atlas_top_ + atlas_height_ > screen_y + screen_h) {
			// It overhangs tthe bottom of the screen
			h = screen_h - (atlas_top_ - screen_y);
		}
		else {
			// It is wholly with the screen
			h = atlas_height_;
		}
	}
	else {
		// Want to place it above the screen
		if (atlas_top_ + atlas_height_ > screen_y) {
			// It overhangs the top edge
			h = atlas_height_ - (screen_y - atlas_top_);
		}
		else {
			// It is wholly above the screen
			h = 0;
		}
	}
	// If we have a reasonable overlap - greater than 10*10 pixels
	if (h > 10 && w > 10) {
		atlas_->PutLeft(atlas_left_);
		atlas_->PutWidth(atlas_width_);
		atlas_->PutTop(atlas_top_);
		atlas_->PutHeight(atlas_height_);
	}
	// Set map properties
	map->PutProjection(projection_);
	map->PutPrefixesVisible((VARIANT_BOOL)prefixes_);
	map->PutCqZonesVisible((VARIANT_BOOL)cq_zones_);
	map->PutItuZonesVisible((VARIANT_BOOL)itu_zones_);
	map->PutGridSquaresVisible((VARIANT_BOOL)grid_squares_);
	map->PutLatLonGridVisible((VARIANT_BOOL)lat_lon_grid_);
	map->PutBearingDistanceGridVisible((VARIANT_BOOL)bearing_distance_);
	map->PutCenterLatitude((float)centre_lat_);
	map->PutCenterLongitude((float)centre_long_);
	map->PutZoom(zoom_value_);

	// Set map parameters
	draw_home_flag();
	// Don't use pins facility within DXATLAS:
	map->PutPinsVisible(false);

	map->EndUpdate();
	is_my_change_ = false;
}

// Create colour buttons with current colour configuration
void dxa_if::create_colour_buttons() {

	int button_num = 0;
	// Delete all existing colour buttons
	for (size_t i = 0; i < colour_bns_.size(); i++) {
		colour_grp_->remove(colour_bns_[i]);
		delete colour_bns_[i];
	}
	// Clear button map (name to widget)
	button_map_.clear();
	// get FLTK scheduler to delete them
	Fl::wait();
	colour_bns_.clear();
	colour_enables_.clear();
	// Hide the window so it can be resized
	hide();

	// Work out the geometry of the buttons 
	int num_colours = colours_used_.size();
	// If we have no colours create a button with the label "None"
	if (num_colours == 0) {
		num_colours = 1;
		colours_used_.push_back("None");
	}

	// Work out how much space we have to draw them in - the size of the group less one row (for set and clear)
	int num_cols = (colour_grp_->w() - GAP - GAP) / WBUTTON;
	int num_rows = ((num_colours - 1) / num_cols) + 1; 

	// Create all buttons
	button_num = 0;
	int button_todo = num_colours;
	for (int r = 0; r < num_rows; r++) {
		for (int c = 0; c < num_cols && button_num < num_colours; c++) {
			Fl_Button* bn = new Fl_Button(colour_grp_->x() + (c * WBUTTON) + GAP, (r * HBUTTON) + HBUTTON + colour_grp_->y() + HTEXT, WBUTTON, HBUTTON);
			bn->box(FL_BORDER_BOX);
			bn->labelfont(FONT);
			bn->labelsize(FONT_SIZE);
			bn->color(button_colour(button_num));
			bn->copy_label(colours_used_[button_num].c_str());
			bn->labelcolor(fl_contrast(FL_BLACK, button_colour(button_num)));
			bn->callback(cb_bn_colour, (void*)button_num);
			colour_bns_.push_back(bn);
			colour_enables_.push_back(true);
			colour_grp_->add(bn);
			button_map_[colours_used_[button_num]] = bn;
			button_num++;
			button_todo--;
		}
	}
	// now resize the window to just contain the buttons and don't allow the user to resize it smaller
	colour_grp_->resizable(nullptr);
	resizable(colour_grp_);
	colour_grp_->size(colour_grp_->w(), (num_rows * HBUTTON) + HTEXT + GAP + GAP);
	size(w(), colour_grp_->y() + colour_grp_->h() + GAP + EDGE);
	size_range(w(), h());
	show();
	redraw();
}

// Is the point displayed
bool dxa_if::is_displayed(record_num_t record_num) {
	string selected_by;
	long days_old;
	record* record = book_->get_record(record_num, false);
	if (record == nullptr) {
		return false;
	}
	switch (atlas_colour_) {
	case AC_NONE:
		// Is displayed, no need to look further
		return true;
	case AC_AWARDMODE:
		// Get the awardmode
		selected_by = spec_data_->dxcc_mode(record->item("MODE"));
		break;
	case AC_LOGMODE:
		// Get the logged mode
		selected_by = record->item("MODE", true);
		break;
	case AC_BANDS:
		// get the logged band
		selected_by = record->item("BAND");
		break;
	case AC_DISTANCE:
		// Get the distance
		selected_by = get_distance(record);
		break;
	case AC_DAYS:
		// Get days ago
		days_old = (long)difftime(last_time_, record->timestamp()) / (24 * 3600);
		if (days_old >= 10) {
			selected_by = ">=10 days";
		}
		else {
			selected_by = to_string(days_old) + " days";
		}
	}
	// Get the colour index for this
	int colour_num = -1;
	for (size_t i = 0; i < colours_used_.size() && colour_num == -1; i++) {
		if (colours_used_[i] == selected_by) {
			colour_num = i;
		}
	}
	// If the colour wasn't found or it's not in the list of used colours
	if (colour_num == -1) {
		return false;
	}
	else {
		return true;
	}
}

// Convert FLTK colour value to DXATLAS: colour value 
// NB. Although DXATLAS uses a 24-bit value to represent colour, it only has a
// limited palette.
DxAtlas::EnumColor dxa_if::convert_colour(Fl_Color colour) {
	unsigned char red;
	unsigned char blue;
	unsigned char green;
	Fl::get_color(colour, red, green, blue);
	unsigned result = ((unsigned)blue << 16) | ((unsigned)green << 8) | (unsigned)red;
	char message[200];
	snprintf(message, 200, "DEBUG: Fl_Color %8x, DxAtlas: %8x\n", colour, result);
	// cout << message;
	return (DxAtlas::EnumColor)result;
}

// Get records to display
void dxa_if::get_records() {
	fl_cursor(FL_CURSOR_WAIT);
	// Temporary list of record numbers
	set<int> record_nums;
	// select all, current, extracted or most recent
	switch (qso_display_) {
	case AQ_ALL:
		// Get all record numbers
		for (record_num_t i = 0; i < book_->size(); i++) {
			record_nums.insert(i);
		}
		break;
	case AQ_CURRENT:
		// List of 1 - the number of currently selected record
		record_nums.insert(book_->selection());
		break;
	case AQ_SEARCH:
		// Copy the list of extracted records
		for (record_num_t i = 0; i < extract_records_->size(); i++) {
			record_nums.insert(extract_records_->record_number(i));
		}
		break;
	case AQ_DAYS:
		// Get all record numbers that are within so many of most recently operated date
	{
		if (book_->size()) {
			// only if the log has records
			record_num_t last_record_num = book_->size() - 1;
			time_t last_date = book_->get_record(last_record_num, false)->timestamp();
			bool done = false;
			// Always insert the last record
			record_nums.insert(last_record_num);
			record_num_t i = last_record_num - 1;
			double most_recent_seconds = (double)most_recent_count_ * 24 * 60 * 60;
			// Go backwards until the time difference is > n days.
			for (; (signed)i >= 0 && !done; i--) {
				// Compare the time difference (in seconds)
				if (difftime(last_date, book_->get_record(i, false)->timestamp()) <= most_recent_seconds) {
					record_nums.insert(i);
				}
				else {
					// We guarantee the book is in time order
					done = true;
				}
			}
		}
		break;
	}
	case AQ_QSOS:
		// The most recent N QSOs
		for (size_t i = book_->size() - 1, j = 0; j < most_recent_count_; i--, j++) {
			record_nums.insert(i);
		}
		break;

	case AQ_YEAR:
		// Get all records in the current year to date
		if (book_->size()) {
			string this_year = now(false, "%Y");
			for (size_t i = book_->size() - 1; book_->get_record(i, false)->item("QSO_DATE").substr(0, 4) == this_year; i--) {
				record_nums.insert(i);
			}
		}
		break;
	case AQ_SESSION:
		if (book_->size()) {
			for (size_t i = book_->size() - 1; in_current_session(book_->get_record(i, false)); i--) {
				record_nums.insert(i);
			}
		}
	}

	// now see if QSOs, DXCCs or zones
	set<string> got_items;

	records_to_display_.clear();
	// All records - copy temporary list
	records_to_display_.insert(record_nums.begin(), record_nums.end());

	if (atlas_) {
		if (records_to_display_.size() == 0) {
			status_->misc_status(ST_WARNING, "DXATLAS: No records match these criteria");
		}
		else {
			char text[256];
			sprintf(text, "DXATLAS: %d records being displayed", records_to_display_.size());
			status_->misc_status(ST_OK, text);
		}
	}
	else {
		status_->misc_status(ST_WARNING, "DXATLAS: Not connected - change has been remembered");
	}
	fl_cursor(FL_CURSOR_DEFAULT);
}

// Allocate colours - and reset filter
void dxa_if::allocate_colours() {
	// Get the colour list
	colours_used_.clear();
	set<string> text_values;
	text_values.clear();
	// Select on colour mode
	switch (atlas_colour_) {
	case AC_NONE:
		// Default colour only
		colours_used_.push_back("Default");
		break;
	case AC_BANDS:
		// Colour by the bands that have been found in the book
		if (book_) {
			text_values = book_->used_bands();
			// Arrange the bands in frequency order rather than string order of the band name
			for (auto it = text_values.begin(); it != text_values.end(); it++) {
				string text = *it;
				bool done = false;
				for (size_t i2 = 0; i2 < colours_used_.size() && !done; i2++) {
					// insert it if it's less than the current entry 
					if (spec_data_->freq_for_band(text) < spec_data_->freq_for_band(colours_used_[i2])) {
						colours_used_.insert(colours_used_.begin() + i2, text);
						done = true;
					}
				}
				if (!done) {
					colours_used_.push_back(text);
				}
			}
		}
		break;
	case AC_LOGMODE:
		// Colour by logged mode that have been found in the book
		if (book_) {
			text_values = book_->used_submodes();
			for (auto it = text_values.begin(); it != text_values.end(); it++) {
				string text = *it;
				bool done = false;
				for (size_t i2 = 0; i2 < colours_used_.size() && !done; i2++) {
					// insert it if it's less than the current entry 
					if (text < colours_used_[i2]) {
						colours_used_.insert(colours_used_.begin() + i2, text);
						done = true;
					}
				}
				if (!done) {
					colours_used_.push_back(text);
				}
			}
		}
		break;
	case AC_AWARDMODE:
		// Colour by DXCC Award mode - first get all modes that have been used in the book
		if (book_) {
			text_values = book_->used_modes();
			for (auto it = text_values.begin(); it != text_values.end(); it++) {
				// Convert it to award mode
				string text = spec_data_->dxcc_mode(*it);
				bool done = false;
				for (size_t i2 = 0; i2 < colours_used_.size() && !done; i2++) {
					// insert it if it's less than the current entry 
					if (text < colours_used_[i2]) {
						colours_used_.insert(colours_used_.begin() + i2, text);
						done = true;
					}
					else if (text == colours_used_[i2]) {
						done = true;
					}
				}
				if (!done) {
					colours_used_.push_back(text);
				}
			}
		}
		break;
	case AC_DISTANCE:
		// Colour by distance
		for (int i = 1; i < (int)(EARTH_RADIUS * PI) / 1000; i ++) {
			string value = "<" + to_string(i * 1000) + "km";
			colours_used_.push_back(value);
		}
		break;
	case AC_DAYS:
		for (int i = 0; i < 10; i++) {
			string value = to_string(i) + " days";
			colours_used_.push_back(value);
		}
		colours_used_.push_back(">=10 days");
		break;
	}
}


// Draw pins
void dxa_if::draw_pins() {
	if (atlas_) {
		// Put up the hour-glass
		fl_cursor(FL_CURSOR_WAIT);
		try {
			// deactivate all colour buttons
			for (size_t i = 0; i < colour_bns_.size(); i++) {
				Fl_Color inactive_colour = fl_color_average(button_colour(i), FL_WHITE, 0.5f);
				colour_bns_[i]->color(inactive_colour);
				colour_bns_[i]->labelcolor(fl_contrast(FL_BLACK, inactive_colour));
				colour_bns_[i]->deactivate();
			}
			_variant_t pt_lat, pt_long, pt_value, pt_text;
			_variant_t point, points;
			// Set these "variant_t" to single precision floating point
			pt_lat.ChangeType(VT_R4);
			pt_long.ChangeType(VT_R4);
			pt_value.ChangeType(VT_R4);
			// Set these to arrays of variants.
			point.vt = VT_ARRAY | VT_VARIANT;
			points.vt = VT_ARRAY | VT_VARIANT;

			// Get map
			DxAtlas::IDxMapPtr map = atlas_->GetMap();
			// It is necessary to have Begin/EndUpdate around deleting the existing data to
			// allow DxAtlas to implement it before reapplying the data - this may be an artifice
			// of FLTK scheduling
			map->BeginUpdate();
			//delete all existing layers - set call_layer_ to nullptr to prevent callbacks from using it
			call_layer_ = nullptr;
			DxAtlas::ICustomLayersPtr pin_layers = map->GetCustomLayers();
			pin_layers->Clear();
			map->EndUpdate();
			status_->misc_status(ST_NOTE, "DXATLAS: Update started");
			status_->progress(records_to_display_.size(), OT_DXATLAS, "Displaying records on DxAtlas", "records");
			int count = 0;
			// Set the bounds for zooming - always include the home location, these will get when stations are further W, E, N or S
			westernmost_ = home_long_;
			easternmost_ = home_long_;
			// Get most recent date (for AC_DAYS)
			northernmost_ = home_lat_;
			southernmost_ = home_lat_;
			// Set smallest distance to a practicable limit to avoid zero divide if no pins to draw
			furthest_ = 100;
			last_time_ = time(nullptr);

			// Clear records displayed
			records_displayed_.clear();
			dxatlas_records_->clear_criteria(false);
			int* colour_count = new int[colours_used_.size()];
			for (size_t i = 0; i < colours_used_.size(); i++) {
				colour_count[i] = 0;
			}
			// Create a set of colour names;
			set<string> colours_used;
			colours_used.clear();

			// For all the wanted colours
			for (int colour_layer = 0; colour_layer < (signed)colours_used_.size(); colour_layer++) {
				// Get the text associated with this colour
				string colour_text;
				if (colour_layer < (signed)colours_used_.size()) {
					colour_text = colours_used_[colour_layer];
				}
				else {
					colour_text = "";
				}
				// Only check if we hav enabled colour
				if (colour_enables_[colour_layer]) {
					// do not repaint map while we are updating data
					// This will repaint the map between each colour
					map->BeginUpdate();

					// add new layer for points - allow lower layers to be visible
					DxAtlas::ICustomLayerPtr layer = pin_layers->Add(DxAtlas::LK_POINTS);
					layer->PutLabelsTransparent(true);

					// create 1-D array of labels - use the maximum possible array size
					SAFEARRAYBOUND rgsabound[1];
					rgsabound[0].lLbound = 0;
					rgsabound[0].cElements = records_to_display_.size();

					// The colour of points to display
					DxAtlas::EnumColor colour = convert_colour(button_colour(colour_layer));

					// Array to hold all the points
					SAFEARRAY* point_array = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
					long index_2 = 0;

					// For each record check it is this colour and whether to display it
					for (auto it2 = records_to_display_.begin(); it2 != records_to_display_.end(); it2++) {
						// Go through all the selected records
						record_num_t record_num = *it2;
						long index_1 = 0;
						record* record = book_->get_record(record_num, false);
						lat_long_t lat_long;
						SAFEARRAY* point_data;
						bool use_item = false;
						string item;
						if (record != nullptr) {
							// Select on 'colour by' mode
							switch (atlas_colour_) {
							case AC_NONE:
								// 1 colour - use every record
								use_item = true;
								break;
							case AC_BANDS:
								// select record if it's the band we are drawing
								use_item = (colour_text == record->item("BAND"));
								break;
							case AC_LOGMODE:
								// select record if it's the ADIF submode we are drawing
								use_item = (colour_text == record->item("MODE", true));
								break;
							case AC_AWARDMODE:
								// select record if it's the DXCC award mode we are drawing
								item = record->item("MODE");
								use_item = (colour_text == spec_data_->dxcc_mode(item));
								break;
							case AC_DISTANCE:
								use_item = (colour_text == get_distance(record));
								break;
							case AC_DAYS:
								if (colour_layer < 10) {
									use_item = (((long)difftime(last_time_, record->timestamp()) / (24 * 3600)) == colour_layer);
								}
								else {
									use_item = (((long)difftime(last_time_, record->timestamp()) / (24 * 3600)) >= colour_layer);
								}
								break;
							}
	
							// Don't use item if disabled
							if (use_item) {
								if (include_swl_ || record->item("SWL") != "Y") {
									// Add the colour name to the used list
									colours_used.insert(colour_text);
									// This record is to be drawn in this layer
									lat_long = record->location(false);
									// Only add if the location is valid
									if (!isnan(lat_long.latitude) && !isnan(lat_long.longitude)) {
										//create label entry (array of 3 elements: Long, Lat, Text)
										rgsabound[0].cElements = 3;
										point_data = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
										//calculate attributes - explicitly convert from double to single precision
										pt_long = (float)lat_long.longitude;
										pt_lat = (float)lat_long.latitude;
										// Adjust furthest in each direction
										if (lat_long.longitude < westernmost_) westernmost_ = lat_long.longitude;
										if (lat_long.longitude > easternmost_) easternmost_ = lat_long.longitude;
										if (lat_long.latitude < southernmost_) southernmost_ = lat_long.latitude;
										if (lat_long.latitude > northernmost_) northernmost_ = lat_long.latitude;
										pt_value = 0;
										//set attributes 
										index_1 = 0; (void)SafeArrayPutElement(point_data, &index_1, &pt_long);
										index_1 = 1; (void)SafeArrayPutElement(point_data, &index_1, &pt_lat);
										index_1 = 2; (void)SafeArrayPutElement(point_data, &index_1, &pt_value);
										//add point to the array
										point.parray = point_data;
										(void)SafeArrayPutElement(point_array, &index_2, &point);
										index_2++;
										// Adjust furthest away;
										int distance = 0;
										record->item("DISTANCE", distance);
										if (distance > furthest_) {
											furthest_ = distance;
										}
										// Add it to the set of records being displayed
										records_displayed_.insert(record_num);
										dxatlas_records_->add_record(record_num);
									}
								}
								count += 1;
								colour_count[colour_layer]++;
								status_->progress(count, OT_DXATLAS);
							}
						}
					}
					// Don't attempt to draw an empty set of points
					if (index_2 > 0) {
						// Resize array if fewer items have been created.
						if (index_2 != records_to_display_.size()) {
							rgsabound[0].cElements = index_2;
							SafeArrayRedim(point_array, rgsabound);
						}
						// Set point size and paint colour
						layer->PutBrushColor(colour);
						layer->PutPointSize(pin_size_);
						// font/line color
						layer->PutPenColor(DxAtlas::clBlack);
						// put data into the layer
						points.parray = point_array;
						// now put the data onto the DXATLAS: map and display an error if it failed
						try {
							layer->SetData(points);
						}
						catch (_com_error& e) {
							char error[256];
							sprintf(error, "DXATLAS: Got error displaying data : %s", e.ErrorMessage());
							status_->misc_status(ST_ERROR, error);
						}
					}
					// now allow repainting
					map->EndUpdate();
				}
			}
			// now add a layer for textual information - used for interactive displays
			call_layer_ = pin_layers->Add(DxAtlas::LK_LABELS);
			// Make it opaque so that the label is visible against the background
			call_layer_->LabelsTransparent = false;
			// font attributes for this layer - default size, courier new, navy text on a white background
			IFontPtr pFont = call_layer_->GetFont();
			pFont->put_Name(_com_util::ConvertStringToBSTR("Courier New"));
			call_layer_->PenColor = DxAtlas::clNavy;
			call_layer_->BrushColor = DxAtlas::clWhite;

			// now centre on selected record
			switch(map->GetProjection()) {
			case DxAtlas::PRJ_RECTANGULAR:
				centre_map();
				break;
			case DxAtlas::PRJ_AZIMUTHAL:
				zoom_azimuthal();
				break;
			}

			// Set the colours of the buttons to match that of the points on the map - and a paler one for unused colours
			bool set_pz_colour = false;
			for (size_t i = 0; i < colour_bns_.size(); i++) {
				if (colours_used.find(colours_used_[i]) == colours_used.end()) {
					Fl_Color inactive_colour = fl_color_average(button_colour(i), FL_WHITE, 0.5f);
					colour_bns_[i]->color(inactive_colour);
					colour_bns_[i]->labelcolor(fl_contrast(FL_BLACK, inactive_colour));
				}
				else {
					Fl_Color active_colour = button_colour(i);
					colour_bns_[i]->color(active_colour);
					colour_bns_[i]->labelcolor(fl_contrast(FL_BLACK, active_colour));
					// Set the pinsize colour to the first enabled colour
					if (!set_pz_colour) {
						set_pz_colour = true;
						pz_w_->color(button_colour(i));
					}
				}
			}
			// Now deactivate all colour buttons
			for (auto it = colour_bns_.begin(); it != colour_bns_.end(); it++) {
				(*it)->deactivate();
			}
			// And activate those buttons used
			for (auto it = colours_used.begin(); it != colours_used.end(); it++) {
				(button_map_.at(*it))->activate();
			}

			//// Now display the list of records
			//if (records_displayed_.size()) {
			//	book_->selection(*(records_displayed_.begin()), HT_DXATLAS);
			//}

			// Report finished
			status_->misc_status(ST_OK, "DXATLAS: Update done!");
			if (count != records_to_display_.size()) {
				char message[100];
				snprintf(message, 100, "DXATLAS: %d record(s) not displayed", records_to_display_.size() - count);
				status_->misc_status(ST_WARNING, message);
				for (size_t i = 0; i < colours_used_.size(); i++) {
					snprintf(message, 100, "DXATLAS: %s - %d records", colours_used_[i].c_str(), colour_count[i]);
					status_->misc_status(ST_LOG, message);
				}
				status_->progress("Not all records displayed", OT_DXATLAS);
			}
			else if (count == 0) {
				status_->progress("No records to display", OT_DXATLAS);
			}

			char label[30];
			snprintf(label, 30, "%d QSOs displayed", count);
			((Fl_Choice*)qso_count_)->copy_label(label);
		}
		catch (exception& /*e*/) {
			status_->misc_status(ST_SEVERE, "DXATLAS: Error detected during update");
			status_->progress("Errored", OT_DXATLAS);
			disconnect_dxatlas(false);
		}
		redraw();
		fl_cursor(FL_CURSOR_DEFAULT);
	}
}

// draw home flag
void dxa_if::draw_home_flag() {
	if (atlas_) {
		// Set home location
		atlas_->GetMap()->PutHomeLatitude((float)home_lat_);
		atlas_->GetMap()->PutHomeLongitude((float)home_long_);
	}
}

// Get colours and initialse them - if reset then removes any existing filter
void dxa_if::get_colours(bool reset) {
	allocate_colours();
	create_colour_buttons();
}

// something has changed in the book that requires DxAtlas to redraw
void dxa_if::update(hint_t hint) {
	switch(hint) {
	case HT_ALL:
	case HT_CHANGED:
	case HT_INSERTED:
	case HT_DELETED:
	case HT_DUPE_DELETED:    
	case HT_NEW_DATA:
	case HT_NO_DATA:
		// Data has changed
		if (book_->size()) {
			selected_locn_ = book_->get_record()->location(false);
		}
		enable_widgets();
		get_records();
		get_colours(false);
		draw_pins();
		break;

	case HT_SELECTED:
		if (book_->size()) {
			selected_locn_ = book_->get_record()->location(false);
		}
		// Only redraw if we are displaying selected or centring on selected
		if (centre_mode_ == SELECTED || qso_display_ == AQ_CURRENT) {
			// Data has changed 
			enable_widgets();
			get_records();
			get_colours(false);
			draw_pins();
		}
		break;

	case HT_EXTRACTION:
		// The search results have changed
		if (book_->size()) {
			selected_locn_ = book_->get_record()->location(false);
		}
		if (qso_display_ == AQ_SEARCH) {
			// Data has changed 
			enable_widgets();
			get_records();
			get_colours(false);
			draw_pins();
		}
		break;

	case HT_LOCATION:
		// The home location may have changed - redraw pins as we may need to re-zoom
		load_values();
		update_loc_widgets();
		draw_home_flag();
		draw_pins();
		break;
		
	case HT_FORMAT:
		// We may have changed session start
		if (qso_display_ == AQ_SESSION) {
			// Data has changed 
			enable_widgets();
			get_records();
			get_colours(false);
			draw_pins();
		}
		break;

	default:
		// HT_MINOR_CHANGE,          // Invalidate the record, but not location, band or mode - don't redraw
		// HT_IMPORT_QUERY,          // Import record cannot be processed without user intervention - 
		// HT_IMPORT_QUERYNEW,       // Query whether mismatch is a new record
		// HT_DUPE_QUERY,            // Query whether records are duplicates
		// HT_STARTING,              // Record is being created as HT_INSERTED but don't include it
		// HT_DXATLAS                // Change originated here - avoid endless loop
		// Do nothing 
		break;
	}
}

// Centre the map on the specified location
void dxa_if::centre_map(lat_long_t centre) {
	DxAtlas::IDxMapPtr map = atlas_->GetMap();
	map->PutCenterLatitude((float)centre.latitude);
	map->PutCenterLongitude((float)centre.longitude);
	centre_lat_ = map->GetCenterLatitude();
	centre_long_ = map->GetCenterLongitude();
}

// Zoom the map on the specified centre to cover whole world (full) or just drawn points
void dxa_if::zoom_centre(lat_long_t centre, bool full) {
	DxAtlas::IDxMapPtr map = atlas_->GetMap();
	if (full) {
		// Zoom whole map
		map->PutZoom(1);
	} else {
		// Zoom to include all records - get the furthermost from the centre E/W and N/S
		double zoom_long = 180. / (max(easternmost_ - centre.longitude, centre.longitude - westernmost_));
		double zoom_lat = 90. / (max(northernmost_ - centre.latitude, centre.latitude - southernmost_));
		// now zoom by the smaller of these with 5% margin
		map->PutZoom((float)min(zoom_long, zoom_lat) * 0.95f);
	}
	// Read the actual amount zoomed
	zoom_value_ = map->GetZoom();
}

// Zoom the map in azimuthal mode
void dxa_if::zoom_azimuthal() {
	DxAtlas::IDxMapPtr map = atlas_->GetMap();
	float zoom;
	switch (centre_mode_) {
	case HOME:
		// Zoom in to just the displayed QSOs
		zoom = (float)((EARTH_RADIUS * PI) / furthest_ * 0.95);
		map->PutZoom(zoom);
		break;
	case ZERO:
		// Zoom full size
		map->PutZoom(1.0);
		break;
	}
}

// Centre the map according to the settings
void dxa_if::centre_map() {
	lat_long_t centre;
	double save_n;
	double save_s;
	double save_w;
	double save_e;
	switch (centre_mode_) {
	case HOME:
		// Centre on the home location
		centre = { (double)home_lat_, (double)home_long_ };
		centre_map(centre);
		zoom_centre(centre, false);
		break;
	case SELECTED:
		// Centre on the selected record
		centre = selected_locn_;
		centre_map(centre);
		zoom_centre(centre, false);
		break;
	case GROUP:
		// Centre so that the group is displayed evenly
		centre = { (northernmost_ + southernmost_) * 0.5, (westernmost_ + easternmost_) * 0.5 };
		centre_map(centre);
		zoom_centre(centre, false);
		break;
	case ZERO:
		// Centre at 0 N 0 W
		centre = { 0.0, 0.0 };
		centre_map(centre);
		zoom_centre(centre, true);
		break;
	default:
		// Display an entire continent
		save_n = northernmost_;
		save_s = southernmost_;
		save_w = westernmost_;
		save_e = easternmost_;
		switch (centre_mode_) {
		case EU:
			northernmost_ = 60.0;
			southernmost_ = 35.0;
			westernmost_ = -30.0;
			easternmost_ = 65.0;
			break;
		case AS:
			northernmost_ = 80.0;
			southernmost_ = -10.0;
			westernmost_ = 20.0;
			easternmost_ = 205.0;
			break;
		case AF:
			northernmost_ = 40.0;
			southernmost_ = -35.0;
			westernmost_ = -25.0;
			easternmost_ = 55.0;
			break;
		case NA:
			northernmost_ = 80.0;
			southernmost_ = 10.0;
			westernmost_ = -155.0;
			easternmost_ = -20.0;
			break;
		case SA:
			northernmost_ = 15.0;
			southernmost_ = -55.0;
			westernmost_ = -95.0;
			easternmost_ = -30.0;
			break;
		case OC:
			northernmost_ = 30.0;
			southernmost_ = -55.0;
			westernmost_ = 120.0;
			easternmost_ = 245.0;
			break;
		case AN:
			northernmost_ = -40.0;
			southernmost_ = -90.0;
			westernmost_ = -180.0;
			easternmost_ = 180.0;
			break;
		}
		centre = { (northernmost_ + southernmost_) * 0.5, (westernmost_ + easternmost_) * 0.5 };
		centre_map(centre);
		zoom_centre(centre, false);
		// Restore bounds
		northernmost_ = save_n;
		southernmost_ = save_s;
		westernmost_ = save_w;
		easternmost_ = save_e;
		break;
	}
	save_values();
}

// Change the text on the dialog depending on the colour-by selection
void dxa_if::label_colour_grp() {
	switch (atlas_colour_) {
	case AC_NONE:
		colour_grp_->label("Colour legend");
		break;
	case AC_BANDS:
		colour_grp_->label("Colour legend - band");
		break;
	case AC_LOGMODE:
		colour_grp_->label("Colour legend - logged mode");
		break;
	case AC_AWARDMODE:
		colour_grp_->label("Colour legend - award mode");
		break;
	case AC_DISTANCE:
		colour_grp_->label("Colour legend - distance");
		break;
	}
}

string dxa_if::get_distance(record* this_record) {
	int distance;
	this_record->item("DISTANCE", distance);
	distance = (((distance - 1) / 1000) + 1) * 1000;
	string result = "<" + to_string(distance) + "km";
	return result;
}

// Add the Dx location and include it in the displayed group
void dxa_if::set_dx_loc(string location) {
	lat_long_t lat_long = grid_to_latlong(location);
	DxAtlas::IDxMapPtr map = atlas_->GetMap();
	map->PutDxLatitude((float)lat_long.latitude);
	map->PutDxLongitude((float)lat_long.longitude);
	map->PutDxVisible(true);
	if (lat_long.latitude > northernmost_) northernmost_ = lat_long.latitude;
	if (lat_long.latitude < southernmost_) southernmost_ = lat_long.latitude;
	if (lat_long.longitude > easternmost_) easternmost_ = lat_long.longitude;
	if (lat_long.longitude < westernmost_) westernmost_ = lat_long.longitude;
	double bearing;
	double distance;
	great_circle({ home_lat_, home_long_ }, lat_long, bearing, distance);
	if (distance > furthest_) furthest_ = (int)distance;
	centre_map();
}

// REmove the Dx Location
void dxa_if::clear_dx_loc() {
	DxAtlas::IDxMapPtr map = atlas_->GetMap();
	map->PutDxVisible(false);
}

// Create a widget to contain the pin size indication - draw a null box with a circle the correct size (denoted by value)
dxa_if::pz_widget::pz_widget(int X, int Y, int W, int H) :
	Fl_Widget(X, Y, W, H, nullptr)
{
	box(FL_NO_BOX);
}

// Draw the widget
void dxa_if::pz_widget::draw() {
	int x_centre = x() + (w() / 2);
	int y_centre = y() + (h() / 2);
	// Limit the drawing to the widget
	fl_push_clip(x(), y(), w(), h());
	// Create the solid coloured dot - value() supplies the radious of the dot.
	fl_color(color());
	fl_pie(x_centre - value(), y_centre - value(), value() + value(), value() + value(), 0.0, 360.0);
	// Draw the outer circle over it.
	fl_color(FL_BLACK);
	fl_circle(x_centre, y_centre, value());


	fl_pop_clip();
}

// Value set and get
void dxa_if::pz_widget::value(int v) {
	value_ = v;
}

int dxa_if::pz_widget::value() { return value_; }

#endif // _WIN32
