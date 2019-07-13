#include "qsl_form.h"
#include "record.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

using namespace zzalog;

bool qsl_form::data_initialised_ = false;
int qsl_form::width_ = 0;
int qsl_form::height_ = 0;
vector<qsl_form::qsl_widget> qsl_form::widget_data_;

// Constructor
qsl_form::qsl_form(int X, int Y, record* record) :
	Fl_Group(X, Y, 0, 0) {
	// Only initialise data on first ever call
	if (!data_initialised_) {
		load_data();
		data_initialised_ = true;
	}
	// Get record
	record_ = record;
	// Draw the widgets
	create_form();
}

// Destructor
qsl_form::~qsl_form() {

}

// Load data
void qsl_form::load_data() {
	// TODO: Replace this with data read from settings once the qsl_designer has been coded
	// Card size - 99.1 x 67.7 mm
	width_ = (int)(99.1f * MM_TO_POINT);
	height_ = (int)(66.7f * MM_TO_POINT);

	//struct qsl_widget {
	//	string text;
	//	int x;
	//	int y;
	//	Fl_Color colour;
	//	int font_size;
	//	Fl_Font font;
	//	Fl_Align align;
	//	Fl_Boxtype type;
	//};
	const int box_width = (width_ - 20) / 5;
	const int text_width = (width_ - 20) / 2;
	widget_data_ = {
		{ "To <CALL>", width_ - 10 - text_width, 10, text_width, 15,  FL_RED, 14, FL_HELVETICA, FL_ALIGN_RIGHT | FL_ALIGN_INSIDE, FL_NO_BOX },
		//{ "Via <QSL_VIA>", width_ - 10 - text_width, 26, text_width, 15, FL_RED, 14, FL_HELVETICA, FL_ALIGN_RIGHT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "<MY_NAME> <STATION_CALLSIGN>", 10, 10, text_width, 9, FL_BLACK, 8, FL_HELVETICA, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "<MY_CITY>", 10, 19, text_width, 9, FL_BLACK, 8, FL_HELVETICA, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "<MY_COUNTRY>", 10, 28, text_width, 9, FL_BLACK, 8, FL_HELVETICA, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "CQ: <MY_CQ_ZONE> ITU:<MY_ITU_ZONE> IOTA:<MY_IOTA>", 10, 37, text_width, 9, FL_BLACK, 8, FL_HELVETICA, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "Grid: <MY_GRIDSQUARE>", 10, 46, text_width, 9, FL_BLACK, 8, FL_HELVETICA, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "Date", 10, 60, box_width, 20, FL_BLACK, 10, FL_HELVETICA, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "Time", 10 + box_width, 60, box_width, 20, FL_BLACK, 10, FL_HELVETICA, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "Band", 10 + (2 * box_width), 60, box_width, 20, FL_BLACK, 10, FL_HELVETICA, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "Mode", 10 + (3 * box_width), 60, box_width, 20, FL_BLACK, 10, FL_HELVETICA, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "Report", 10 + (4 * box_width), 60, box_width, 20, FL_BLACK, 10, FL_HELVETICA, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "<QSO_DATE>", 10, 80, box_width, 20, FL_BLUE, 10, FL_HELVETICA_ITALIC, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "<TIME_ON>", 10 + box_width, 80, box_width, 20, FL_BLUE, 10, FL_HELVETICA_ITALIC, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "<BAND>", 10 + (2 * box_width), 80, box_width, 20, FL_BLUE, 10, FL_HELVETICA_ITALIC, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "<MODE>", 10 + (3 * box_width), 80, box_width, 20, FL_BLUE, 10, FL_HELVETICA_ITALIC, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "<RST_SENT>", 10 + (4 * box_width), 80, box_width, 20, FL_BLUE, 10, FL_HELVETICA_ITALIC, FL_ALIGN_CENTER, FL_BORDER_FRAME },
		{ "Tnx Report, <NAME>, 73", 10, 110, text_width, 9, FL_BLACK, 10, FL_HELVETICA_ITALIC, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "Rig: <MY_RIG>", 10, 125, text_width, 9, FL_BLACK, 8, FL_HELVETICA, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX },
		{ "Ant: <MY_ANTENNA>", 10, 134, (width_ / 2) - 10, 9, FL_BLACK, 8, FL_HELVETICA, FL_ALIGN_LEFT | FL_ALIGN_INSIDE, FL_NO_BOX }
	};
}

// Draw form
void qsl_form::create_form() {
	size(width_, height_);
	Fl_Group* grp = new Fl_Group(x(), y(), width_, height_);
	grp->box(FL_BORDER_FRAME);
	for (auto it = widget_data_.begin(); it != widget_data_.end(); it++) {
		Fl_Box* box = new Fl_Box(x() + it->x, y() + it->y, it->w, it->h);
		box->color(FL_BLACK);
		box->copy_label(record_->item_merge(it->text).c_str());
		box->labelcolor(it->colour);
		box->labelfont(it->font);
		box->labelsize(it->font_size);
		box->align(it->align);
		box->box(it->type);
		add(box);
	}
	grp->end();
	end();
}

