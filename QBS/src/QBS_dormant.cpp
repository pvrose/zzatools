#include "QBS_dormant.h"
#include "QBS_window.h"
#include "QBS_data.h"

#include "utils.h"
#include "drawing.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Int_Input.H>

QBS_dormant::QBS_dormant(int X, int Y, int W, int H, const char* L = nullptr) :
	Fl_Group(X, Y, W, H, L)
{
	win_ = ancestor_view<QBS_window>(this);
	data_ = win_->data_;
	create_form();
}

QBS_dormant::~QBS_dormant() 
{}

void QBS_dormant::create_form() {
	int last_box = data_->get_current();
	char l[128];
	snprintf(l, sizeof(l), "DORMANT: Most recent batch %s", data_->get_batch(last_box).c_str());
	copy_label(l);
	align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	labelsize(FL_NORMAL_SIZE + 2);

	int cx = x() + GAP;
	int cy = y() + GAP;

	bn_receive_sase_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Receive SASEs");
	bn_receive_sase_->callback(cb_receive_sases, nullptr);
	bn_receive_sase_->tooltip("Received ");


}
void enable_widgets();

// Callback - Receive QSL
static void cb_receive_qsl(Fl_Widget* w, void* v);
// Callback - Receive SASEs
static void cb_receive_sases(Fl_Widget* w, void* v);
// Callback - New Batch
static void cb_new_batch(Fl_Widget* w, void* v);
// Callback - Recycle
static void cb_recycle(Fl_Widget* w, void* v);
// Callback - Edit
static void cb_edit(Fl_Widget* w, void* v);
// Callback - Reports
static void cb_reports(Fl_Widget* w, void* v);
// callback - callsign input
static void cb_callsign(Fl_Widget* w, void* v);
// callback - numeric input

