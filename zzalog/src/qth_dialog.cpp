#include "qth_dialog.h"
#include "callback.h"
#include "utils.h"

#include "record.h"
#include "pfx_data.h"
#include "prefix.h"
#include "status.h"
#include "tabbed_forms.h"
#include "intl_widgets.h"
#include "spec_data.h"

#include <set>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/fl_ask.H>




extern Fl_Preferences* settings_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern spec_data* spec_data_;

// QTH group constructor
qth_dialog::qth_dialog(string qth_name) :
	win_dialog(10, 10)
{
	qth_name_ = qth_name;
	changed_ = false;
	qth_details_ = nullptr;

	// Create window
	char label[100];
	snprintf(label, 100, "QTH details - %s", qth_name_.c_str());
	copy_label(label);

	// Read settings
	load_values();
	// Create the form
	create_form(0, 0);
	// Enable widgets
	enable_widgets();
}

// Destructor
qth_dialog::~qth_dialog() {
	delete qth_details_;
}

// Get initial data from settings - additional ones for the QTH group
void qth_dialog::load_values() {
	// Get the QTH details
	qth_details_ = new record(*spec_data_->expand_macro("APP_ZZA_QTH", qth_name_));
	current_qth_.name = qth_details_->item("MY_NAME");
	current_qth_.street = qth_details_->item("MY_STREET");
	current_qth_.city = qth_details_->item("MY_CITY");
	current_qth_.postcode = qth_details_->item("MY_POSTAL_CODE");
	current_qth_.locator = qth_details_->item("MY_GRIDSQUARE");
	current_qth_.dxcc_name = qth_details_->item("MY_COUNTRY");
	current_qth_.dxcc_id = qth_details_->item("MY_DXCC");
	current_qth_.state = qth_details_->item("MY_STATE");
	current_qth_.county = qth_details_->item("MY_CNTY");
	current_qth_.cq_zone = qth_details_->item("MY_CQ_ZONE");
	current_qth_.itu_zone = qth_details_->item("MY_ITU_ZONE");
	// MY_CONT is not a valid ADIF field
	//current_qth_.continent = qth_details_->item("MY_CONT");
	current_qth_.iota = qth_details_->item("MY_IOTA");
	current_qth_.description = qth_details_->item("APP_ZZA_QTH_DESCR");
	original_qth_ = current_qth_;
}

// create the form - additional widgets for QTH settings
void qth_dialog::create_form(int X, int Y) {

	// Explicitly call begin to ensure that we haven't had too many ends.
	begin();

	const int WIP = WBUTTON * 2;

	// Column 1: Opeartor name and address
	int curr_x = X + GAP;
	int curr_y = Y + HTEXT;
	int max_w = curr_x - X;
	int max_h = curr_y - Y;
	// Operator name
	ip_name_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "Op. Name");
	ip_name_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_name_->when(FL_WHEN_RELEASE);
	ip_name_->value(current_qth_.name.c_str());
	ip_name_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.name);
	ip_name_->tooltip("Enter operator name");
	// Address line 1 (
	curr_y += HBUTTON + HTEXT;
	ip_address1_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "Street");
	ip_address1_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_address1_->when(FL_WHEN_RELEASE);
	ip_address1_->value(current_qth_.street.c_str());
	ip_address1_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.street);
	ip_address1_->tooltip("Enter Street address - forms MY_STREET");
	// Address line 3
	curr_y += HBUTTON + HTEXT;
	ip_address3_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "Town/City");
	ip_address3_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_address3_->when(FL_WHEN_RELEASE);
	ip_address3_->value(current_qth_.city.c_str());
	ip_address3_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.city);
	ip_address3_->tooltip("Enter town/city - forms MY_CITY");
	// Address line 4
	curr_y += HBUTTON + HTEXT;
	ip_address4_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "Postal code");
	ip_address4_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_address4_->when(FL_WHEN_RELEASE);
	ip_address4_->value(current_qth_.postcode.c_str());
	ip_address4_->callback(cb_ip_upper, (void*)&current_qth_.postcode);
	ip_address4_->tooltip("Enter Postcode - forms MY_POSTCODE");

	max_h += 5 * (HBUTTON + HTEXT);
	max_w += WIP + GAP;
	curr_y = Y + HTEXT;
	curr_x += WIP + GAP;

	// Locator
	ip_locator_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "Locator");
	ip_locator_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_locator_->when(FL_WHEN_RELEASE);
	ip_locator_->value(current_qth_.locator.c_str());
	ip_locator_->callback(cb_ip_upper, (void*)&current_qth_.locator);
	ip_locator_->tooltip("Enter grid locator");
	// DXCC name (
	curr_y += HBUTTON + HTEXT;
	ip_dxcc_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "DXCC Entity");
	ip_dxcc_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_dxcc_->when(FL_WHEN_RELEASE);
	ip_dxcc_->value(current_qth_.dxcc_name.c_str());
	ip_dxcc_->callback(cb_ip_cty, (void*)&current_qth_.dxcc_name);
	ip_dxcc_->tooltip("Enter DXCC Entity - used to generate MY_COUNTRY");
	// DXCC Reference id. number
	curr_y += HBUTTON + HTEXT;
	ip_dxcc_adif_ = new Fl_Int_Input(curr_x, curr_y, WIP, HBUTTON, "DXCC Id.");
	ip_dxcc_adif_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_dxcc_adif_->when(FL_WHEN_RELEASE);
	ip_dxcc_adif_->value(current_qth_.dxcc_id.c_str());
	ip_dxcc_adif_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.dxcc_id);
	ip_dxcc_adif_->tooltip("Enter DXCC Id - forms MY_DXCC");
	// Primry Admin. Subdivision (E.g. US State)
	curr_y += HBUTTON + HTEXT;
	ip_admin1_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "1st Level");
	ip_admin1_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_admin1_->when(FL_WHEN_RELEASE);
	ip_admin1_->value(current_qth_.state.c_str());
	ip_admin1_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.state);
	ip_admin1_->tooltip("Enter first level sub-division - eg US State - forms MY_STATE");
	// Secondary Admin. subdivision (e.g. US County)
	curr_y += HBUTTON + HTEXT;
	ip_admin2_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "2nd level");
	ip_admin2_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_admin2_->when(FL_WHEN_RELEASE);
	ip_admin2_->value(current_qth_.county.c_str());
	ip_admin2_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.county);
	ip_admin2_->tooltip("Enter second level sub-division - eg US County - forms MY_COUNTY");

	max_h = max(max_h, Y + HTEXT + 5 * (HBUTTON + HTEXT));
	max_w += WIP + GAP;
	curr_y = Y + HTEXT;
	curr_x += WIP + GAP;

	// CQ Zone
	ip_cq_zone_ = new Fl_Int_Input(curr_x, curr_y, WIP, HBUTTON, "CQ Zone");
	ip_cq_zone_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_cq_zone_->when(FL_WHEN_RELEASE);
	ip_cq_zone_->value(current_qth_.cq_zone.c_str());
	ip_cq_zone_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.cq_zone);
	ip_cq_zone_->tooltip("Enter CQ Zone - forms MY_CQ_ZONE");
	// ITU Zone
	curr_y += HBUTTON + HTEXT;
	ip_itu_zone_ = new Fl_Int_Input(curr_x, curr_y, WIP, HBUTTON, "ITU Zone");
	ip_itu_zone_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_itu_zone_->when(FL_WHEN_RELEASE);
	ip_itu_zone_->value(current_qth_.itu_zone.c_str());
	ip_itu_zone_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.itu_zone);
	ip_itu_zone_->tooltip("Enter ITU Zone - forms MY_ITU_ZONE");
	// Continent
	curr_y += HBUTTON + HTEXT;
	ip_cont_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "Continent");
	ip_cont_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_cont_->when(FL_WHEN_RELEASE);
	ip_cont_->value(current_qth_.continent.c_str());
	ip_cont_->callback(cb_ip_upper, (void*)&current_qth_.continent);
	ip_cont_->tooltip("Enter continent (2-char) - forms MY_CONT");
	// Address line 4
	curr_y += HBUTTON + HTEXT;
	ip_iota_ = new Fl_Input(curr_x, curr_y, WIP, HBUTTON, "IOTA");
	ip_iota_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_iota_->when(FL_WHEN_RELEASE);
	ip_iota_->value(current_qth_.iota.c_str());
	ip_iota_->callback(cb_ip_upper, (void*)&current_qth_.iota);
	ip_iota_->tooltip("Enter IOTA Reference - forms MY_IOTA");

	max_h = max(max_h, Y + HTEXT + 4 * (HBUTTON + HTEXT));
	max_w += WIP + GAP;
	curr_y = Y + max_h;
	curr_x = X + GAP;

	ip_description_ = new Fl_Input(curr_x, curr_y, WIP * 2 + GAP, HBUTTON, "Description");
	ip_description_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_description_->when(FL_WHEN_RELEASE);
	ip_description_->value(current_qth_.description.c_str());
	ip_description_->callback(cb_value<Fl_Input, string>, (void*)&current_qth_.description);
	ip_description_->tooltip("Enter Description");

	curr_y = ip_description_->y() + ip_description_->h() + GAP;
	max_h = curr_y;

	curr_y += HTEXT;
	curr_x = X + GAP;

	ip_latitude_ = new Fl_Float_Input(curr_x, curr_y, WIP, HBUTTON, "Latitude");
	ip_latitude_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_latitude_->tooltip("Enter the latitude to convert (+ve = N, -ve = S)");
	curr_x += ip_latitude_->w() + GAP;
	ip_longitude_ = new Fl_Float_Input(curr_x, curr_y, WIP, HBUTTON, "Latitude");
	ip_longitude_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_longitude_->tooltip("Enter the longiitude to convert (+ve = E, -ve = W)");
	curr_x += ip_longitude_->w() + GAP;
	Fl_Button* bn_convert = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Gridsquare");
	bn_convert->callback(cb_bn_convert, (void*)&current_qth_.locator);
	bn_convert->tooltip("Update gridsquare based on latitude and longitude");

	curr_y += ip_latitude_->h() + GAP;
	max_w = max(max_w, bn_convert->x() + bn_convert->w() + GAP);
	curr_x = X + GAP;
	max_h = curr_y;

	// OK Button
	Fl_Button* bn_ok = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "OK");
	bn_ok->callback(cb_bn_ok);
	bn_ok->when(FL_WHEN_RELEASE);
	bn_ok->tooltip("Accept changes");
	// Cancel button
	curr_x += WBUTTON + GAP;
	Fl_Button* bn_cancel = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Cancel");
	bn_cancel->callback(cb_bn_cancel);
	bn_cancel->when(FL_WHEN_RELEASE);
	bn_cancel->tooltip("Cancel changes");

	max_w = max(max_w, bn_cancel->x() + bn_cancel->w() + GAP);
	max_h += HBUTTON + GAP;

	// resize the group accordingly
	resizable(nullptr);
	size(max_w, max_h);
	show();
	end();
}

// Save values in settings
void qth_dialog::save_values() {
	// Note QTH structure only has == operator
	if (!(current_qth_ == original_qth_)) {
		changed_ = true;
		qth_details_->item("MY_NAME", current_qth_.name);
		qth_details_->item("MY_STREET", current_qth_.street);
		qth_details_->item("MY_CITY", current_qth_.city);
		qth_details_->item("MY_POSTAL_CODE", current_qth_.postcode);
		qth_details_->item("MY_GRIDSQUARE", current_qth_.locator);
		qth_details_->item("MY_COUNTRY", current_qth_.dxcc_name);
		qth_details_->item("MY_DXCC", current_qth_.dxcc_id);
		qth_details_->item("MY_STATE", current_qth_.state);
		qth_details_->item("MY_CNTY", current_qth_.county);
		qth_details_->item("MY_CQ_ZONE", current_qth_.cq_zone);
		qth_details_->item("MY_ITU_ZONE", current_qth_.itu_zone);
		// MY_CONT is not a valid ADIF field
		//qth_details_->item("MY_CONT", current_qth_.continent);
		qth_details_->item("MY_IOTA", current_qth_.iota);
		qth_details_->item("APP_ZZA_QTH_DESCR", current_qth_.description);
		spec_data_->add_user_macro("APP_ZZA_QTH", qth_name_, { qth_details_, current_qth_.description });
	}
}

// Add values
void qth_dialog::enable_widgets() {
	ip_name_->value(current_qth_.name.c_str());
	ip_address1_->value(current_qth_.street.c_str());
	ip_address3_->value(current_qth_.city.c_str());
	ip_address4_->value(current_qth_.postcode.c_str());
	ip_locator_->value(current_qth_.locator.c_str());
	if (current_qth_.dxcc_id.length() && current_qth_.dxcc_name.length()) {
		prefix* pfx = pfx_data_->get_prefix(stoi(current_qth_.dxcc_id));
		ip_dxcc_->value((pfx->nickname_ + ":" + current_qth_.dxcc_name).c_str());
	}
	else {
		ip_dxcc_->value(current_qth_.dxcc_name.c_str());
	}
	ip_dxcc_adif_->value(current_qth_.dxcc_id.c_str());
	ip_admin1_->value(current_qth_.state.c_str());
	ip_admin2_->value(current_qth_.county.c_str());
	ip_cq_zone_->value(current_qth_.cq_zone.c_str());
	ip_itu_zone_->value(current_qth_.itu_zone.c_str());
	ip_cont_->value(current_qth_.continent.c_str());
	ip_iota_->value(current_qth_.iota.c_str());
	ip_description_->value(current_qth_.description.c_str());
	// TODO - there isn't a MY_CONT in ADIF
	ip_cont_->deactivate();
}

// Callback that converts what is typed to upper-case
// v is pointer to the field in the QTH structure
void qth_dialog::cb_ip_upper(Fl_Widget* w, void* v) {
	cb_value<intl_input, string>(w, v);
	*(string*)v = to_upper(*(string*)v);
	((intl_input*)w)->value(((string*)v)->c_str());
}

// DXCC callback 
// v is pointer to the callsign field of the QTH structure
void qth_dialog::cb_ip_cty(Fl_Widget* w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	qth_info_t* qth = &that->current_qth_;
	cb_value<Fl_Input, string>(w, v);
	string* value = (string*)v;
	// Find prefix (nickname) - either GM or GM:SCOTLAND
	size_t pos = ::find(value->c_str(), value->length(), ':');
	string nickname = value->substr(0, pos);
	// Get the prefix and track up to the DXCC prefix
	prefix* prefix = pfx_data_->get_prefix(nickname);
	if (prefix) {
		// Get CQZone - if > 1 supplied then message for now
		if (prefix->cq_zones_.size() > 1) {
			string message = "STATION: Multiple CQ Zones: ";
			for (unsigned int i = 0; i < prefix->cq_zones_.size(); i++) {
				string zone = to_string(prefix->cq_zones_[i]);
				message += zone;
			}
			status_->misc_status(ST_WARNING, message.c_str());
			qth->cq_zone = "";
		}
		else {
			// Set it in the QTH
			qth->cq_zone = to_string(prefix->cq_zones_[0]);
		}
		// Get ITUZone - if > 1 supplied then message for now
		if (prefix->itu_zones_.size() > 1) {
			string message = "STATION: Multiple ITU Zones: ";
			for (unsigned int i = 0; i < prefix->itu_zones_.size(); i++) {
				string zone = to_string(prefix->itu_zones_[i]);
				message += zone;
			}
			status_->misc_status(ST_WARNING, message.c_str());
			qth->itu_zone = "";
		}
		else {
			// Set it in the QTH
			qth->itu_zone = to_string(prefix->itu_zones_[0]);
		}
		// Get Continent - if > 1 supplied then message for now
		if (prefix->continents_.size() > 1) {
			string message = "STATION: Multiple Continents: ";
			for (unsigned int i = 0; i < prefix->continents_.size(); i++) {
				message += prefix->continents_[i] + "; ";
			}
			status_->misc_status(ST_WARNING, message.c_str());
			qth->continent = "";
		}
		else {
			// Set it in the QTH
			qth->continent = prefix->continents_[0];
		}
		qth->state = prefix->state_;
		// Track up to DXCC prefix record
		while (prefix->parent_ != nullptr) {
			prefix = prefix->parent_;
		}
		// Fill in DXCC fields
		qth->dxcc_id = to_string(prefix->dxcc_code_);
		qth->dxcc_name = prefix->name_;
	}
	else {
		// Not a valid nickname - set fields to blurgh
		qth->cq_zone = "";
		qth->itu_zone = "";
		qth->continent = "";
		qth->state = "";
		qth->dxcc_id = "";
		qth->dxcc_name = "";
	}
	// Update other widgets
	that->enable_widgets();
	that->redraw();
}

void qth_dialog::cb_bn_ok(Fl_Widget * w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	that->save_values();
	// If changed report OK else CANCEL
	if (that->changed_) that->do_button(BN_OK);
	else that->do_button(BN_CANCEL);
}

void qth_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	that->do_button(BN_CANCEL);
}

// Convert lat and long fields to gridsquare
void qth_dialog::cb_bn_convert(Fl_Widget* w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	string* grid = (string*)v;
	const char* latitude = that->ip_latitude_->value();
	const char* longitude = that->ip_longitude_->value();
	// Convert to locatio pair
	lat_long_t location; 
	try {
		location = { atof(latitude), atof(longitude) };
	}
	catch (exception e) {
		location = { nan(""), nan("") };
	}
	// Get the number of decomal points
	int lat_decimals = strlen(latitude) - ::find(latitude, strlen(latitude), '.');
	int long_decimals = strlen(longitude) - ::find(longitude, strlen(longitude), '.');
	int num_decimals = min(lat_decimals, long_decimals);
	int num_chars;
	switch (num_decimals) {
	case 0:
	case 1:
		num_chars = 4;
		break;
	case 2:
	case 3:
		num_chars = 6;
		break;
	case 4:
		num_chars = 8;
		break;
	case 5:
		num_chars = 10;
		break;
	default:
		num_chars = 12;
		break;
	}
	*grid = latlong_to_grid(location, num_chars);
	that->enable_widgets();
}
