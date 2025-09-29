#include "condx_view.h"

#include "qso_manager.h"
#include "qso_wx.h"
#include "status.h"
#include "ticker.h"
#include "url_handler.h"

#include "drawing.h"
#include "pugixml.hpp"
#include "utils.h"

#include <ctime>
#include <sstream>
#include <string>

#include <FL/Fl_Box.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tabs.H>

extern bool DARK;
extern qso_manager* qso_manager_;
extern status* status_;
extern std::string default_data_directory_;
extern std::string VENDOR;
extern std::string PROGRAM_ID;
extern ticker* ticker_;
extern url_handler* url_handler_;
extern void open_html(const char*);

Fl_Color COLOUR_BAD = FL_RED;         //!< Use for bad stuff
Fl_Color COLOUR_FAIR = FL_FOREGROUND_COLOR;
//!< Use for fair stuff
Fl_Color COLOUR_GOOD = FL_DARK_GREEN;      //!< Use for good stuff
Fl_Color COLOYR_NOTE = FL_BLUE;       //!< Use for extra annotation


//! Constructor

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
condx_view::condx_view(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, data_(nullptr)
	, solar_image_(nullptr)
{
	if (DARK) COLOUR_GOOD = FL_GREEN;
	if (load_data()) {
//		load_image();
		create_form();
		enable_widgets();
	}
	// Set a time to update every hour
	ticker_->add_ticker(this, cb_ticker, 360000, false);
}

//! Destructor
condx_view::~condx_view() {

}

int condx_view::handle(int event) {
	int result = Fl_Group::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		if (!result) take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("condx_view.html");
			return true;
		}
		break;
	}
	return result;
}


//! Load data
bool condx_view::load_data() {
	char msg[128];
	pugi::xml_document doc;
	std::string filename = default_data_directory_ + "solar.xml";

	// Check if saved file is > 1 hour old
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	int temp;
	settings.get("Solar Timestamp", temp, 0);
	time_t when = temp;
	time_t now = time(nullptr);
	if (difftime(now, when) > 3600) {
		// Download fresh data
		status_->misc_status(ST_NOTE, "SOLAR: Downloading solar data");
		// Copy file to data
		std::stringstream ss;
		if (!fetch_data(ss)) {
			status_->misc_status(ST_ERROR, "SOLAR: Download failed");
			return false;
		}
		doc.load(ss);
		// Save file
		doc.save_file(filename.c_str());
		settings.set("Solar Timestamp", (int)now);
	}
	else {
		// Load from old data
		status_->misc_status(ST_NOTE, "SOLAR: Loading previous data");
		doc.load_file(filename.c_str());
	}
	// 

	pugi::xml_node top = doc.document_element();
	if (strcmp(top.name(), "solar") != 0) {
		snprintf(msg, sizeof(msg), "SOLAR: Invalid data downloaded");
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
	pugi::xml_node n_solar = top.child("solardata");
	data_ = new solar_data;
	data_->update = n_solar.child("updated").text().as_string();
	data_->solar_flux_index = n_solar.child("solarflux").text().as_int();
	data_->A_index = n_solar.child("aindex").text().as_int();
	data_->K_index = n_solar.child("kindex").text().as_int();
	data_->x_ray = n_solar.child("xray").text().as_string();
	data_->num_sunpsots = n_solar.child("sunspots").text().as_int();
	data_->He_strengh = n_solar.child("heliumline").text().as_double();
	data_->proton_flux = n_solar.child("protonflux").text().as_int();
	data_->electron_flux = n_solar.child("electonflux").text().as_int();
	data_->aurora = n_solar.child("aurora").text().as_string();
	data_->aurora_latitude = n_solar.child("latdegree").text().as_double();
	data_->solar_wind_vel = n_solar.child("solarwind").text().as_double();
	data_->mag_delta = n_solar.child("magneticfield").text().as_double();
	data_->mag_field = n_solar.child("geomagfield").text().as_string();
	data_->solar_noise = n_solar.child("signalnoise").text().as_string();
	// HF band conditions
	pugi::xml_node n_hf = n_solar.child("calculatedconditions");
	for (auto n_band : n_hf.children()) {
		std::string band = n_band.attribute("name").as_string();
		std::string ttime = n_band.attribute("time").as_string();
		data_->hf_forecasts[band][ttime] = n_band.text().as_string();
	}
	// VHF band phenonema
	pugi::xml_node n_vhf = n_solar.child("calculatedvhfconditions");
	for (auto n_phen : n_vhf.children()) {
		string phenomenon = "";
		string phen_name = n_phen.attribute("name").value();
		string phen_loc = n_phen.attribute("location").value();
		if (phen_name == "vhf-aurora") {
			if (phen_loc == "northern_hemi") {
				phenomenon = "Aurora (Bor.)";
			}
		}
		else if (phen_name == "E-Skip") {
			if (phen_loc == "europe") {
				// Sporadic E - E<sub>s</sub>
				phenomenon = "Es Europe";
			}
			else if (phen_loc == "north_america") {
				phenomenon = "Es N.America";
			}
			else if (phen_loc == "europe_6m") {
				phenomenon = "Es Europe (6m)";
			}
			else if (phen_loc == "europe_4m") {
				phenomenon = "Es Europe (4m)";
			}
		}
		if (phenomenon.length()) {
			data_->vhf_forecasts[phenomenon] = n_phen.text().as_string();
		}
		else {
			snprintf(msg, sizeof(msg), "SOLAR: Unexpected VHF phenomenon. Name=%s, location=%s",
				phen_name.c_str(), phen_loc.c_str());
			status_->misc_status(ST_WARNING, msg);
		}
	}
	status_->misc_status(ST_OK, "SOLAR: Solar data loaded OK");
	return true;
}

//! Instantiate component widgets
void condx_view::create_form() {
	box(FL_BORDER_BOX);
	w_tabs_ = new Fl_Tabs(x(), y(), w(), h() - HBUTTON);
	w_tabs_->labeltype(FL_NO_LABEL);
	w_tabs_->box(FL_BORDER_BOX);
	w_tabs_->handle_overflow(Fl_Tabs::OVERFLOW_PULLDOWN);
	w_tabs_->callback(cb_tabs);
	int rx = 0, ry = 0, rw = 0, rh = 0;
	w_tabs_->client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;

	create_hf(rx, ry, rw, rh);
	rw = max(rw, g_hf_->w());
	rh = max(rh, g_hf_->h());

	create_vhf(rx, ry, rw, rh);
	rw = max(rw, g_vhf_->w());
	rh = max(rh, g_vhf_->h());

	create_solar(rx, ry, rw, rh);
	rw = max(rw, g_solar_->w());
	rh = max(rh, g_solar_->h());

	create_geomag(rx, ry, rw, rh);
	rw = max(rw, g_geo_->w());
	rh = max(rh, g_geo_->h());

	//create_image(rx, ry, rw, rh);
	//rw = max(rw, g_image_->w());
	//rh = max(rh, g_image_->h());

	// Resize tabs accordingly
	w_tabs_->resizable(nullptr);
	w_tabs_->size(w_tabs_->w() + rw - saved_rw, w_tabs_->h() + rh - saved_rh);
	w_tabs_->end();

	for (int ix = 0; ix < w_tabs_->children(); ix++) {
		w_tabs_->child(ix)->size(rw, rh);
	}

	int cx = x() + GAP;
	int cy = w_tabs_->y() + w_tabs_->h();

	w_updated_ = new Fl_Box(cx, cy, w() - GAP - GAP, HBUTTON - 1);
	w_updated_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	w_updated_->box(FL_FLAT_BOX);
	w_updated_->labelsize(FL_NORMAL_SIZE - 1);

	resizable(nullptr);
	
	cy += HBUTTON;
	size(w(), cy - y());

	end();

}

//! Enable widgets - copy data to them
void condx_view::enable_widgets() {
	char text[128];
	// Set standard tab label formats
	for (int ix = 0; ix < w_tabs_->children(); ix++) {
		Fl_Widget* wx = w_tabs_->child(ix);
		if (wx == w_tabs_->value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
	}

	// "Solar" tab
	// SFI
	snprintf(text, sizeof(text), "%d", data_->solar_flux_index);
	w_sfi_->value(text);
	if (data_->solar_flux_index > 150) w_sfi_->textcolor(COLOUR_GOOD);
	else if (data_->solar_flux_index > 90) w_sfi_->textcolor(COLOUR_FAIR);
	else w_sfi_->textcolor(COLOUR_BAD);
	// A-index
	snprintf(text, sizeof(text), "%d", data_->A_index);
	w_aix_->value(text);
	// K-index
	snprintf(text, sizeof(text), "%d", data_->K_index);
	w_kix_->value(text);
	if (data_->K_index > 7) w_kix_->textcolor(COLOUR_BAD);
	else if (data_->K_index > 3) w_kix_->textcolor(COLOUR_FAIR);
	else w_kix_->textcolor(COLOUR_GOOD);
	// X-ray
	w_xray_->value(data_->x_ray.c_str());
	if (data_->x_ray[0] == 'X') w_xray_->textcolor(COLOUR_BAD);
	else if (data_->x_ray[0] == 'M') w_xray_->textcolor(COLOUR_FAIR);
	else w_xray_->textcolor(COLOUR_GOOD);
	//Sunspots
	snprintf(text, sizeof(text), "%d", data_->num_sunpsots);
	w_sunspots_->value(text);
	// 304A
	snprintf(text, sizeof(text), "%g", data_->He_strengh);
	w_helium_->value(text);
	// Proton flux
	snprintf(text, sizeof(text), "%d", data_->proton_flux);
	w_proton_->value(text);
	if (data_->proton_flux > 100000) w_proton_->textcolor(COLOUR_BAD);
	else if (data_->proton_flux > 100) w_proton_->textcolor(COLOUR_FAIR);
	else w_proton_->textcolor(COLOUR_GOOD);
	// Electron flux
	snprintf(text, sizeof(text), "%d", data_->electron_flux);
	w_electron_->value(text);
	if (data_->electron_flux > 1000) w_electron_->textcolor(COLOUR_BAD);
	else if (data_->electron_flux > 100) w_electron_->textcolor(COLOUR_FAIR);
	else w_electron_->textcolor(COLOUR_GOOD);
	// Solar wind
	snprintf(text, sizeof(text), "%g km s\342\201\273\302\271", data_->solar_wind_vel);
	w_wind_->value(text);
	if (data_->solar_wind_vel > 700) w_wind_->textcolor(COLOUR_BAD);
	else if (data_->solar_wind_vel > 400) w_wind_->textcolor(COLOUR_FAIR);
	else w_wind_->textcolor(COLOUR_GOOD);

	// "Geomag:" tab
	// Aurora
	w_aurora_->value(data_->aurora.c_str());
	if (data_->aurora >= "10") w_aurora_->textcolor(COLOUR_BAD);
	else if (data_->aurora >= "8") w_aurora_->textcolor(COLOUR_FAIR);
	else w_aurora_->textcolor(COLOUR_GOOD);
	// Aurora latitude
	snprintf(text, sizeof(text), "%0.1f\302\260 %c",
		data_->aurora_latitude,
		data_->aurora_latitude > 0 ? 'N' : 'S');
	w_aur_lat_->value(text);
	// Bz
	snprintf(text, sizeof(text), "%g", data_->mag_delta);
	w_bz_->value(text);
	if (data_->mag_delta < -20.) w_bz_->textcolor(COLOUR_BAD);
	else if (data_->mag_delta < 0) w_bz_->textcolor(COLOUR_FAIR);
	else w_bz_->textcolor(COLOUR_GOOD);
	// Geomag
	w_geomag_->value(data_->mag_field.c_str());
	// Solar noise
	w_noise_->value(data_->solar_noise.c_str());

	// "HF" tab
	// Band daytime condiitons
	time_t now = time(nullptr);
	if (!qso_manager_ || qso_manager_->wx()->is_day(now)) {
		g_day_->labelfont(FL_BOLD);
	}
	else {
		g_day_->labelfont(FL_ITALIC);
	}
	for (auto b : data_->hf_forecasts) {
		if (b.second.find("day") != b.second.end()) {
			w_day_condx_.at(b.first)->value(b.second.at("day").c_str());
			w_day_condx_.at(b.first)->textfont(g_day_->labelfont() & FL_ITALIC);
			if (b.second.at("day") == "Good")
				w_day_condx_.at(b.first)->textcolor(COLOUR_GOOD);
			else if (b.second.at("day") == "Poor")
				w_day_condx_.at(b.first)->textcolor(COLOUR_BAD);
			else w_day_condx_.at(b.first)->textcolor(COLOUR_FAIR);
		}			
	}
	// Band night-time condiiton
	if (!qso_manager_ || qso_manager_->wx()->is_night(now)) {
		g_night_->labelfont(FL_BOLD);
	}
	else {
		g_night_->labelfont(FL_ITALIC);
	}
	g_night_->activate();
	for (auto b : data_->hf_forecasts) {
		if (b.second.find("night") != b.second.end()) {
			w_night_condx_.at(b.first)->value(b.second.at("night").c_str());
			w_night_condx_.at(b.first)->textfont(g_night_->labelfont() & FL_ITALIC);
			if (b.second.at("night") == "Good")
				w_night_condx_.at(b.first)->textcolor(COLOUR_GOOD);
			else if (b.second.at("night") == "Poor")
				w_night_condx_.at(b.first)->textcolor(COLOUR_BAD);
			else w_night_condx_.at(b.first)->textcolor(COLOUR_FAIR);
		}
	}

	// "VHF" tab
	// vHF phenomena
	for (auto b : data_->vhf_forecasts) {
		w_vhf_phenomena_.at(b.first)->value(b.second.c_str());
		if (b.second == "Band Closed") w_vhf_phenomena_.at(b.first)->textcolor(COLOUR_BAD);
		else if (b.second.substr(0,4) == "High")
			w_vhf_phenomena_.at(b.first)->textcolor(COLOUR_FAIR);
		else  w_vhf_phenomena_.at(b.first)->textcolor(COLOUR_GOOD);
	}

	//// "Image" tab
	//w_image_->image(solar_image_);

	// Updated
	snprintf(text, sizeof(text), "Updated: %s", data_->update.c_str());
	w_updated_->copy_label(text);

	redraw();
}

//! Fetch new data (greater than 1 hour old)
bool condx_view::fetch_data(std::stringstream& ss) {
	// Fetch the solar data
	if (!url_handler_->read_url("https://www.hamqsl.com/solarxml.php", &ss)) {
		status_->misc_status(ST_ERROR, "SOLAR: Unable to download data");
		return false;
	}
	// Go the start of the stream again
	ss.seekg(0, ss.beg);
	return true;
}

//! Fetch the image
bool condx_view::load_image() {
	if (solar_image_) delete solar_image_;
	// Fetch the image
	std::stringstream ss;
	if (!url_handler_->read_url("https://www.hamqsl.com/mdi.gif", &ss)) {
		status_->misc_status(ST_WARNING, "SOLAR: Unable to download solar image");
		solar_image_ = nullptr;
		return false;
	}
	solar_image_ = 
		new Fl_GIF_Image(nullptr, (unsigned char*)ss.str().c_str(), ss.str().length());
	if (solar_image_->fail()) {
		status_->misc_status(ST_WARNING, "SOLAR: Unable to load solar image");
		delete solar_image_;
		solar_image_ = nullptr;
		ss.seekg(std::ios::beg);
		Fl_Help_Dialog* dlg = new Fl_Help_Dialog();
		dlg->value(ss.str().c_str());
		dlg->show();
		return false;
	}
	return true;
}

// Create individual tabs
void condx_view::create_solar(int x, int y, int w, int h) {
	g_solar_ = new Fl_Group(x, y, w, h, "Solar data");
	int cx = g_solar_->x() + GAP;
	int cy = g_solar_->y() + GAP;

	cx += WLLABEL;
	w_sfi_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Solar Flux Index");
	w_sfi_->box(FL_FLAT_BOX);
	w_sfi_->align(FL_ALIGN_LEFT);
	w_sfi_->tooltip("Solar Flux Index:\n Indicates F-layer ionisation:\n Higher SFI=>Higher MUF");

	cy += ROW_HEIGHT;
	w_aix_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "A index");
	w_aix_->box(FL_FLAT_BOX);
	w_aix_->align(FL_ALIGN_LEFT);
	w_aix_->tooltip("A-index:\n Daily average geomagnetic activity");

	cy += ROW_HEIGHT;
	w_kix_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "K index");
	w_kix_->box(FL_FLAT_BOX);
	w_kix_->align(FL_ALIGN_LEFT);
	w_kix_->tooltip("K-index:\n Instantaneous geomagnetic activity");

	cy += ROW_HEIGHT;
	w_xray_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "X-ray");
	w_xray_->box(FL_FLAT_BOX);
	w_xray_->align(FL_ALIGN_LEFT);
	w_xray_->tooltip("Hard X-rays:\n Strength of solar X-radiation\n Affects D-layer");

	cy += ROW_HEIGHT;
	w_sunspots_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Sunspots");
	w_sunspots_->box(FL_FLAT_BOX);
	w_sunspots_->align(FL_ALIGN_LEFT);
	w_sunspots_->tooltip("Normalised sunspot number");

	cy += ROW_HEIGHT;
	w_helium_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "304\303\205 (He)");
	w_helium_->box(FL_FLAT_BOX);
	w_helium_->align(FL_ALIGN_LEFT);
	w_helium_->tooltip("30.4 nm (304\303\205) radiation\n Helium spectral emission.");

	cy += ROW_HEIGHT;
	w_proton_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Proton Flux");
	w_proton_->box(FL_FLAT_BOX);
	w_proton_->align(FL_ALIGN_LEFT);
	w_proton_->tooltip("Proton Flux:\n Density of protons in solar wind\n Impacts E-layer");

	cy += ROW_HEIGHT;
	w_electron_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Electron Flux");
	w_electron_->box(FL_FLAT_BOX);
	w_electron_->align(FL_ALIGN_LEFT);
	w_electron_->tooltip("Electron Flux:\n Density of electrons in solar wind\n Impacts E-layer");

	cy += ROW_HEIGHT;
	w_wind_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Solar wind");
	w_wind_->box(FL_FLAT_BOX);
	w_wind_->align(FL_ALIGN_LEFT);
	w_wind_->tooltip("Solar wind speed:\n Velocity of solar patrticles in km/s\n Affects HF propagation");

	cy += ROW_HEIGHT + GAP;
	g_solar_->resizable(nullptr);
	g_solar_->size(GAP + GAP + WLABEL + WBUTTON, cy - g_solar_->y());
	g_solar_->end();
}

void condx_view::create_geomag(int x, int y, int w, int h) {
	g_geo_ = new Fl_Group(x, y, w, h, "Geo: data");
	int cx = g_geo_->x() + GAP + WLLABEL;
	int cy = g_geo_->y() + GAP;

	w_aurora_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Aurora Category");
	w_aurora_->box(FL_FLAT_BOX);
	w_aurora_->align(FL_ALIGN_LEFT);
	w_aurora_->tooltip("Aurora index:\n Indicates strength of F-layer ionisation (1 to 10, 10+, 10++)");

	cy += ROW_HEIGHT;

	w_aur_lat_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Latitude");
	w_aur_lat_->box(FL_FLAT_BOX);
	w_aur_lat_->align(FL_ALIGN_LEFT);
	w_aur_lat_->tooltip("Aurora latitude:\n Indicates the minimum latitude of the aurora event");

	cy += ROW_HEIGHT;

	w_bz_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Bz");
	w_bz_->box(FL_FLAT_BOX);
	w_bz_->align(FL_ALIGN_LEFT);
	w_bz_->tooltip("Bz component:\n Change in geomagnetic field caused by solar radiation.");

	cy += ROW_HEIGHT;

	w_geomag_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Geomag:");
	w_geomag_->box(FL_FLAT_BOX);
	w_geomag_->align(FL_ALIGN_LEFT);
	w_geomag_->tooltip("Geomagnetic field disruption\n Impact of solar radiation");

	cy += ROW_HEIGHT;
	w_noise_ = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT, "Solar noise");
	w_noise_->box(FL_FLAT_BOX);
	w_noise_->align(FL_ALIGN_LEFT);
	w_noise_->tooltip("Solar noise\n Background noise due to solar radiation");

	cy += GAP;
	g_geo_->resizable(nullptr);
	g_geo_->size(GAP + GAP + WLABEL + WBUTTON, cy - g_geo_->y());
	g_geo_->end();
}

void condx_view::create_hf(int x, int y, int w, int h) {
	g_hf_ = new Fl_Group(x, y, w, h, "HF");
	int cx = g_hf_->x() + GAP;
	int cy = g_hf_->y() + HTEXT;

	// Create band titles
	for (auto& b : data_->hf_forecasts) {
		Fl_Box* bx = new Fl_Box(cx, cy, WLABEL, ROW_HEIGHT);
		bx->box(FL_FLAT_BOX);
		bx->copy_label(b.first.c_str());
		cy += ROW_HEIGHT;
	}

	cx += WLABEL;
	cy = g_hf_->y() + HTEXT;
	g_day_ = new Fl_Group(cx, cy, WBUTTON, 4 * ROW_HEIGHT, "Day");
	g_day_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	g_day_->tooltip("Daytime conditions\n: Summary for each HF band");

	w_day_condx_.clear();
	for (auto b : data_->hf_forecasts) {
		Fl_Output* o = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT);
		o->box(FL_FLAT_BOX);
		w_day_condx_[b.first] = o;
		cy += ROW_HEIGHT;
	}
	g_day_->end();
	cx += WBUTTON;
	cy = g_hf_->y() + HTEXT;
	g_night_ = new Fl_Group(cx, cy, WBUTTON, 4 * ROW_HEIGHT, "Night");
	g_night_->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
	g_night_->tooltip("Nighttime conditions\n: Summary for each HF band");
	w_night_condx_.clear();
	for (auto b : data_->hf_forecasts) {
		Fl_Output* o = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT);
		o->box(FL_FLAT_BOX);
		w_night_condx_[b.first] = o;
		cy += ROW_HEIGHT;
	}
	g_night_->end();

	g_hf_->resizable(nullptr);
	g_hf_->size(GAP + GAP + WLLABEL + WBUTTON, cy - g_hf_->y());
	g_hf_->end();
}

void condx_view::create_vhf(int x, int y, int w, int h) {
	g_vhf_ = new Fl_Group(x, y, w, h, "VHF");
	int cx = g_vhf_->x() + GAP;
	int cy = g_vhf_->y() + GAP;
	cx += WLLABEL;
	w_vhf_phenomena_.clear();
	for (auto b : data_->vhf_forecasts) {
		Fl_Output* o = new Fl_Output(cx, cy, WBUTTON, ROW_HEIGHT);
		o->box(FL_FLAT_BOX);
		o->align(FL_ALIGN_LEFT);
		o->copy_label(b.first.c_str());
		w_vhf_phenomena_[b.first] = o;
		cy += ROW_HEIGHT;
	}

	g_vhf_->resizable(nullptr);
	g_vhf_->size(GAP + GAP + WLLABEL + WBUTTON, cy - g_vhf_->y());
	g_vhf_->end();
}

void condx_view::create_image(int x, int y, int w, int h) {
	g_image_ = new Fl_Group(x, y, w, h, "Image");
	int cx = g_image_->x() + GAP;
	int cy = g_image_->y() + GAP;
	w_image_ = new Fl_Box(cx, cy, 128, 128);
	w_image_->box(FL_FLAT_BOX);

	cy += w_image_->h() + GAP;
	g_image_->resizable(nullptr);
	g_image_->size(GAP + GAP + w_image_->w(), cy - g_image_->y());
	g_image_->end();
}

void condx_view::cb_ticker(void* v) {
	condx_view* that = (condx_view*)v;
	that->load_data();
	that->enable_widgets();
}

// Callback on switching tab
void condx_view::cb_tabs(Fl_Widget* w, void* v) {
	Fl_Tabs* that = (Fl_Tabs*)w;
	that->label(that->value()->label());
	condx_view* cx = ancestor_view<condx_view>(that);
	cx->enable_widgets();
}
