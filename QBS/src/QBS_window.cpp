#include "QBS_window.h"
#include "QBS_consts.h"
#include "QBS_data.h"
#include "QBS_dormant.h"
#include "QBS_call.h"
#include "QBS_batch.h"
#include "QBS_file.h"

#include "callback.h"
#include "input_hierch.h"

#include <iostream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Wizard.H>

using namespace std;

const extern char* DATE_FORMAT;

// Constructor
QBS_window::QBS_window(int W, int H, const char* L, const char* filename) :
	Fl_Single_Window(W, H, L),
	data_(nullptr),
	qbs_filename_(filename),
	blog_file_(nullptr),
	reading_(false)
{
	spells_.clear();

	Fl_Preferences settings(Fl_Preferences::USER, VENDOR.c_str(), PROGRAM_ID.c_str());

		// Get CSV directory name from settings
	char* temp;
	settings.get("CSV Directory", temp, "");
	csv_directory_ = temp;
	free(temp);
	// If filename has not been supplied in command-line, get it from settings
	if (qbs_filename_.length() == 0) {
		settings.get("Filename", temp, "");
		qbs_filename_ = temp;
		free(temp);
	}
	data_ = new QBS_data;
	// 
	begin();
	create_form();
	end();
	// Now allow the data to be processed
	data_->set_window(this);
	callback(cb_close);
	// If have data then load it
	if (qbs_filename_.length()) {
		read_qbs();
	}
}

QBS_window::~QBS_window() {
	blog_file_->close();
}

// Update filename
void QBS_window::filename(const char* value) {
	qbs_filename_ = value;
}

// Update directry
void QBS_window::directory(const char* value) {
	csv_directory_ = value;
}

// Window close: clear data, and call default closure
void QBS_window::cb_close(Fl_Widget* w, void* v) {
	
	QBS_window* that = ancestor_view<QBS_window>(w);
	that->data_->close_qbs();
	Fl_Preferences settings(Fl_Preferences::USER, VENDOR.c_str(), PROGRAM_ID.c_str());
	settings.set("CSV Directory", that->csv_directory_.c_str());
	settings.set("Filename", that->qbs_filename_.c_str());
	settings.flush();
	Fl_Single_Window::default_callback(that, v);
	delete that->data_;
	return;
}

// Instantiate all the widgets
void QBS_window::create_form() {
	const int WPBUTTON = WBUTTON * 2;

	wiz_ = new Fl_Wizard(x(), y(), w(), h());

	g_file_ = new QBS_file(x(), y(), w(), h());
	spells_[INITIAL] = g_file_;

	g_dormant_ = new QBS_dormant(x(), y(), w(), h());
	spells_[DORMANT] = g_dormant_;

	g_call_ = new QBS_call(x(), y(), w(), h());
	spells_[SORTING] = g_call_;
	spells_[PROCESSING] = g_call_;
	spells_[LOG_CARD] = g_call_;
	spells_[LOG_SASE] = g_call_;
	spells_[CALL_HISTORY] = g_call_;

	g_batch_ = new QBS_batch(x(), y(), w(), h());
	spells_[POSTING] = g_batch_;
	spells_[FINISHING] = g_batch_;
	spells_[RECYCLING] = g_batch_;
	spells_[LOG_BATCH] = g_batch_;
	spells_[BATCH_SUMMARY] = g_batch_;
	spells_[BATCH_REPORT] = g_batch_;

	wiz_->end();

	end();
	show();

	enable_widgets();
	return;
}

// Depending on the current processin phase allow/disallow the command
// buttons;
void QBS_window::update_actions() {
	g_batch_->initialise();
	if (!reading_) {
		process_mode_t mode = data_->mode();
		Fl_Group* spell = spells_.at(mode);
		if (spell) {
			wiz_->value(spell);
		}
		else {
			fl_message("Attempting to show a spell that's not yet implemented");
		}
		enable_widgets();
	}
	
}

// Callback - read QBS data - pass filename to data to start reading it
bool QBS_window::read_qbs() {
	reading_ = true;
	// Initialise stack to DORMANT
	if (data_->read_qbs(qbs_filename_)) {
		reading_ = false;
		g_batch_->populate_batch_choice();
		update_actions();
		return true;
	}
	reading_ = false;
	return false;
}

// Open batch log
void QBS_window::open_batch_log(string batch_name) {
	if (blog_file_ != nullptr) {
		blog_file_->close();
		delete blog_file_;
	}
	string name = csv_directory_ + "/" + batch_name + ".log";
	cout << "Opening " << name << endl;
	blog_file_ = new ofstream(name.c_str(), ios::out | ios::app);
}

// Append batch log - both to putput file and log display
void QBS_window::append_batch_log(const char* text) {
	*blog_file_ << text;
	cout << text;
}


// Populate call choice with extant callsigns
void QBS_window::populate_call_choice(input_hierch* ch) {
	ch->clear();
	string call = data_->get_first_call(RCVD_ALL);
	while (call.length()) {
		ch->add(call.c_str());
		call = data_->get_next_call(RCVD_ALL, call);
	}
}

// Enable widgets
void QBS_window::enable_widgets() {
	g_file_->enable_widgets();
	g_dormant_->enable_widgets();
	g_call_->enable_widgets();
	g_batch_->enable_widgets();
}
