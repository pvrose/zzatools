#include "qso_net_entry.h"
#include "qso_entry.h"
#include "status.h"

#include "book.h"
#include "record.h"

#include <FL/Fl_Tabs.H>

extern status* status_;
extern book* book_;
extern void open_html(const char*);

// Constructor
qso_net_entry::qso_net_entry(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	create_form(X, Y);
}

// Destructor
qso_net_entry::~qso_net_entry() {
}

// Create form
void qso_net_entry::create_form(int X, int Y) {

	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	labelfont(FL_BOLD);

	// Tabbed std::set of qso_entry forms
	entries_ = new Fl_Tabs(X, Y + HTEXT, w(), h());
	entries_->callback(cb_entries);
	entries_->handle_overflow(Fl_Tabs::OVERFLOW_PULLDOWN);
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	entries_->client_area(rx, ry, rw, rh, 0);
	int dw = w() - rw;
	int dh = h() - rh;
	// Initially with an "empty" copy of qso_entry
	qso_entry* wx = new qso_entry(rx, ry, rw, rh, "");
	wx->labelsize(FL_NORMAL_SIZE + 2);
	entries_->add(wx);
	entries_->resizable(nullptr);
	entries_->size(wx->w() + dw, wx->h() + dh);
	entries_->end();

	resizable(nullptr);
	size(entries_->w(), HTEXT + entries_->h());
	end();
}

// Returns the currently displayed qso_entry
qso_entry* qso_net_entry::entry() {
	return (qso_entry*)entries_->value();
}

// Set the entry
void qso_net_entry::entry(qso_entry* w) {
	entries_->value(w);
}

// Returns the number of qso_entry forms
int qso_net_entry::entries() {
	return entries_->children();
}

// Returns the last entry
qso_entry* qso_net_entry::last_entry() {
	int ix = entries_->children() - 1;
	Fl_Widget* w = entries_->child(ix);
	return (qso_entry*)w;
}

// Returns the first enytty
qso_entry* qso_net_entry::first_entry() {
	return (qso_entry*)entries_->child(0);
}

// Enable/disable widgets - pass on to children
void qso_net_entry::enable_widgets() {
	for (int cx = 0; cx < entries_->children(); cx++) {
		Fl_Widget* wx = entries_->child(cx);
		if (wx == entries_->value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		qso_entry* qe = (qso_entry*)wx;
		if (qe->qso()) qe->copy_label(qe->qso()->item("CALL").c_str());
		qe->enable_widgets();
	}
}

// Return QSO - return open tab's QSO
record* qso_net_entry::qso() {
	return entry()->qso();
}

// Return record number - return open tab;s QSO number
qso_num_t qso_net_entry::qso_number() {
	return entry()->qso_number();
}

// Add QSO - add a tab
void qso_net_entry::add_entry() {
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	entries_->client_area(rx, ry, rw, rh, 0);
	// Initially with an "empty" copy of qso_entry
	entries_->begin();
	qso_entry* wx = new qso_entry(rx, ry, rw, rh, "");
	wx->labelsize(FL_NORMAL_SIZE + 2);
	entries_->add(wx);
	entries_->end();
	entry(wx);
	
}

// Save and remove selected qso
void qso_net_entry::remove_entry() {
	Fl_Widget* w = (qso_entry*)entries_->value();
	int cx = 0;
	for (; cx < entries_->children() && w != nullptr; cx++) {
		if (w == entries_->child(cx)) {
			((qso_entry*)w)->delete_qso();
			entries_->delete_child(cx);
			w = nullptr;
		}
	}
	if (cx < entries_->children()) {
		entries_->value(entries_->child(cx));
	}
	else if (cx > 0 && entries_->children() > 0) {
		entries_->value(entries_->child(entries_->children() - 1));
	}
}

// Set the QSO into the selected tab
void qso_net_entry::set_qso(qso_num_t qso_number) {
	qso_entry* qe = (qso_entry*)entries_->value();
	if (qe != nullptr) {
		if (qe->qso() != nullptr) {
			char msg[128];
			snprintf(msg, sizeof(msg), "DASH: Trying to use a net QSO tab that is used for %s", qe->qso()->item("CALL").c_str());
			status_->misc_status(ST_SEVERE, msg);
		}
		else {
			qe->qso(qso_number);
			record* qso = qe->qso();
			qe->copy_label(qso->item("CALL").c_str());
			entries_->value(qe);
		}
	}
}

// Return original QSO
record* qso_net_entry::original_qso() {
	qso_entry* qe = (qso_entry*)entries_->value();
	return qe->original_qso();
}


// Copy fields from record
void qso_net_entry::copy_qso_to_display(int flags) {
	qso_entry* qe = (qso_entry*)entries_->value();
	qe->copy_qso_to_display(flags);
}

// QSO number in net
bool qso_net_entry::qso_in_net(qso_num_t qso_number) {
	bool result = false;
	for (int cx = 0; cx < entries_->children() && !result; cx++) {
		qso_entry* qe = (qso_entry*)entries_->child(cx);
		if (qe->qso_number() == qso_number) { result = true; }
	}
	return result;
}

// Select QSO with the provided number - if it exists
void qso_net_entry::select_qso(qso_num_t qso_number) {
	bool found = false;
	qso_entry* qe;
	for (int cx = 0; cx < entries_->children() && !found; cx++) {
		qe = (qso_entry*)entries_->child(cx);
		if (qe->qso_number() == qso_number) {
			found = true;
		}
	}
	if (found) entries_->value(qe);
}

// Append the selected QSO to the book
void qso_net_entry::append_qso() {
	qso_entry* qe = (qso_entry*)entries_->value();
	qe->append_qso();
}

// Callback on selecting a tab
void qso_net_entry::cb_entries(Fl_Widget* w, void* v) {
	Fl_Tabs* tabs = (Fl_Tabs*)w;
	qso_net_entry* that = ancestor_view<qso_net_entry>(w);
	that->enable_widgets();
	qso_entry* qe = (qso_entry*)tabs->value();
	book_->selection(qe->qso_number());
}

// Navigate to target
void qso_net_entry::navigate(navigate_t target) {
	// Current tab
	int ix = 0;
	bool found = false;
	Fl_Widget* curr = entries_->value();
	for (int i = 0; i < entries() && !found; i ++) {
		if (curr == entries_->child(i)) {
			found = true;
			ix = i;
		}
	}
	switch(target) {
		case NV_FIRST: {
			entries_->value(first_entry());
			break;
		}
		case NV_PREV: {
			if ( ix > 0) {
				ix--;
				entries_->value(entries_->child(ix));
			}
			break;
		}
		case NV_NEXT: {
			if (ix < entries_->children() - 1) {
				ix++;
				entries_->value(entries_->child(ix));
			}
			break;
		}
		case NV_LAST: {
			entries_->value(last_entry());

		}
	}
	enable_widgets();
	qso_entry* qe = (qso_entry*)entries_->value();
	book_->selection(qe->qso_number());
}

// Return whetehr can navigate
bool qso_net_entry::can_navigate(navigate_t target) {
	int ix = 0;
	int found = false;
	Fl_Widget* curr = entries_->value();
	for (int i = 0; i < entries() && !found; i ++) {
		if (curr == entries_->child(i)) {
			found = true;
			ix = i;
		}
	}
	if (!found) return false;
	switch(target) {
		case NV_FIRST:
		case NV_PREV: {
			if (ix == 0) {
				return false;
			} else {
				return true;
			}
		}
		case NV_LAST:
		case NV_NEXT: {
			if (ix >= entries_->children() - 1) {
				return false;
			} else {
				return true;
			}
		}
		default: return false;
	}
}

void qso_net_entry::set_focus_call() {
	qso_entry* qe = (qso_entry*)entries_->value();
	qe->set_focus_call();
}