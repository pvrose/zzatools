#include "rig_dgl73.h"
#include "../zzalib/callback.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Preferences.H>

using namespace zza7300;
using namespace zzalib;
using namespace std;

extern Fl_Preferences* settings_;

rig_dlg73::rig_dlg73(int W, int H, const char* label) :
	win_dialog(W, H, label)
	, ip_port_(0)
	, ip_resource_("")
{
	for (int i = 0; i < 4; i++) ip_address_[i] = 0;
	load_values();
	create_form();
}

rig_dlg73::~rig_dlg73() {

};

void rig_dlg73::create_form() {
	const int X0 = EDGE;
	const int Y0 = EDGE;
	const int C1 = X0 + WLABEL;
	const int R1 = Y0 + GAP;
	const int R2 = R1 + HTEXT + GAP;
	const int R3 = R2 + HTEXT + GAP;
	const int W1 = WLABEL + 2 * WBUTTON;
	const int WALL = W1 + 2 * GAP;
	const int HALL = 3 * HTEXT + 4 * GAP;

	// Flrig group
	Fl_Group* flrig_grp = new Fl_Group(X0, Y0, WALL, HALL, "Flrig");
	flrig_grp->labelsize(FONT_SIZE);
	flrig_grp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	flrig_grp->box(FL_DOWN_FRAME);
	// 4 separate inputs for the 4 bytes of the IPv4 address of the flrig server
	Fl_Int_Input* ip_addr_in[4];
	for (int i = 0; i < 4; i++) {
		ip_addr_in[i] = new Fl_Int_Input(C1 + (i * WBUTTON / 2), R1, WBUTTON / 2, HTEXT);
		if (i == 0) {
			ip_addr_in[i]->label("Address");
			ip_addr_in[i]->align(FL_ALIGN_LEFT);
			ip_addr_in[i]->labelsize(FONT_SIZE);
		}
		ip_addr_in[i]->textsize(FONT_SIZE);
		ip_addr_in[i]->tooltip("The IP address of the Flrig server");
		ip_addr_in[i]->value(to_string(ip_address_[i]).c_str());
		ip_addr_in[i]->callback(cb_value_int<Fl_Int_Input>, &ip_address_[i]);
		ip_addr_in[i]->when(FL_WHEN_ENTER_KEY_ALWAYS);
	}
	// IPv4 port number
	Fl_Int_Input* ip_port_in = new Fl_Int_Input(C1, R2, WBUTTON, HTEXT, "Port");
	ip_port_in->align(FL_ALIGN_LEFT);
	ip_port_in->labelsize(FONT_SIZE);
	ip_port_in->textsize(FONT_SIZE);
	ip_port_in->tooltip("The IP port number of the Flrig server");
	ip_port_in->value(to_string(ip_port_).c_str());
	ip_port_in->callback(cb_value_int<Fl_Int_Input>, &ip_port_);
	ip_port_in->when(FL_WHEN_ENTER_KEY_ALWAYS);
	// XML-RPC resource name
	Fl_Input* ip_resource_in = new Fl_Input(C1, R3, WBUTTON, HTEXT, "Resource");
	ip_resource_in->align(FL_ALIGN_LEFT);
	ip_resource_in->labelsize(FONT_SIZE);
	ip_resource_in->textsize(FONT_SIZE);
	ip_resource_in->tooltip("The IP port number of the Flrig server");
	ip_resource_in->value(ip_resource_.c_str());
	ip_resource_in->callback(cb_value<Fl_Input, string>, &ip_resource_);
	ip_resource_in->when(FL_WHEN_ENTER_KEY_ALWAYS);
	flrig_grp->end();

	const int C2 = C1 + WBUTTON + GAP;
	const int R4 = flrig_grp->y() + flrig_grp->h() + GAP;
	
	// And the OK Button
	Fl_Button* bn_ok = new Fl_Button(C1, R4, WBUTTON, HBUTTON, "OK");
	bn_ok->labelfont(FONT);
	bn_ok->labelsize(FONT_SIZE);
	bn_ok->callback(cb_bn_ok);
	bn_ok->when(FL_WHEN_RELEASE);
	// And the Cancel Button
	Fl_Button* bn_cancel = new Fl_Button(C1 + bn_ok->w() + GAP, R4, WBUTTON, HBUTTON, "Cancel");
	bn_cancel->labelfont(FONT);
	bn_cancel->labelsize(FONT_SIZE);
	bn_cancel->callback(cb_bn_ok);
	bn_cancel->when(FL_WHEN_RELEASE);

	// Close button 
	callback(cb_bn_cancel);

	// Resize  the window to fit all the widgets
	resizable(nullptr);
	int width = WLABEL + WBUTTON * 2 + GAP * 3 + 2 * EDGE;
	int height = R4 + HBUTTON + EDGE;
	size(width, height);

	end();
	

}

// Get the settings to preload the widget values
void rig_dlg73::load_values() {
	Fl_Preferences rig_settings(settings_, "Rig");
	// Flrig settings
	Fl_Preferences flrig_settings(rig_settings, "FLRig");
	unsigned long ip_address = 0;
	flrig_settings.get("IP Address", (int&)ip_address, 0x7F000001);
	for (int i = 3; i >= 0; i--) {
		ip_address_[i] = ip_address & 0x000000FF;
		ip_address >>= 8;
	}
	char* temp;
	flrig_settings.get("Port Number", ip_port_, 12345);
	flrig_settings.get("Resource", temp, "/RPC2");
	ip_resource_ = temp;
	free(temp);
}

// Store data
void rig_dlg73::save_values() {
	// 
	Fl_Preferences rig_settings(settings_, "Rig");
	// Flrig settings
	Fl_Preferences flrig_settings(rig_settings, "FLRig");

	unsigned long ip_address = 0;
	for (int i = 3; i >= 0; i--) {
		ip_address <<= 8;
		ip_address |= ip_address_[i] & 0xFF;
	}
	flrig_settings.set("IP Address", (int)ip_address);
	flrig_settings.set("Port Number", ip_port_);
	flrig_settings.set("Resource", ip_resource_.c_str());

}

void rig_dlg73::cb_bn_ok(Fl_Widget* w, void* v) {
	rig_dlg73* that = ancestor_view<rig_dlg73>(w);
	that->save_values();
	that->do_button(BN_OK);
}

void rig_dlg73::cb_bn_cancel(Fl_Widget* w, void* v) {
	rig_dlg73* that = ancestor_view<rig_dlg73>(w);
	that->do_button(BN_CANCEL);
}