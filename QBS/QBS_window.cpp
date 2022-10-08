#include "QBS_window.h"
#include "QBS_consts.h"
#include "QBS_data.h"
#include "../zzalib/callback.h"

#include <iostream>

using namespace zzalib;
using namespace std;

extern const char* DATE_FORMAT;

// Constructor
QBS_window::QBS_window(int W, int H, const char* L, const char* filename) :
	Fl_Single_Window(W, H, L),
	settings_(Fl_Preferences::USER, VENDOR.c_str(), PROGRAM_ID.c_str()),
	data_(nullptr),
	qbs_filename_(filename)
{
	data_ = new QBS_data;
	// Get CSV directory name from settings
	char* temp;
	settings_.get("CSV Directory", temp, "");
	csv_directory_ = temp;
	free(temp);
	// If filename has not been supplied in command-line, get it from settings
	if (qbs_filename_.length() == 0) {
		settings_.get("Filename", temp, "");
		qbs_filename_ = temp;
		free(temp);
	}
	// 
	begin();
	create_form();
	end();
	// Now allow the data to be processed
	data_->set_window(this);
}

QBS_window::~QBS_window() {
}

// Window close: clear data, and call default closure
void QBS_window::cb_close(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	delete that->data_;
	Fl_Single_Window::default_callback((Fl_Window*)w, v);
	return;
}

// Instantiate all the widgets
void QBS_window::create_form() {
	int curr_x = GAP;
	int curr_y = GAP;
	int max_x = curr_x;

	g_input_ = new Fl_Group(curr_x, curr_y, 1000, 1000, "File input");
	g_input_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	g_input_->labelfont(FONT | FL_ITALIC | FL_BOLD);
	g_input_->labelsize(FONT_SIZE);

	// Import CSV
	// This button when pressed will instigate the import CSV process
	// It will display in the light the status of file imports
	// BLACK  - not loaded
	// YELLOW - loading
	// RED    - all files loaded
	curr_x += GAP;
	curr_y += HTEXT;
	bn_import_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Import CSV");
	bn_import_->labelfont(FONT);
	bn_import_->labelsize(FONT_SIZE);
	bn_import_->value(0);
	bn_import_->callback(cb_import, nullptr);

	// CVS directory name
	// This input cab ne used to input the V directory name - use of the
	// associated browse button will fill it in
	curr_x += bn_import_->w() + GAP;
	ip_csv_dir_ = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON);
	ip_csv_dir_->textfont(FONT);
	ip_csv_dir_->textsize(FONT_SIZE);
	ip_csv_dir_->value(csv_directory_.c_str());

	// CVS browse directory
	// This will open the native file browser to get the directory
	curr_x += ip_csv_dir_->w();
	bn_brf_csv_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Browse");
	bn_brf_csv_->labelfont(FONT);
	bn_brf_csv_->labelsize(FONT_SIZE);
	csv_browser_ = browser_data_t(
		"Please select directory containing CSV files",
		"",
		&csv_directory_,
		nullptr,
		ip_csv_dir_,
		nullptr
	);
	bn_brf_csv_->callback(cb_bn_browsedir, &csv_browser_);

	curr_x += bn_brf_csv_->w() + GAP;
	max_x = max(max_x, curr_x);

	// Read QBS file
	// This button when pressed will instigate a native (.qbs) file read
	// It will display in the light the status of file read
	// BACKGROUND - not loaded
	// RED - file loaded
	curr_x = g_input_->x() + GAP;
	curr_y = bn_import_->y() + bn_import_->h();
	bn_read_qbs_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Read file");
	bn_read_qbs_->labelfont(FONT);
	bn_read_qbs_->labelsize(FONT_SIZE);
	bn_read_qbs_->value(0);
	bn_read_qbs_->callback(cb_read_qbs, nullptr);

	// QBS filename name
	// This input cab ne used to input the QBS filename - use of the
	// associated browse button will fill it in
	curr_x += bn_read_qbs_->w() + GAP;
	ip_qbs_file_ = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON);
	ip_qbs_file_->textfont(FONT);
	ip_qbs_file_->textsize(FONT_SIZE);
	ip_qbs_file_->value(qbs_filename_.c_str());

	// CVS browse directory
	// This will open the native file browser to get the directory
	curr_x += ip_qbs_file_->w();
	bn_brf_qbs_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Browse");
	bn_brf_qbs_->labelfont(FONT);
	bn_brf_qbs_->labelsize(FONT_SIZE);
	qbs_browser_ = browser_data_t(
		"Please select QBS input file",
		"QBS files\t*.{qbs,txt}",
		&qbs_filename_,
		nullptr,
		ip_qbs_file_,
		nullptr
	);
	bn_brf_qbs_->callback(cb_bn_browsefile, &qbs_browser_);

	curr_x += bn_brf_qbs_->w() + GAP;
	max_x = max(max_x, curr_x);

	curr_y += HBUTTON + GAP;
	curr_x += bn_brf_qbs_->w() + GAP;
	// End of group
	g_input_->resizable(nullptr);
	g_input_->size(curr_x - g_input_->x(), curr_y - g_input_->y());
	g_input_->end();

	// Process group
	curr_x = x() + GAP;
	curr_y = g_input_->y() + g_input_->h() + GAP;
	g_process_ = new Fl_Group(curr_x, curr_y, 1000, 1000, "Activity");
	g_process_->labelfont(FONT | FL_ITALIC | FL_BOLD);
	g_process_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Process action buttons
	curr_y += HTEXT;
	g_proc_bns_ = new Fl_Group(curr_x, curr_y, 1000, 1000);
	g_proc_bns_->box(FL_FLAT_BOX);

	// Buttons
	curr_x = g_proc_bns_->x() + GAP;
	curr_y = g_proc_bns_->y() + GAP;
	const int WPBUTTON = WBUTTON * 2;
	// Receive batch 
	bn_rcv_batch_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Receive Batch");
	bn_rcv_batch_->labelfont(FONT);
	bn_rcv_batch_->labelsize(FONT_SIZE);
	bn_rcv_batch_->value(action_ == RECEIVE_BATCH);
	rp_rcv_batch_ = radio_param_t(RECEIVE_BATCH, (int*)&action_);
	bn_rcv_batch_->callback(cb_process, &rp_rcv_batch_);
	curr_y += HBUTTON;

	// Receive cards 
	bn_rcv_card_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Receive Card");
	bn_rcv_card_->labelfont(FONT);
	bn_rcv_card_->labelsize(FONT_SIZE);
	bn_rcv_card_->value(action_ == RECEIVE_CARD);
	rp_rcv_card_ = radio_param_t(RECEIVE_CARD, (int*)&action_);
	bn_rcv_card_->callback(cb_process, &rp_rcv_card_);
	curr_y += HBUTTON;

	// Receive SASEs
	bn_rcv_sase_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Receive SASEs");
	bn_rcv_sase_->labelfont(FONT);
	bn_rcv_sase_->labelsize(FONT_SIZE);
	bn_rcv_sase_->value(action_ == RECEIVE_SASE);
	rp_rcv_sase_ = radio_param_t(RECEIVE_SASE, (int*)&action_);
	bn_rcv_sase_->callback(cb_process, &rp_rcv_sase_);
	curr_y += HBUTTON;

	// Move cards in envelopes to outtray
	bn_send_cards_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Move cards to out-tray");
	bn_send_cards_->labelfont(FONT);
	bn_send_cards_->labelsize(FONT_SIZE);
	bn_send_cards_->value(action_ == SEND_CARDS);
	rp_send_cards_ = radio_param_t(SEND_CARDS, (int*)&action_);
	bn_send_cards_->callback(cb_process, &rp_send_cards_);
	curr_y += HBUTTON;

	// Dispose cards - mark for recycling
	bn_dispose_cards_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Mark for recycling");
	bn_dispose_cards_->labelfont(FONT);
	bn_dispose_cards_->labelsize(FONT_SIZE);
	bn_dispose_cards_->value(action_ == DISPOSE_CARDS);
	rp_dispose_cards_ = radio_param_t(DISPOSE_CARDS, (int*)&action_);
	bn_dispose_cards_->callback(cb_process, &rp_dispose_cards_);
	curr_y += HBUTTON;

	// Receive batch 
	bn_post_cards_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Post cards");
	bn_post_cards_->labelfont(FONT);
	bn_post_cards_->labelsize(FONT_SIZE);
	bn_post_cards_->value(action_ == POST_CARDS);
	rp_post_cards_ = radio_param_t(POST_CARDS, (int*)&action_);
	bn_post_cards_->callback(cb_process, &rp_post_cards_);
	curr_y += HBUTTON;

	// Receive batch 
	bn_recycle_cards_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Recycle cards");
	bn_recycle_cards_->labelfont(FONT);
	bn_recycle_cards_->labelsize(FONT_SIZE);
	bn_recycle_cards_->value(action_ == RECYCLE_CARDS);
	rp_recycle_cards_ = radio_param_t(RECYCLE_CARDS, (int*)&action_);
	bn_recycle_cards_->callback(cb_process, &rp_recycle_cards_);
	curr_y += HBUTTON;

	// Receive batch 
	bn_dispose_sase_ = new Fl_Radio_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Dispose SASE");
	bn_dispose_sase_->labelfont(FONT);
	bn_dispose_sase_->labelsize(FONT_SIZE);
	bn_dispose_sase_->value(action_ == DISPOSE_SASE);
	rp_dispose_sase_ = radio_param_t(DISPOSE_SASE, (int*)&action_);
	bn_dispose_sase_->callback(cb_process, &rp_dispose_sase_);
	curr_y += HBUTTON + GAP;
	curr_x += WPBUTTON;

	g_proc_bns_->resizable(nullptr);
	g_proc_bns_->size(curr_x - g_proc_bns_->x(), curr_y - g_proc_bns_->y());
	g_proc_bns_->end();

	curr_x += GAP;
	int save_x = curr_x;
	curr_y = g_proc_bns_->y() + GAP;

	// Batch output
	curr_x += WLABEL;
	op_batch_ = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON, "Batch");
	op_batch_->labelfont(FONT);
	op_batch_->labelsize(FONT_SIZE);
	op_batch_->align(FL_ALIGN_LEFT);
	op_batch_->textfont(FONT);
	op_batch_->textsize(FONT_SIZE);

	// Batch navigation buttons
	curr_x += op_batch_->w();
	bn_b_navbb_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@|<");
	bn_b_navbb_->callback(cb_nav_batch, (void*)(long)PREV_MAJOR);
	curr_x += bn_b_navbb_->w();
	bn_b_navb_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@<");
	bn_b_navb_->callback(cb_nav_batch, (void*)(long)PREV_MINOR);
	curr_x += bn_b_navb_->w();
	bn_b_navf_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@>");
	bn_b_navf_->callback(cb_nav_batch, (void*)(long)NEXT_MAJOR);
	curr_x += bn_b_navf_->w();
	bn_b_navff_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@>|");
	bn_b_navff_->callback(cb_nav_batch, (void*)(long)NEXT_MINOR);
	curr_x += bn_b_navff_->w();
	bn_b_sumall_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@+");
	bn_b_sumall_->callback(cb_nav_batch, (void*)(long)SUMM);
	curr_x += bn_b_sumall_->w() + GAP;
	
	// Batch action button
	bn_b_action_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "New");
	bn_b_action_->labelfont(FONT);
	bn_b_action_->labelsize(FONT_SIZE);
	bn_b_action_->callback(cb_exec_batch, nullptr);
	bn_b_action_->color(FL_GREEN);
	
	curr_x += bn_b_action_->w() + GAP;
	max_x = max(max_x, curr_x);

	// Call input choice
	curr_x = save_x + WLABEL;
	curr_y += HBUTTON + GAP;
	ip_call_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Call");
	ip_call_->labelfont(FONT);
	ip_call_->labelsize(FONT_SIZE);
	ip_call_->align(FL_ALIGN_LEFT);
	ip_call_->textfont(FONT);
	ip_call_->textsize(FONT_SIZE);
	ip_call_->callback(cb_input_call, &call_);

	// Batch navigation buttons
	curr_x += ip_call_->w();
	bn_c_navbb_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@<<");
	bn_c_navbb_->callback(cb_nav_call, (void*)(long)PREV_MAJOR);
	curr_x += bn_c_navbb_->w();
	bn_c_navb_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@<");
	bn_c_navb_->callback(cb_nav_call, (void*)(long)PREV_MINOR);
	curr_x += bn_c_navb_->w();
	bn_c_navf_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@>");
	bn_c_navf_->callback(cb_nav_call, (void*)(long)NEXT_MAJOR);
	curr_x += bn_c_navf_->w();
	bn_c_navff_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@>>");
	bn_c_navff_->callback(cb_nav_call, (void*)(long)NEXT_MINOR);
	curr_x += bn_c_navff_->w();
	bn_c_sumall_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@+");
	bn_c_sumall_->callback(cb_nav_call, (void*)(long)SUMM);
	curr_x += bn_c_sumall_->w() + GAP;

	// Call action button
	bn_c_action_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Execute");
	bn_c_action_->labelfont(FONT);
	bn_c_action_->labelsize(FONT_SIZE);
	bn_c_action_->callback(cb_exec_call, nullptr);
	bn_c_action_->color(FL_GREEN);

	curr_x += bn_c_action_->w() + GAP;
	max_x = max(max_x, curr_x);

	// card count input and output widgets
	int col1 = save_x + WLABEL;
	int col2 = col1 + WBUTTON + GAP;
	curr_y += HTEXT;
	// Labels
	bx_current_ = new Fl_Box(col1, curr_y, WBUTTON, HBUTTON, "Current");
	bx_current_->box(FL_FLAT_BOX);
	bx_current_->labelfont(FONT);
	bx_current_->labelsize(FONT_SIZE);

	bx_change_ = new Fl_Box(col2, curr_y, WBUTTON, HBUTTON, "Diff.");
	bx_change_->box(FL_FLAT_BOX);
	bx_change_->labelfont(FONT);
	bx_change_->labelsize(FONT_SIZE);

	// Count i/o
	curr_y += HBUTTON;
	for (int ix = 0; ix < NUM_COUNTS; ix++) {
		char label[10];
		snprintf(label, 10, "#%d", ix);
		op_value_[ix] = new Fl_Value_Output(col1, curr_y, WBUTTON, HBUTTON);
		op_value_[ix]->labelfont(FONT);
		op_value_[ix]->labelsize(FONT_SIZE);
		op_value_[ix]->copy_label(label);
		op_value_[ix]->align(FL_ALIGN_LEFT);
		op_value_[ix]->step(1);
		op_value_[ix]->bounds(0, 10000);

		ip_delta_[ix] = new Fl_Value_Input(col2, curr_y, WBUTTON, HBUTTON);
		ip_delta_[ix]->step(1);
		ip_delta_[ix]->bounds(0, 10000);
		curr_y += HBUTTON;
	}

	curr_y += GAP;
	curr_x = col2 + WBUTTON + GAP;
	max_x = max(max_x, curr_x);

	// Calculate process group size
	curr_y = max(curr_y, g_proc_bns_->y() + g_proc_bns_->h() + GAP);
	g_process_->resizable(nullptr);
	g_process_->size(max_x - g_process_->x(), curr_y - g_process_->y());

	g_process_->end();

	// Calculate window size
	curr_y = g_process_->y() + g_process_->h() + GAP;
	resizable(nullptr);
	size(max_x, curr_y);

	end();
	show();
	return;
}

// RECEIVE_BATCH
// Enable the defiinition of a new box for the batch, then all
// the movement of cards from the in-box and the newly received cards into
// the new box
void QBS_window::update_rcv_batch() {
	g_process_->label("Receive cards from Bureau");
	// Batch output
	op_batch_->activate();
	selected_box_ = data_->get_current();
	selected_box_++;
	op_batch_->value(data_->get_batch(selected_box_).c_str());
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->activate();
	bn_b_action_->label("Create");
	// Call input - get first call in box
	ip_call_->activate();
	call_ = data_->get_next_call(IN_BOX, "");
	ip_call_->value(call_.c_str());
	// Call navigation buttons -
	bn_c_navbb_->activate();
	bn_c_navb_->activate();
	bn_c_navf_->activate();
	bn_c_navff_->activate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Add cards");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("Add");
	// Count inputs IN-BOX
	int ix = 0;
	op_value_[ix]->activate();
	op_value_[ix]->label("IN-BOX");
	op_value_[ix]->value(data_->get_count(IN_BOX, call_));
	ip_delta_[ix]->deactivate();
	index_inbox_ = ix;
	ix++;
	// New cards in batch - eg "2022 Q4 (C)"
	op_value_[ix]->activate();
	char temp[12];
	snprintf(temp, 12, "%s -C", op_batch_->value());
	op_value_[ix]->copy_label(temp);
	op_value_[ix]->value(0);
	ip_delta_[ix]->activate();
	ip_delta_[ix]->value(0);
	index_curr_ = ix;
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		ip_delta_[ix]->deactivate();
	}
	return;
}

// RECEIVE_CARD
// Add any cards received from elsewhere than the bireau to be 
// stored in the in-box
void QBS_window::update_rcv_card() {
	g_process_->label("Receive individual cards - returns, mis-sorts etc");
	// Batch output
	op_batch_->activate();
	op_batch_->value(data_->get_batch(selected_box_).c_str());
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value("");
	// Call navigation buttons -
	bn_c_navbb_->activate();
	bn_c_navb_->activate();
	bn_c_navf_->activate();
	bn_c_navff_->activate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Log cards");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("Add");
	// Count inputs IN-BOX
	int ix = 0;
	op_value_[ix]->activate();
	op_value_[ix]->label("IN-BOX");
	op_value_[ix]->value(data_->get_count(IN_BOX, call_));
	ip_delta_[ix]->activate();
	ip_delta_[ix]->value(0);
	index_inbox_ = ix;
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		ip_delta_[ix]->deactivate();
	}
	return;
}

// RECEIVE_SASE
// Save new envelopes received in the SASE box
void QBS_window::update_rcv_sase() {
	g_process_->label("Receive SASEs");
	// Batch output
	op_batch_->deactivate();
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	call_ = data_->get_next_call(SASES, "");
	// Call navigation buttons -
	bn_c_navbb_->activate();
	bn_c_navb_->activate();
	bn_c_navf_->activate();
	bn_c_navff_->activate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Add SASEs");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("Add");
	// Count inputs IN-BOX
	int ix = 0;
	op_value_[ix]->activate();
	op_value_[ix]->label("SASES");
	op_value_[ix]->value(data_->get_count(IN_BOX, call_));
	ip_delta_[ix]->activate();
	ip_delta_[ix]->value(0);
	index_inbox_ = ix;
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		ip_delta_[ix]->deactivate();
	}
	return;
}

// SEND_CARDS
// Any cards in kept boxes (current and those in the disposal queue) to
// be put into envelopes and left in the out box.
void QBS_window::update_send_cards() {
	g_process_->label("Stuff cards in envelopes and move to out-tray");
	// Batch output
	op_batch_->activate();
	op_batch_->value(data_->get_batch(selected_box_).c_str());
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	call_ = data_->get_next_call(selected_box_, "");
	// Call navigation buttons -
	bn_c_navbb_->activate();
	bn_c_navb_->activate();
	bn_c_navf_->activate();
	bn_c_navff_->activate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Move cards");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("After");
	// Count inputs KEEP
	int ix = 0;
	op_value_[ix]->activate();
	op_value_[ix]->label("KEEP");
	op_value_[ix]->value(data_->get_count(KEEP_BOX, call_));
	ip_delta_[ix]->activate();
	ip_delta_[ix]->value(0);
	index_keep_ = ix;
	// Now output all the bins
	int curr = data_->get_current();
	int tail = data_->get_tail();
	int head = data_->get_head();
	int box = curr;
	ix++;
	char temp[12];
	while (box >= head) {
		// Display the current and disposal queue boxes (by batch id)
		op_value_[ix]->activate();
		if (box == curr) {
			snprintf(temp, 12, "%s C", data_->get_batch(box).c_str());
		}
		else if (box == head) {
			snprintf(temp, 12, "%s H", data_->get_batch(box).c_str());
		}
		else if (box == tail) {
			snprintf(temp, 12, "%s T", data_->get_batch(box).c_str());
		}
		else {
			snprintf(temp, 12, "%s", data_->get_batch(box).c_str());
		}
		op_value_[ix]->copy_label(temp);
		op_value_[ix]->value(data_->get_count(box, call_));
		ip_delta_[ix]->activate();
		ip_delta_[ix]->value(0);
		if (box == curr) index_curr_ = ix;
		if (box == head) index_head_ = ix;
		box--;
		ix++;
	}
	op_value_[ix]->activate();
	op_value_[ix]->label("OUT BOX");
	op_value_[ix]->value(data_->get_count(OUT_BOX, call_));
	ip_delta_[ix]->deactivate();
	ip_delta_[ix]->value(0);
	index_outbox_ = ix;
	op_value_[ix]->activate();
	op_value_[ix]->label("SASEs used");
	op_value_[ix]->value(0);
	ip_delta_[ix]->activate();
	ip_delta_[ix]->value(0);
	index_sase_ = ix;
	// Deactivate the rest
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		ip_delta_[ix]->deactivate();
	}
	return;
}

// DISPOSE CARDS
// Make the current sorting box, the tail of the disposal queue
void QBS_window::update_dispose_cards() {
	g_process_->label("Mark current batch for disposal");
	// Batch output
	op_batch_->activate();
	selected_box_ = data_->get_current();
	selected_box_++;
	op_batch_->value(data_->get_batch(selected_box_).c_str());
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->activate();
	bn_b_action_->label("Dispose");
	// Call input - get first call in box
	ip_call_->deactivate();
	// Call navigation buttons -
	bn_c_navbb_->deactivate();
	bn_c_navb_->deactivate();
	bn_c_navf_->deactivate();
	bn_c_navff_->deactivate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->deactivate();
	bn_c_action_->label("");
	// Column headers
	bx_current_->deactivate();
	bx_current_->label("");
	bx_change_->deactivate();
	bx_change_->label("");
	for (int ix = 0; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		op_value_[ix]->label("");
		ip_delta_[ix]->deactivate();
	}
	return;
}

// POST CARDS
// Cards in the out box are ready for posting
void QBS_window::update_post_cards() {
	g_process_->label("Mark current batch for disposal");
	// Batch output
	op_batch_->activate();
	selected_box_ = data_->get_current();
	selected_box_++;
	op_batch_->value(data_->get_batch(selected_box_).c_str());
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->activate();
	bn_b_action_->label("Post");
	// Call input - get first call in box
	ip_call_->deactivate();
	// Call navigation buttons -
	bn_c_navbb_->deactivate();
	bn_c_navb_->deactivate();
	bn_c_navf_->deactivate();
	bn_c_navff_->deactivate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->deactivate();
	bn_c_action_->label("");
	// Column headers
	bx_current_->deactivate();
	bx_current_->label("");
	bx_change_->deactivate();
	bx_change_->label("");
	for (int ix = 0; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		op_value_[ix]->label("");
		ip_delta_[ix]->deactivate();
	}
	return;
}

// RECYCLE CARDS
// Cards in the box at the head of the disposal queue are to be recycled
void QBS_window::update_recycle_cards() {
	g_process_->label("Mark current batch for disposal");
	// Batch output
	op_batch_->activate();
	op_batch_->value(data_->get_batch(data_->get_head()).c_str());
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->activate();
	bn_b_action_->label("Recycle");
	// Call input - get first call in box
	ip_call_->deactivate();
	// Call navigation buttons -
	bn_c_navbb_->deactivate();
	bn_c_navb_->deactivate();
	bn_c_navf_->deactivate();
	bn_c_navff_->deactivate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->deactivate();
	bn_c_action_->label("");
	// Column headers
	bx_current_->deactivate();
	bx_current_->label("");
	bx_change_->deactivate();
	bx_change_->label("");
	// Display recycle data
	recycle_data& recycle = data_->get_recycle_data(data_->get_head());
	int ix = 0;
	op_value_[ix]->activate();
	op_value_[ix]->label("Cards rcvd");
	op_value_[ix]->value(recycle.sum_received);
	ip_delta_[ix]->deactivate();

	ix++;
	op_value_[ix]->activate();
	op_value_[ix]->label("..recycled");
	op_value_[ix]->value(recycle.sum_recycled);
	ip_delta_[ix]->deactivate();

	ix++;
	op_value_[ix]->activate();
	op_value_[ix]->label("Calls rcvd");
	op_value_[ix]->value(recycle.count_received);
	ip_delta_[ix]->deactivate();

	ix++;
	op_value_[ix]->activate();
	op_value_[ix]->label("..recycled");
	op_value_[ix]->value(recycle.count_recycled);
	ip_delta_[ix]->deactivate();

	ix++;
	op_value_[ix]->activate();
	op_value_[ix]->label("Weight");
	op_value_[ix]->value(recycle.weight_kg);
	ip_delta_[ix]->activate();
	ip_delta_[ix]->value(0);
	index_weight_ = ix;

	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		op_value_[ix]->label("");
		ip_delta_[ix]->deactivate();
	}
	return;
}

// DISPOSE_SASE
// Remove envelopes from the SASE box and return or recycle them
// TODO: Add a reason for disposal field
void QBS_window::update_dispose_sase() {
	g_process_->label("Discard SASEs");
	// Batch output
	op_batch_->deactivate();
	// Batch navigation buttons
	bn_b_navbb_->deactivate();
	bn_b_navb_->deactivate();
	bn_b_navff_->deactivate();
	bn_b_navf_->deactivate();
	bn_b_sumall_->deactivate();
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	call_ = data_->get_next_call(SASES, "");
	// Call navigation buttons -
	bn_c_navbb_->activate();
	bn_c_navb_->activate();
	bn_c_navf_->activate();
	bn_c_navff_->activate();
	bn_c_sumall_->deactivate();
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Discard SASEs");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("Take");
	// Count inputs IN-BOX
	int ix = 0;
	op_value_[ix]->activate();
	op_value_[ix]->label("SASES");
	op_value_[ix]->value(data_->get_count(IN_BOX, call_));
	ip_delta_[ix]->activate();
	ip_delta_[ix]->value(0);
	index_sase_ = ix;

	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->deactivate();
		ip_delta_[ix]->deactivate();
	}
	return;
}

// Depending on the current processin phase allow/disallow the command
// buttons;
void QBS_window::update_actions(process_mode_t mode) {
	switch (mode) {
	case INITIAL:
	case IMPORT:
	case READING:
		// Not yet processing cards
		g_process_->deactivate();
		g_input_->activate();
		break;
	case DORMANT:
		// Not in an active sorting phase - omly allow command for
		// receiving cards or envelopes
		g_process_->activate();
		g_input_->deactivate();
		bn_rcv_batch_->activate();
		bn_rcv_card_->activate();
		bn_rcv_sase_->activate();
		bn_send_cards_->deactivate();
		bn_dispose_cards_->deactivate();
		bn_post_cards_->deactivate();
		bn_recycle_cards_->activate();
		bn_dispose_cards_->activate();
		break;
	case ACTIVE:
		// In an active sorting phase - allow all commands
		// except that to start a new batch (must finish this one first!)
		g_process_->activate();
		g_input_->deactivate();
		bn_rcv_batch_->deactivate();
		bn_rcv_card_->activate();
		bn_rcv_sase_->activate();
		bn_send_cards_->activate();
		bn_dispose_cards_->activate();
		bn_post_cards_->activate();
		bn_recycle_cards_->activate();
		bn_dispose_cards_->activate();
		break;
	}
	return;
}

// Update batches
void QBS_window::update_batches() {
	if (op_batch_->active()) {
		op_batch_->value(data_->get_batch(selected_box_).c_str());
		if (selected_box_ == 0) {
			bn_b_navbb_->deactivate();
			bn_b_navb_->deactivate();
			bn_b_navf_->activate();
			bn_b_navff_->activate();
		}
		else if (selected_box_ == data_->get_current()) {
			bn_b_navbb_->activate();
			bn_b_navb_->activate();
			bn_b_navf_->deactivate();
			bn_b_navff_->deactivate();
		}
		else {
			bn_b_navbb_->activate();
			bn_b_navb_->activate();
			bn_b_navf_->activate();
			bn_b_navff_->activate();
		}
	}
}

// Update calls
void QBS_window::update_calls() {
	if (op_batch_->active()) {
		op_batch_->value(call_.c_str());
		if (call_ == data_->get_prev_call(selected_box_, call_)) {
			bn_c_navbb_->deactivate();
			bn_c_navb_->deactivate();
			bn_c_navf_->activate();
			bn_c_navff_->activate();
		}
		else if (call_ == data_->get_next_call(selected_box_, call_)) {
			bn_c_navbb_->activate();
			bn_c_navb_->activate();
			bn_c_navf_->deactivate();
			bn_c_navff_->deactivate();
		}
		else {
			bn_c_navbb_->activate();
			bn_c_navb_->activate();
			bn_c_navf_->activate();
			bn_c_navff_->activate();
		}
	}
}

// Update import
void QBS_window::update_import(float loaded) {
	// BLACK - not loaded
	// YELLOW - loading
	// RED - all files loaded
	Fl_Color colour;
	switch ((int)trunc(loaded)) {
	case 0:
		colour = FL_BACKGROUND_COLOR;
		break;
	case 1:
		colour = fl_color_average(FL_YELLOW, FL_GREEN, 2.0F - loaded);
		break;
	case 2:
		colour = FL_GREEN;
		break;
	default:
		colour = FL_BLACK;
		break;
	}
	bn_import_->color(colour);
	Fl::wait();
	return;
}

// Update QBS 
void QBS_window::update_qbs(int loaded) {
	// BLACK - not loaded
	// YELLOW - loading
	// RED - all files loaded
	Fl_Color colour;
	switch (loaded) {
	case 0:
		colour = FL_BACKGROUND_COLOR;
		break;
	case 1:
		colour = FL_YELLOW;
		break;
	case 2:
		colour = FL_GREEN;
		break;
	default:
		colour = FL_BLACK;
		break;
	}
	bn_read_qbs_->color(colour);
	Fl::wait();
	return;
}

// Callback - import CSV data - pass the current directory to the data to 
// start redaing it
void QBS_window::cb_import(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	that->update_import(0);
	that->data_->import_cvs(that->csv_directory_);
	return;
}

// Callback - read QBS data - pass filename to data to start reading it
void QBS_window::cb_read_qbs(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	that->update_qbs(false);
	that->data_->read_qbs(that->qbs_filename_);
	return;
}

// Callback - process selection button pressed, select the appropriare
// command
void QBS_window::cb_process(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	// Get radio value - to action_
	cb_radio(w, v);
	// depending on action_ value configure the widgets accordingly
	switch (that->action_) {
	case RECEIVE_BATCH:
		that->update_rcv_batch();
		break;
	case RECEIVE_CARD:
		that->update_rcv_card();
		break;
	case RECEIVE_SASE:
		that->update_rcv_sase();
		break;
	case SEND_CARDS:
		that->update_send_cards();
		break;
	case DISPOSE_CARDS:
		that->update_dispose_cards();
		break;
	case POST_CARDS:
		that->update_post_cards();
		break;
	case RECYCLE_CARDS:
		that->update_recycle_cards();
		break;
	case DISPOSE_SASE:
		that->update_dispose_sase();
		break;
	}
	return;
}

// Callback - execute batch
// RECEIVE_BATCH: create new batch
// POST_CARDS: change box status
// DISPOSE_CARDS: ditto
// RECYCLE_CARDS: ditto
void QBS_window::cb_exec_batch(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	string date = now(true, DATE_FORMAT);
	string batch = that->op_batch_->value();
	float weight = (float)that->ip_delta_[that->index_weight_]->value();
	switch (that->action_) {
	case RECEIVE_BATCH:
		that->data_->new_batch(that->selected_box_, date, batch);
		that->update_rcv_batch();
		break;
	case POST_CARDS:
		that->data_->post_cards(date);
		that->update_post_cards();
		break;
	case DISPOSE_CARDS:
		that->data_->dispose_cards(date);
		that->update_dispose_cards();
		break;
	case RECYCLE_CARDS:
		that->data_->recycle_cards(date, weight);
		that->update_recycle_cards();
		break;
	}
	return;
}

// Callback - execute call
// RECEIVE_BATCH: Add inputs from in-box and received cards to current box
// RECEIVE_CARD: Add received cards to in-box
// RECEIVE_SASE: Add received envelopes to SASE box
// DISPOSE_SASE: Remove envelopes from SASE box
// SEND_BATCH: Move envelopes from all identified boxes to out-box
//             Remove envelopes from SASE box 
void QBS_window::cb_exec_call(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	string date = now(true, DATE_FORMAT);
	string call = that->ip_call_->value();
	int num_cards = 0;
	int num_sases = 0;
	switch (that->action_) {
	case RECEIVE_BATCH:
		num_cards = (int)that->ip_delta_[that->index_inbox_]->value();
		// Remove them from inbox
		that->data_->receive_cards(IN_BOX, date, call, -(num_cards));
		// Add them and the current ones to current_box
		num_cards += (int)that->ip_delta_[that->index_curr_]->value();
		that->data_->receive_cards(that->selected_box_, date, call, num_cards);
		that->update_rcv_batch();
		break;
	case RECEIVE_CARD:
		num_cards = (int)that->ip_delta_[that->index_inbox_]->value();
		that->data_->receive_cards(IN_BOX, date, call, num_cards);
		that->update_rcv_card();
		break;
	case RECEIVE_SASE:
		num_sases = (int)that->ip_delta_[that->index_sase_]->value();
		that->data_->receive_sases(date, call, num_sases);
		that->update_rcv_sase();
		break;
	case DISPOSE_SASE:
		num_sases = (int)that->ip_delta_[that->index_sase_]->value();
		that->data_->receive_sases(date, call, -(num_sases));
		that->update_dispose_sase();
		break;
	case SEND_CARDS:
		// Send those in keep
		num_cards = (int)that->ip_delta_[that->index_keep_]->value();
		that->data_->send_cards(KEEP_BOX, date, call, num_cards);
		// From HEAD to CURR - send cards
		int curr = that->data_->get_current();
		int head = that->data_->get_head();
		int box = head;
		int ix = that->index_head_;
		while (ix >= that->index_curr_ && box <= curr) {
			num_cards = (int)that->ip_delta_[ix]->value();
			that->data_->send_cards(box, date, call, num_cards);
			ix--;
			box++;
		}
		// Use SASEs
		num_sases = (int)that->ip_delta_[that->index_sase_]->value();
		that->data_->use_sases(date, call, num_sases);
		that->update_send_cards();
		break;
	}
}

// Call back - call input
// TODO: Check call exists etc
void QBS_window::cb_input_call(Fl_Widget* w, void* v) {
	cb_value<Fl_Input, string>(w, v);
}

// Callback navigate batch
void QBS_window::cb_nav_batch(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	navigate_t nav_action = (navigate_t)(long)v;
	switch (nav_action) {
	case SUMM:
		cout << "Batch summation not yet implemented" << endl;
		break;
	case PREV_MAJOR:
		// Go to first box
		that->selected_box_ = 0;
		break;
	case PREV_MINOR:
		// Go to previous box
		if (that->selected_box_ > 0) {
			that->selected_box_--;
		}
		else {
			that->selected_box_ = 0;
		}
		break;
	case NEXT_MINOR:
		// Go to next box
		if (that->selected_box_ < that->data_->get_current()) {
			that->selected_box_--;
		}
		else {
			that->selected_box_ = that->data_->get_current();
		}
		break;
	case NEXT_MAJOR:
		that->selected_box_ = that->data_->get_current();
		break;
	}
	that->update_batches();
	return;
}

// Callback navigate call
void QBS_window::cb_nav_call(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	navigate_t nav_action = (navigate_t)(long)v;
	string call = that->call_;
	string pfx = call.substr(0, 3);
	switch (nav_action) {
	case SUMM:
		cout << "Batch summation not yet implemented" << endl;
		break;
	case PREV_MAJOR:
		while (call.substr(0, 3) == pfx) {
			call = that->data_->get_prev_call(that->selected_box_, call);
		}
		that->call_ = call;
		break;
	case PREV_MINOR:
		that->call_ = that->data_->get_prev_call(that->selected_box_, call);
		break;
	case NEXT_MAJOR:
		while (call.substr(0, 3) == pfx) {
			call = that->data_->get_next_call(that->selected_box_, call);
		}
		call = that->data_->get_next_call(that->selected_box_, call);
		that->call_ = call;
		break;
	case NEXT_MINOR:
		that->call_ = that->data_->get_next_call(that->selected_box_, call);
		break;
	}
	that->update_calls();
	return;
}

// Display top 20
void QBS_window::display_top20() {
	cerr << "Display top 20 not yet implemented " << endl;
	// TODO Implemet it
	return;
}