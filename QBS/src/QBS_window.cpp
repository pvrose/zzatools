#include "QBS_window.h"
#include "QBS_consts.h"
#include "QBS_data.h"
#include "QBS_callback.h"

#include <iostream>

using namespace std;

const extern char* DATE_FORMAT;
const Fl_Color COLOUR_ORANGE = 93; /* R=4/4, B=0/4, G = 5/7 */
const Fl_Color COLOUR_ADHOC = fl_lighter(FL_CYAN);
const Fl_Color COLOUR_BATCH = fl_lighter(COLOUR_ORANGE);
const Fl_Color COLOUR_REPORT = fl_lighter(FL_MAGENTA);
const Fl_Color COLOUR_RESET = fl_lighter(FL_RED);
const Fl_Color COLOUR_CLOSE = fl_lighter(FL_GREEN);
const Fl_Color COLOUR_NOTES = FL_YELLOW;

// Constructor
QBS_window::QBS_window(int W, int H, const char* L, const char* filename) :
	Fl_Single_Window(W, H, L),
	settings_(Fl_Preferences::USER, VENDOR.c_str(), PROGRAM_ID.c_str()),
	data_(nullptr),
	qbs_filename_(filename),
	batch_op_done_(false),
	pos_batch_log_(0),
	default_inputs_(true)
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
	callback(cb_close);
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
	that->settings_.set("CSV Directory", that->csv_directory_.c_str());
	that->settings_.set("Filename", that->qbs_filename_.c_str());
	that->settings_.flush();
	Fl_Single_Window::default_callback(that, v);
	delete that->data_;
	return;
}

// Instantiate all the widgets
void QBS_window::create_form() {
	const int WPBUTTON = WBUTTON * 2;
	int curr_x = GAP;
	int curr_y = GAP;
	int max_x = curr_x;

	g_input_ = new Fl_Group(curr_x, curr_y, 1000, 1000, "Data input");
	g_input_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	g_input_->labelfont(FL_ITALIC | FL_BOLD);

	// Import CSV
	// This button when pressed will instigate the import CSV process
	// It will display in the light the status of file imports
	// BLACK  - not loaded
	// YELLOW - loading
	// RED    - all files loaded
	curr_x += GAP;
	curr_y += HTEXT;
	bn_import_ = new Fl_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Import CSV");
	bn_import_->value(0);
	bn_import_->callback(cb_import, nullptr);

	// CVS directory name
	// This input cab ne used to input the V directory name - use of the
	// associated browse button will fill it in
	curr_x += bn_import_->w() + GAP + WLABEL;
	ip_csv_dir_ = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON);
	ip_csv_dir_->value(csv_directory_.c_str());

	// CVS browse directory
	// This will open the native file browser to get the directory
	curr_x += ip_csv_dir_->w() + GAP;
	bn_brf_csv_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Browse");
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
	bn_read_qbs_ = new Fl_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Read file");
	bn_read_qbs_->value(0);
	bn_read_qbs_->callback(cb_read_qbs, nullptr);

	// QBS filename name
	// This input cab ne used to input the QBS filename - use of the
	// associated browse button will fill it in
	curr_x += bn_read_qbs_->w() + GAP + WLABEL;
	ip_qbs_file_ = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON);
	ip_qbs_file_->value(qbs_filename_.c_str());

	// CVS browse directory
	// This will open the native file browser to get the directory
	curr_x += ip_qbs_file_->w() + GAP;
	bn_brf_qbs_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Browse");
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

	// End of group
	g_input_->resizable(nullptr);
	g_input_->size(max_x - g_input_->x(), curr_y - g_input_->y());
	g_input_->end();

	// Process group
	curr_x = x() + GAP;
	curr_y = g_input_->y() + g_input_->h() + GAP;
	g_process_ = new Fl_Group(curr_x, curr_y, 1000, 1000, "Activity");
	g_process_->labelfont(FL_ITALIC | FL_BOLD);
	g_process_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Process action buttons
	curr_y += HTEXT;
	g_proc_bns_ = new Fl_Group(curr_x, curr_y, 1000, 1000);
	g_proc_bns_->box(FL_FLAT_BOX);

	// Buttons
	curr_x = g_proc_bns_->x() + GAP;
	curr_y = g_proc_bns_->y() + GAP;

	// Receive card 
	bn_rcv_card_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Receive Card");
	bn_rcv_card_->color(COLOUR_ADHOC);
	bn_rcv_card_->selection_color(FL_BLACK);
	bn_rcv_card_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_rcv_card_->color()));
	rp_rcv_card_ = radio_param_t(RECEIVE_CARD, (int*)&action_);
	bn_rcv_card_->callback(cb_process, &rp_rcv_card_);
	curr_y += HBUTTON;

	// Receive SASEs
	bn_rcv_sase_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Receive SASEs");
	bn_rcv_sase_->color(COLOUR_ADHOC);
	bn_rcv_sase_->selection_color(FL_BLACK);
	bn_rcv_sase_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_rcv_sase_->color()));
	rp_rcv_sase_ = radio_param_t(RECEIVE_SASE, (int*)&action_);
	bn_rcv_sase_->callback(cb_process, &rp_rcv_sase_);
	curr_y += HBUTTON;

	// Dispose SASEs
	bn_dispose_sase_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Dispose SASE");
	bn_dispose_sase_->color(COLOUR_ADHOC);
	bn_dispose_sase_->selection_color(FL_BLACK);
	bn_dispose_sase_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_dispose_sase_->color()));
	rp_dispose_sase_ = radio_param_t(DISPOSE_SASE, (int*)&action_);
	bn_dispose_sase_->callback(cb_process, &rp_dispose_sase_);
	curr_y += HBUTTON;

	curr_y += GAP;

	// Receive batch 
	bn_new_batch_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "New Batch");
	bn_new_batch_->color(COLOUR_BATCH);
	bn_new_batch_->selection_color(FL_BLACK);
	bn_new_batch_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_new_batch_->color()));
	rp_new_batch_ = radio_param_t(NEW_BATCH, (int*)&action_);
	bn_new_batch_->callback(cb_process, &rp_new_batch_);
	curr_y += HBUTTON;

	// Receive cards (in batch)
	bn_sort_cards_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Sort received cards");
	bn_sort_cards_->color(COLOUR_BATCH);
	bn_sort_cards_->selection_color(FL_BLACK);
	bn_sort_cards_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_sort_cards_->color()));
	rp_sort_cards_ = radio_param_t(SORT_CARDS, (int*)&action_);
	bn_sort_cards_->callback(cb_process, &rp_sort_cards_);
	curr_y += HBUTTON;

	// Move cards in envelopes to outtray
	bn_stuff_cards_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Stuff cards in SASEs");
	bn_stuff_cards_->color(COLOUR_BATCH);
	bn_stuff_cards_->selection_color(FL_BLACK);
	bn_stuff_cards_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_stuff_cards_->color()));
	rp_stuff_cards_ = radio_param_t(STUFF_CARDS, (int*)&action_);
	bn_stuff_cards_->callback(cb_process, &rp_stuff_cards_);
	curr_y += HBUTTON;

	// Keep cards in KEEP_BOX
	bn_keep_cards_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Keep cards");
	bn_keep_cards_->color(COLOUR_BATCH);
	bn_keep_cards_->selection_color(FL_BLACK);
	bn_keep_cards_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_keep_cards_->color()));
	rp_keep_cards_ = radio_param_t(KEEP_CARDS, (int*)&action_);
	bn_keep_cards_->callback(cb_process, &rp_keep_cards_);
	curr_y += HBUTTON;

	// Empty out box 
	bn_post_cards_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Post cards");
	bn_post_cards_->color(COLOUR_BATCH);
	bn_post_cards_->selection_color(FL_BLACK);
	bn_post_cards_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_post_cards_->color()));
	rp_post_cards_ = radio_param_t(POST_CARDS, (int*)&action_);
	bn_post_cards_->callback(cb_process, &rp_post_cards_);
	curr_y += HBUTTON;

	// Dispose cards - mark for recycling
	bn_dispose_cards_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Finish processing");
	bn_dispose_cards_->color(COLOUR_BATCH);
	bn_dispose_cards_->selection_color(FL_BLACK);
	bn_dispose_cards_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_dispose_cards_->color()));
	rp_dispose_cards_ = radio_param_t(DISPOSE_CARDS, (int*)&action_);
	bn_dispose_cards_->callback(cb_process, &rp_dispose_cards_);
	curr_y += HBUTTON;

	// Receive batch 
	bn_recycle_cards_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Recycle cards");
	bn_recycle_cards_->color(COLOUR_BATCH);
	bn_recycle_cards_->selection_color(FL_BLACK);
	bn_recycle_cards_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_recycle_cards_->color()));
	rp_recycle_cards_ = radio_param_t(RECYCLE_CARDS, (int*)&action_);
	bn_recycle_cards_->callback(cb_process, &rp_recycle_cards_);
	curr_y += HBUTTON;

	curr_y += GAP;

	// Allow correction of data without any transsfers
	bn_correction_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Correct data");
	bn_correction_->color(COLOUR_NOTES);
	bn_correction_->selection_color(FL_BLACK);
	bn_correction_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_correction_->color()));
	rp_correction_ = radio_param_t(CORRECT_DATA, (int*)&action_);
	bn_correction_->callback(cb_process, &rp_correction_);
	curr_y += HBUTTON;

	// Edit notes
	bn_edit_notes_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Edit Notes");
	bn_edit_notes_->color(COLOUR_NOTES);
	bn_edit_notes_->selection_color(FL_BLACK);
	bn_edit_notes_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_edit_notes_->color()));
	rp_edit_notes_ = radio_param_t(EDIT_NOTES, (int*)&action_);
	bn_edit_notes_->callback(cb_process, &rp_edit_notes_);
	curr_y += HBUTTON;

	curr_y += GAP;

	bn_summ_batch_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Batch summary report");
	bn_summ_batch_->color(COLOUR_REPORT);
	bn_summ_batch_->selection_color(FL_BLACK);
	bn_summ_batch_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_summ_batch_->color()));
	rp_summ_batch_ = radio_param_t(SUMMARY_BATCH, (int*)&action_);
	bn_summ_batch_->callback(cb_process, &rp_summ_batch_);
	curr_y += HBUTTON;

	bn_list_batch_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Batch report");
	bn_list_batch_->color(COLOUR_REPORT);
	bn_list_batch_->selection_color(FL_BLACK);
	bn_list_batch_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_list_batch_->color()));
	rp_list_batch_ = radio_param_t(LIST_BATCH, (int*)&action_);
	bn_list_batch_->callback(cb_process, &rp_list_batch_);
	curr_y += HBUTTON;

	bn_summ_call_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Call summary report");
	bn_summ_call_->color(COLOUR_REPORT);
	bn_summ_call_->selection_color(FL_BLACK);
	bn_summ_call_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_summ_call_->color()));
	rp_summ_call_ = radio_param_t(SUMMARY_CALL, (int*)&action_);
	bn_summ_call_->callback(cb_process, &rp_summ_call_);
	curr_y += HBUTTON;

	bn_hist_call_ = new Fl_Radio_Light_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Call history report");
	bn_hist_call_->color(COLOUR_REPORT);
	bn_hist_call_->selection_color(FL_BLACK);
	bn_hist_call_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_hist_call_->color()));
	rp_hist_call_ = radio_param_t(HISTORY_CALL, (int*)&action_);
	bn_hist_call_->callback(cb_process, &rp_hist_call_);
	curr_y += HBUTTON;

	curr_y += GAP;

	bn_reset_ = new Fl_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Reset");
	bn_reset_->color(COLOUR_RESET);
	bn_reset_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_reset_->color()));
	bn_reset_->labelfont(FL_BOLD_ITALIC);
	bn_reset_->callback(cb_reset, nullptr);
	curr_y += HBUTTON;

	bn_close_ = new Fl_Button(curr_x, curr_y, WPBUTTON, HBUTTON, "Close");
	bn_close_->color(COLOUR_CLOSE);
	bn_close_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_close_->color()));
	bn_close_->labelfont(FL_BOLD_ITALIC);
	bn_close_->callback(cb_close, nullptr);

	curr_x += WPBUTTON;
	curr_y += HBUTTON;

	g_proc_bns_->resizable(nullptr);
	g_proc_bns_->size(curr_x - g_proc_bns_->x(), curr_y - g_proc_bns_->y());
	g_proc_bns_->end();

	curr_x += GAP;
	int save_x = curr_x;
	curr_y = g_proc_bns_->y() + GAP;

	// Batch output
	curr_x += WLABEL;
	ch_batch_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Batch");
	ch_batch_->align(FL_ALIGN_LEFT);
	// Batch navigation buttons
	curr_x += ch_batch_->w() + GAP;

	// Batch action button
	bn_b_action_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "New");
	bn_b_action_->callback(cb_exec_batch, nullptr);
	bn_b_action_->color(FL_GREEN);
	bn_b_action_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_b_action_->color()));

	curr_x += bn_b_action_->w() + GAP;
	max_x = max(max_x, curr_x);

	// Call input choice
	curr_x = save_x + WLABEL;
	curr_y += HBUTTON;
	ip_call_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Call");
	ip_call_->align(FL_ALIGN_LEFT);
	ip_call_->callback(cb_input_call, &call_);
	ip_call_->when(FL_WHEN_CHANGED);

	curr_x += ip_call_->w();

	curr_x += GAP;
	// Call action button
	bn_c_action_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Execute");
	bn_c_action_->callback(cb_exec_call, nullptr);
	bn_c_action_->color(FL_GREEN);
	bn_c_action_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_c_action_->color()));

	curr_x += bn_c_action_->w() + GAP;
	max_x = max(max_x, curr_x);

	curr_y += HBUTTON;
	curr_x = save_x + WLABEL;

	// Prepopulate input values with default
	bn_use_defaults_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Use default values");
	bn_use_defaults_->value(default_inputs_);
	bn_use_defaults_->align(FL_ALIGN_RIGHT);
	bn_use_defaults_->callback(cb_value<Fl_Check_Button, bool>, (void*)&default_inputs_);
	curr_y += bn_use_defaults_->h();

	// card count input and output widgets
	// save_x and save_y are the common (X,Y) for counts and notes
	int col1 = save_x + WLABEL;
	int col2 = col1 + WBUTTON + GAP;
	int col3 = col2 + WBUTTON + GAP;
	curr_y += HTEXT;
	int save_y = curr_y;

	// Labels
	bx_current_ = new Fl_Box(col1, curr_y, WBUTTON, HBUTTON, "Current");
	bx_current_->box(FL_FLAT_BOX);

	bx_change_ = new Fl_Box(col2, curr_y, WBUTTON, HBUTTON, "Diff.");
	bx_change_->box(FL_FLAT_BOX);

	// Count i/o
	curr_y += HBUTTON;
	for (int ix = 0; ix < NUM_COUNTS; ix++) {
		char label[10];
		snprintf(label, 10, "#%d", ix);
		op_value_[ix] = new Fl_Output(col1, curr_y, WBUTTON, HBUTTON);
		op_value_[ix]->copy_label(label);
		op_value_[ix]->align(FL_ALIGN_LEFT);
		op_value_[ix]->color(FL_BACKGROUND_COLOR);

		ip_delta_[ix] = new Fl_Input(col2, curr_y, WBUTTON, HBUTTON);
		ip_delta_[ix]->callback(cb_ip_enter, nullptr);
		ip_delta_[ix]->when(FL_WHEN_ENTER_KEY_ALWAYS);

		bx_label_[ix] = new Fl_Box(col3, curr_y, WBUTTON, HBUTTON);
		bx_label_[ix]->box(FL_FLAT_BOX);
		bx_label_[ix]->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
		bx_label_[ix]->color(FL_BACKGROUND_COLOR);
		curr_y += HBUTTON;
	}

	curr_y += GAP;
	curr_x = col2 + WBUTTON + GAP;
	max_x = max(max_x, curr_x);
	curr_x = save_x + WLABEL;

	// Create the full width of the above
	const int WNOTES = max_x - GAP - curr_x;
	tab_old_notes_ = new QBS_notes(curr_x, curr_y, WNOTES, 100, "Current\nNotes");
	tab_old_notes_->align(FL_ALIGN_LEFT_TOP);
	max_x = max(max_x, tab_old_notes_->x() + tab_old_notes_->w());
	
	curr_y += tab_old_notes_->h();

	int max_y = curr_y;

	// Now create the notes widgets in the same place
	curr_y = save_y + GAP;
	curr_x = save_x + WLABEL;

	op_note_date_ = new Fl_Output(curr_x, curr_y, WNOTES, HBUTTON, "Date");
	op_note_date_->align(FL_ALIGN_LEFT);
	op_note_date_->color(FL_BACKGROUND_COLOR);
	max_x = max(max_x, op_note_date_->x() + op_note_date_->w());

	curr_y += op_note_date_->h();
	ip_note_name_ = new Fl_Input(curr_x, curr_y, WNOTES, HBUTTON, "Name");
	ip_note_name_->align(FL_ALIGN_LEFT);
	max_x = max(max_x, ip_note_name_->x() + ip_note_name_->w());

	curr_y += ip_note_name_->h();
	ip_note_value_ = new Fl_Input(curr_x, curr_y, WNOTES, HBUTTON, "Value");
	ip_note_value_->align(FL_ALIGN_LEFT);
	max_x = max(max_x, ip_note_value_->x() + ip_note_value_->w());

	curr_y += ip_note_value_->h() + GAP;
	max_y = max(max_y, curr_y);

	// Calculate process group size
	max_y = max(max_y, g_proc_bns_->y() + g_proc_bns_->h() + GAP);


	g_process_->resizable(nullptr);
	g_process_->size(max_x - g_process_->x(), max_y - g_process_->y());

	g_process_->end();

	// Calculate window size
	max_y = g_process_->y() + g_process_->h() + GAP;
	curr_x = col3 + WLABEL + GAP;

	g_charts_ = new QBS_charth(curr_x, save_y, 160, 100, "Card history");
	g_charts_->data(data_);

	max_x = max(max_x, curr_x + g_charts_->w() + GAP);

	// Log display
	curr_x = GAP;
	curr_y = max_y + GAP;
	Fl_Text_Buffer* buffer = new Fl_Text_Buffer;
	td_log_ = new Fl_Text_Display(curr_x, curr_y, max_x - curr_x - GAP, 5 * HTEXT, "Session log");
	td_log_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	td_log_->labelfont(FL_ITALIC | FL_BOLD);
	td_log_->buffer(buffer);
	td_log_->textfont(FL_COURIER);

	max_y = td_log_->y() + td_log_->h() + GAP;

	resizable(nullptr);
	size(max_x, max_y);

	end();
	show();
	return;
}

// RECEIVE_NEW_BATCH
// Enable the defiinition of a new box for the batch, then all
// the movement of cards from the in-box and the newly received cards into
// the new box
void QBS_window::update_new_batch() {
	g_process_->label("Receive new batch from Bureau");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	selected_box_++;
	bn_b_action_->label("New box");
	if (batch_op_done_) bn_b_action_->deactivate();
	else bn_b_action_->activate();
	update_batches(false);
	// Call input - get first call in box
	ip_call_->deactivate();
	// Action button
	bn_c_action_->deactivate();
	update_calls(selected_box_);
	bx_current_->deactivate();
	bx_current_->label("Existing");
	bx_change_->deactivate();
	bx_change_->label("Add");
	// Count inputs IN-BOX
	int ix = 0;
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// RECEIVE_CARDS(BATCH)
// Enable the defiinition of a new box for the batch, then all
// the movement of cards from the in-box and the newly received cards into
// the new box
void QBS_window::update_sort_cards() {
	char buff[32];
	g_process_->label("Receive cards from Bureau");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	// Batch navigation buttons
	update_batches(false);
	// Button action
	bn_b_action_->label("");
	bn_b_action_->deactivate();
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	// Call navigation buttons -
	update_calls(RCVD_BOX);
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
	op_value_[ix]->show();
	op_value_[ix]->label("IN");
	snprintf(buff, 32, "%0.0f", data_->get_count(IN_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();
	index_inbox_ = ix;
	ix++;
	// Count inputs KEEP BOX
	op_value_[ix]->show();
	op_value_[ix]->label("KEEP");
	snprintf(buff, 32, "%0.0f", data_->get_count(KEEP_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();
	index_keep_ = ix;
	ix++;
	// New cards in batch - eg "2022 Q4 (C)"
	op_value_[ix]->show();
	op_value_[ix]->label("CURRENT");
	snprintf(buff, 32, "%0.0f", data_->get_count(data_->get_current(), call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->show();
	bx_label_[ix]->copy_label(data_->get_batch(ch_batch_->value()).c_str());
	index_curr_ = ix;
	ix++;
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();

	}
	// Hide the note widgets
	hide_edit_notes(true);
	update_history(true);
	return;
}

// KEEP_CARDS
// Move specified cards into the 
void QBS_window::update_keep_cards() {
	char buff[32];
	g_process_->label("Save cards from recycling box");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	// Batch navigation buttons
	update_batches(false);
	// Button action
	bn_b_action_->label("");
	bn_b_action_->deactivate();
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	// Call navigation buttons -
	update_calls(RCVD_BOX);
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Keep cards");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("Add");
	// Count inputs IN-BOX
	int ix = 0;
	// Count inputs KEEP BOX
	op_value_[ix]->show();
	op_value_[ix]->label("KEEP");
	snprintf(buff, 32, "%0.0f", data_->get_count(KEEP_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_keep_ = ix;
	ix++;
	// New cards in batch - eg "2022 Q4 (C)"
	op_value_[ix]->show();
	op_value_[ix]->label("CURRENT");
	snprintf(buff, 32, "%0.0f", data_->get_count(data_->get_current(), call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->show();
	bx_label_[ix]->copy_label(data_->get_batch(ch_batch_->value()).c_str());
	index_curr_ = ix;
	ix++;
	for (; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(true);
	update_history(true);
	return;
}


// RECEIVE_CARD(SINGLE)
// Add any cards received from elsewhere than the bireau to be 
// stored in the in-box
void QBS_window::update_rcv_card() {
	char buff[32];
	g_process_->label("Receive individual cards - returns, mis-sorts etc");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	// Call navigation buttons -
	update_calls(RCVD_BOX);
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
	op_value_[ix]->show();
	op_value_[ix]->label("IN-BOX");
	snprintf(buff, 32, "%0.0f", data_->get_count(IN_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_inbox_ = ix;
	ix++;
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(true);
	update_history(true);
	return;
}

// RECEIVE_SASE
// Save new envelopes received in the SASE box
void QBS_window::update_rcv_sase() {
	char buff[32];
	g_process_->label("Receive SASEs");
	// Batch output
	ch_batch_->deactivate();
	selected_box_ = data_->get_current();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	update_calls(RCVD_BOX);
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
	op_value_[ix]->show();
	op_value_[ix]->label("SASES");
	snprintf(buff, 32, "%0.0f", data_->get_count(SASE_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_sase_ = ix;
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(true);
	update_history(true);
	return;
}

// SEND_CARDS
// Any cards in kept boxes (current and those in the disposal queue) to
// be put into envelopes and left in the out box.
void QBS_window::update_stuff_cards() {
	char buff[32];
	g_process_->label("Stuff cards in envelopes and move to out-tray");
	selected_box_ = data_->get_current();
	// Batch output
	ch_batch_->activate();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	update_calls(selected_box_);
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Move cards");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("Use");
	// Count inputs KEEP
	int ix = 0;
	op_value_[ix]->show();
	op_value_[ix]->label("KEEP");
	snprintf(buff, 32, "%0.0f", data_->get_count(KEEP_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value(default_inputs_ ? buff: "0");
	index_keep_ = ix;
	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("IN-BOX");
	snprintf(buff, 32, "%0.0f", data_->get_count(IN_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value(default_inputs_ ? buff : "0");
	index_inbox_ = ix;
	// Now output all the bins
	int curr = data_->get_current();
	int tail = data_->get_tail();
	int head = data_->get_head();
	int box = curr;
	ix++;
	while (box >= head) {
		// Display the current and disposal queue boxes (by batch id)
		op_value_[ix]->show();
		if (box == curr) {
			op_value_[ix]->label("CURRENT");
		}
		else if (box == head) {
			op_value_[ix]->label("HEAD");
		}
		else if (box == tail) {
			op_value_[ix]->label("TAIL");
		}
		else {
			op_value_[ix]->label("");
		}
		snprintf(buff, 32, "%0.0f", data_->get_count(box, call_));
		op_value_[ix]->value(buff);
		ip_delta_[ix]->show();
		ip_delta_[ix]->value(default_inputs_ ? buff : "0");
		bx_label_[ix]->show();
		bx_label_[ix]->copy_label(data_->get_batch(box).c_str());
		if (box == curr) index_curr_ = ix;
		if (box == head) index_head_ = ix;
		box--;
		ix++;
	}
	op_value_[ix]->show();
	op_value_[ix]->label("OUT BOX");
	snprintf(buff, 32, "%0.0f", data_->get_count(OUT_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_outbox_ = ix;
	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("SASEs");
	snprintf(buff, 32, "%0.0f", data_->get_count(SASE_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_sase_ = ix;
	ix++;
	
	// Deactivate the rest
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(true);
	update_history(true);
	return;
}

// DISPOSE CARDS
// Make the current sorting box, the tail of the disposal queue
void QBS_window::update_dispose_cards() {
	g_process_->label("Mark current batch for recycling");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	update_batches(false);
	// Button action
	if (batch_op_done_) bn_b_action_->deactivate();
	else bn_b_action_->activate();
	bn_b_action_->label("Dispose");
	// Call input - get first call in box
	ip_call_->deactivate();
	update_calls(selected_box_);
	// Action button
	bn_c_action_->deactivate();
	bn_c_action_->label("");
	// Column headers
	bx_current_->deactivate();
	bx_current_->label("");
	bx_change_->deactivate();
	bx_change_->label("");
	for (int ix = 0; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		op_value_[ix]->label("");
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// POST CARDS
// Cards in the out box are ready for posting
void QBS_window::update_post_cards() {
	g_process_->label("Mark current batch for disposal");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	update_batches(false);
	// Button action
	if (batch_op_done_) bn_b_action_->deactivate();
	else bn_b_action_->activate();
	bn_b_action_->label("Post");
	// Call input - get first call in box
	ip_call_->deactivate();
	// Call navigation buttons -
	update_calls(selected_box_);
	// Action button
	bn_c_action_->deactivate();
	bn_c_action_->label("");
	// Column headers
	bx_current_->deactivate();
	bx_current_->label("");
	bx_change_->deactivate();
	bx_change_->label("");
	for (int ix = 0; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		op_value_[ix]->label("");
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// RECYCLE CARDS
// Cards in the box at the head of the disposal queue are to be recycled
void QBS_window::update_recycle_cards() {
	char buff[32];
	g_process_->label("Mark head of disposal queue for recycling");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_head();
	update_batches(false);
	// Button action
	if (batch_op_done_) bn_b_action_->deactivate();
	else bn_b_action_->activate();
	bn_b_action_->label("Recycle");
	// Call input - get first call in box
	ip_call_->deactivate();
	update_calls(data_->get_head());
	// Action button
	bn_c_action_->deactivate();
	bn_c_action_->label("");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Recycling");
	bx_change_->deactivate();
	bx_change_->label("");
	// Display recycle data
	recycle_data& recycle = data_->get_recycle_data(data_->get_head());
	int ix = 0;
	op_value_[ix]->show();
	op_value_[ix]->label("Cards rcvd");
	snprintf(buff, 32, "%d", recycle.sum_received);
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();

	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("..recycled");
	snprintf(buff, 32, "%d", recycle.sum_recycled);
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();

	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("Calls rcvd");
	snprintf(buff, 32, "%d", recycle.count_received);
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();

	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("..recycled");
	snprintf(buff, 32, "%d", recycle.count_recycled);
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();

	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("Weight");
	snprintf(buff, 32, "%g", recycle.weight_kg);
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0.0");
	bx_label_[ix]->hide();
	index_weight_ = ix;

	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		op_value_[ix]->label("");
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// DISPOSE_SASE
// Remove envelopes from the SASE box and return or recycle them
// TODO: Add a reason for disposal field
void QBS_window::update_dispose_sase() {
	char buff[32];
	g_process_->label("Discard SASEs");
	// Batch output
	ch_batch_->deactivate();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	update_calls(RCVD_BOX);
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
	op_value_[ix]->show();
	op_value_[ix]->label("SASES");
	snprintf(buff, 32, "%0.0f", data_->get_count(SASE_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_sase_ = ix;

	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(true);
	update_history(true);
	return;
}

// SUMMARY_BATCH
// Allow the selection of a batch to then display the summary
void QBS_window::update_batch_summary() {
	g_process_->label("Display a summary of batch activity");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	update_batches(true);
	// Button action
	bn_b_action_->activate();
	bn_b_action_->label("Summarise");
	// Call input - get first call in box
	ip_call_->deactivate();
	update_calls(selected_box_);
	// Action button
	bn_c_action_->deactivate();
	// Column headers
	bx_current_->deactivate();
	bx_current_->label("");
	bx_change_->deactivate();
	bx_change_->label("");
	// Count inputs IN-BOX
	int ix = 0;
	for (; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// LIST_BATCH
// Allow the selection of a batch to then display the listing
void QBS_window::update_batch_listing() {
	g_process_->label("Display a list of all activity for batch");
	// Batch output
	ch_batch_->activate();
	selected_box_ = data_->get_current();
	update_batches(true);
	// Button action
	bn_b_action_->activate();
	bn_b_action_->label("List");
	// Call input - get first call in box
	ip_call_->deactivate();
	update_calls(selected_box_);
	// Action button
	bn_c_action_->deactivate();
	// Column headers
	bx_current_->deactivate();
	bx_current_->label("");
	bx_change_->deactivate();
	bx_change_->label("");
	// Count inputs IN-BOX
	int ix = 0;
	for (; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// CALL_SUMMARY
// Display a summary of cards currently held for this call
void QBS_window::update_call_summary() {
	g_process_->label("Report a summary of currently held cards for this call");
	// Batch output
	ch_batch_->deactivate();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	bn_b_action_->label("");
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	update_calls(RCVD_BOX);
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Summarise");
	// Column headers
	bx_current_->activate();
	bx_current_->label("");
	bx_change_->activate();
	bx_change_->label("");
	// Count inputs IN-BOX
	int ix = 0;
	for (; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// CALL_HISTORY
// Display a list of all card transactions for this call
void QBS_window::update_call_history() {
	g_process_->label("Report a history of all cards processed for this call");
	// Batch output
	ch_batch_->deactivate();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	bn_b_action_->label("");
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	update_calls(RCVD_BOX);
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("History");
	// Column headers
	bx_current_->activate();
	bx_current_->label("");
	bx_change_->activate();
	bx_change_->label("");
	// Count inputs IN-BOX
	int ix = 0;
	for (; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(false);
	update_history(false);
	return;
}

// Show the note dialog
void QBS_window::update_edit_notes() {
	g_process_->label("Add a note for the selected call");
	// Batch output
	ch_batch_->deactivate();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	bn_b_action_->label("");
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	update_calls(RCVD_BOX);
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Add note");
	// Column headers
	bx_current_->activate();
	bx_current_->label("");
	bx_change_->activate();
	bx_change_->label("");
	// Count inputs IN-BOX
	for (int ix = 0; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Show the note widgets
	tab_old_notes_->show();
	tab_old_notes_->set_data(data_->get_notes(call_));
	op_note_date_->show();
	op_note_date_->value(now(true, DATE_FORMAT).c_str());
	ip_note_name_->show();
	ip_note_name_->value("");
	ip_note_value_->show();
	ip_note_value_->value("");
	tab_old_notes_->show();
	tab_old_notes_->set_data(data_->get_notes(call_));
	update_history(false);
	return;

}

// CORRECT_DATA
// Any cards in kept boxes (current and those in the disposal queue) to
// be put into envelopes and left in the out box.
void QBS_window::update_correct_data() {
	char buff[32];
	g_process_->label("Correct card counts in active boxes");
	// Batch output
	ch_batch_->activate();
	update_batches(false);
	// Button action
	bn_b_action_->deactivate();
	// Call input - get first call in box
	ip_call_->activate();
	ip_call_->value(call_.c_str());
	update_calls(selected_box_);
	// Action button
	bn_c_action_->activate();
	bn_c_action_->label("Adjust cards");
	// Column headers
	bx_current_->activate();
	bx_current_->label("Existing");
	bx_change_->activate();
	bx_change_->label("Delta");
	// Count inputs KEEP
	int ix = 0;
	op_value_[ix]->show();
	op_value_[ix]->label("KEEP");
	snprintf(buff, 32, "%0.0f", data_->get_count(KEEP_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_keep_ = ix;
	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("IN-BOX");
	snprintf(buff, 32, "%0.0f", data_->get_count(IN_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	bx_label_[ix]->hide();
	index_inbox_ = ix;
	// Now output all the bins
	int curr = data_->get_current();
	int tail = data_->get_tail();
	int head = data_->get_head();
	int box = curr;
	ix++;
	while (box >= head) {
		// Display the current and disposal queue boxes (by batch id)
		op_value_[ix]->show();
		if (box == curr) {
			op_value_[ix]->label("CURRENT");
		}
		else if (box == head) {
			op_value_[ix]->label("HEAD");
		}
		else if (box == tail) {
			op_value_[ix]->label("TAIL");
		}
		else {
			op_value_[ix]->label("");
		}
		snprintf(buff, 32, "%0.0f", data_->get_count(box, call_));
		op_value_[ix]->value(buff);
		ip_delta_[ix]->show();
		ip_delta_[ix]->value("0");
		bx_label_[ix]->show();
		bx_label_[ix]->copy_label(data_->get_batch(box).c_str());
		if (box == curr) index_curr_ = ix;
		if (box == head) index_head_ = ix;
		box--;
		ix++;
	}
	op_value_[ix]->show();
	op_value_[ix]->label("SASEs");
	snprintf(buff, 32, "%0.0f", data_->get_count(SASE_BOX, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->show();
	ip_delta_[ix]->value("0");
	index_sase_ = ix;
	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("AVERAGE");
	snprintf(buff, 32, "%0.1f", data_->get_count(RCVD_AVE, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();
	ix++;
	op_value_[ix]->show();
	op_value_[ix]->label("LAST 4");
	snprintf(buff, 32, "%0.1f", data_->get_count(LAST4_AVE, call_));
	op_value_[ix]->value(buff);
	ip_delta_[ix]->hide();
	bx_label_[ix]->hide();
	// Deactivate the rest
	for (ix++; ix < NUM_COUNTS; ix++) {
		op_value_[ix]->hide();
		ip_delta_[ix]->hide();
		bx_label_[ix]->hide();
	}
	// Hide the note widgets
	hide_edit_notes(true);
	update_history(true);
	return;
}


// Hide the note dialog
void QBS_window::hide_edit_notes(bool info) {
	if (info) {
		tab_old_notes_->show();
		tab_old_notes_->set_data(data_->get_notes(call_));
		op_note_date_->hide();
		ip_note_name_->hide();
		ip_note_value_->hide();
	}
	else {
		tab_old_notes_->hide();
		op_note_date_->hide();
		ip_note_name_->hide();
		ip_note_value_->hide();
	}
}

// Update the history
void QBS_window::update_history(bool enable) {
	if (enable) {
		g_charts_->show();
		g_charts_->update(call_);
	}
	else {
		g_charts_->hide();
	}
}

// Depending on the current processin phase allow/disallow the command
// buttons;
void QBS_window::update_actions() {
	process_mode_t mode = data_->mode();
	selected_box_ = data_->get_current();
	switch (mode) {
	case INITIAL:
		// Not yet processing cards
		g_process_->deactivate();
		// Count inputs IN-BOX
		for (int ix = 0; ix < NUM_COUNTS; ix++) {
			op_value_[ix]->hide();
			ip_delta_[ix]->hide();
		}
		bx_change_->hide();
		bx_current_->hide();
		hide_edit_notes(false);
		update_history(false);

		g_input_->activate();
		break;
	case DORMANT:
		// Not in an active sorting phase - omly allow command for
		// receiving cards or envelopes
		g_process_->activate();
		g_input_->deactivate();
		ch_batch_->deactivate();
		update_batches(false);
		bn_b_action_->deactivate();
		ip_call_->deactivate();
		update_calls(RCVD_BOX);
		bn_c_action_->deactivate();
		bn_use_defaults_->deactivate();
		bx_change_->hide();
		bx_current_->hide();
		bn_new_batch_->activate();
		bn_sort_cards_->deactivate();
		bn_rcv_card_->activate();
		bn_rcv_sase_->activate();
		bn_stuff_cards_->deactivate();
		bn_dispose_cards_->deactivate();
		bn_post_cards_->deactivate();
		bn_recycle_cards_->activate();
		bn_edit_notes_->activate();
		bn_dispose_cards_->activate();
		bn_summ_batch_->activate();
		bn_summ_call_->activate();
		bn_hist_call_->activate();
		break;
	case ACTIVE:
		// In an active sorting phase - allow all commands
		// except that to start a new batch (must finish this one first!)
		g_process_->activate();
		g_input_->deactivate();
		ch_batch_->deactivate();
		update_batches(false);
		ip_call_->deactivate();
		update_calls(RCVD_BOX);
		bx_change_->show();
		bx_current_->show();
		bn_use_defaults_->activate();
		bn_new_batch_->deactivate();
		bn_sort_cards_->activate();
		bn_rcv_card_->activate();
		bn_rcv_sase_->activate();
		bn_stuff_cards_->activate();
		bn_dispose_cards_->activate();
		bn_post_cards_->activate();
		bn_recycle_cards_->activate();
		bn_edit_notes_->activate();
		bn_dispose_cards_->activate();
		bn_summ_batch_->activate();
		bn_summ_call_->activate();
		bn_hist_call_->activate();
		break;
	}
	update_action_values();
	// Action any redrawing
	return;
}

void QBS_window::update_action_bn(Fl_Button* b, action_t a) {
	b->value(action_ == a);
};

// Update action values
void QBS_window::update_action_values() {
	update_action_bn(bn_new_batch_, NEW_BATCH);
	update_action_bn(bn_sort_cards_, SORT_CARDS);
	update_action_bn(bn_rcv_card_, RECEIVE_CARD);
	update_action_bn(bn_rcv_sase_, RECEIVE_SASE);
	update_action_bn(bn_stuff_cards_, STUFF_CARDS);
	update_action_bn(bn_keep_cards_, KEEP_CARDS);
	update_action_bn(bn_dispose_cards_, DISPOSE_CARDS);
	update_action_bn(bn_post_cards_, POST_CARDS);
	update_action_bn(bn_recycle_cards_, RECYCLE_CARDS);
	update_action_bn(bn_correction_, CORRECT_DATA);
	update_action_bn(bn_edit_notes_, EDIT_NOTES);
	update_action_bn(bn_summ_batch_, SUMMARY_BATCH);
	update_action_bn(bn_summ_call_, SUMMARY_CALL);
	update_action_bn(bn_hist_call_, HISTORY_CALL);
}

// Update batches
void QBS_window::update_batches(bool enable_nav) {
	if (ch_batch_->active()) {
		populate_batch(enable_nav);
		ch_batch_->value(selected_box_);
	}
}

// Update calls
void QBS_window::update_calls(int box_num) {
	if (ip_call_->active()) {
		ip_call_->value(call_.c_str());
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
	return;
}

// Call the appropriate update_... for the current action
void QBS_window::update_whatever() {
	switch (action_) {
	case NEW_BATCH: update_new_batch(); break;
	case SORT_CARDS: update_sort_cards(); break;
	case RECEIVE_CARD: update_rcv_card(); break;
	case RECEIVE_SASE: update_rcv_sase(); break;
	case STUFF_CARDS: update_stuff_cards(); break;
	case KEEP_CARDS: update_keep_cards(); break;
	case DISPOSE_CARDS: update_dispose_cards(); break;
	case DISPOSE_SASE: update_dispose_sase(); break;
	case POST_CARDS: update_post_cards(); break;
	case RECYCLE_CARDS: update_recycle_cards(); break;
	case EDIT_NOTES: update_edit_notes(); break;
	case CORRECT_DATA: update_correct_data(); break;
	}
}

// Callback - import CSV data - pass the current directory to the data to 
// start redaing it
void QBS_window::cb_import(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	that->update_import(0);
	that->data_->import_cvs(that->csv_directory_);
	that->update_import(1);
	return;
}

// Callback - read QBS data - pass filename to data to start reading it
void QBS_window::cb_read_qbs(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	that->update_qbs(false);
	if (that->data_->read_qbs(that->qbs_filename_)) {
		that->update_qbs(true);
		that->action_ = that->data_->get_action();
		that->update_actions();
		that->update_whatever();
	}
	return;
}

// Callback - process selection button pressed, select the appropriare
// command
void QBS_window::cb_process(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	// Get radio value - to action_
	cb_radio(w, v);
	// depending on action_ value configure the widgets accordingly
	that->batch_op_done_ = false;
	switch (that->action_) {
	case NEW_BATCH:
		that->update_new_batch();
		that->data_->mode(ACTIVE);
		break;
	case SORT_CARDS:
		that->update_sort_cards();
		break;
	case RECEIVE_CARD:
		that->update_rcv_card();
		break;
	case RECEIVE_SASE:
		that->update_rcv_sase();
		break;
	case STUFF_CARDS:
		that->update_stuff_cards();
		break;
	case KEEP_CARDS:
		that->update_keep_cards();
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
	case SUMMARY_BATCH:
		that->call_ = "";
		that->update_batch_summary();
		break;
	case LIST_BATCH:
		that->update_batch_listing();
		break;
	case SUMMARY_CALL:
		that->update_call_summary();
		break;
	case HISTORY_CALL:
		that->update_call_history();
		break;
	case EDIT_NOTES:
		that->update_edit_notes();
		break;
	case CORRECT_DATA:
		that->update_correct_data();
		break;
	}
	return;
}

// Callback - execute batch
// RECEIVE_BATCH: create new batch
// POST_CARDS: change box status
// DISPOSE_CARDS: ditto
// RECYCLE_CARDS: ditto
// SUMMARY_BATCH: open wndow to display a summary of the batch
void QBS_window::cb_exec_batch(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	string date = now(true, DATE_FORMAT);
	that->selected_box_ = that->ch_batch_->value();
	string batch = that->data_->get_batch(that->selected_box_);
	char log_msg[256];
	switch (that->action_) {
	case NEW_BATCH:
		that->data_->new_batch(that->selected_box_, date, batch);
		that->batch_op_done_ = true;
		snprintf(log_msg, 256, "Closing batch %s\n", date.c_str());
		that->append_batch_log(log_msg);
		that->open_batch_log(batch);
		snprintf(log_msg, 256, "New batch %s: %s\n", batch.c_str(), date.c_str());
		that->append_batch_log(log_msg);
		break;
	case POST_CARDS:
		that->data_->post_cards(date);
		snprintf(log_msg, 256, "Posting batch: %s\n", date.c_str());
		that->append_batch_log(log_msg);
		that->batch_op_done_ = true;
		break;
	case DISPOSE_CARDS:
		that->data_->dispose_cards(date);
		snprintf(log_msg, 256, "Marking batch for disposal: %s\n", date.c_str());
		that->append_batch_log(log_msg);
		that->batch_op_done_ = true;
		break;
	case RECYCLE_CARDS: {
		float weight = (float)atof(that->ip_delta_[that->index_weight_]->value());
		that->data_->recycle_cards(date, weight);
		snprintf(log_msg, 256, "Recycling batch: %g kg %s\n", weight, date.c_str());
		that->append_batch_log(log_msg);
		that->batch_op_done_ = true;
		break;
	}
	case SUMMARY_BATCH:
		that->data_->display_batch_summary(that->selected_box_);
		that->batch_op_done_ = false;
		break;
	case LIST_BATCH:
		that->data_->display_batch_listing(that->selected_box_);
		that->batch_op_done_ = false;
		break;
	}
	if (that->batch_op_done_)	w->deactivate();
	that->update_action_values();
	return;
}

// Callback - execute call
// SORT_CARDS: Add inputs from in-box and received cards to current box
// RECEIVE_CARD: Add received cards to in-box
// RECEIVE_SASE: Add received envelopes to SASE box
// DISPOSE_SASE: Remove envelopes from SASE box
// STUFF_CARDS: Move envelopes from all identified boxes to out-box
// SUMMARY_CALL: Provide a summary of all cards and sases held for a call
// HISTORY_CALL: List all transactions for the callsign
void QBS_window::cb_exec_call(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	string date = now(true, DATE_FORMAT);
	string call = that->call_;
	int num_cards = 0;
	int num_sases = 0;
	int sum_cards = 0;
	char log_msg[256];
	switch (that->action_) {
	case SORT_CARDS:
		// Take the number of cards, add in the in and keep boxes 
		num_cards = atoi(that->ip_delta_[that->index_curr_]->value());
		that->data_->receive_cards(that->selected_box_, date, call, num_cards);
		snprintf(log_msg, 256, "Received batch cards %s: %s %d\n", date.c_str(), call.c_str(), num_cards);
		that->append_batch_log(log_msg);
		that->update_sort_cards();
		break;
	case RECEIVE_CARD:
		num_cards = atoi(that->ip_delta_[that->index_inbox_]->value());
		that->data_->receive_cards(IN_BOX, date, call, num_cards);
		snprintf(log_msg, 256, "Received non-batch cards %s: %s %d\n", date.c_str(), call.c_str(), num_cards);
		that->append_batch_log(log_msg);
		that->update_rcv_card();
		break;
	case RECEIVE_SASE:
		num_sases = atoi(that->ip_delta_[that->index_sase_]->value());
		that->data_->receive_sases(date, call, num_sases);
		snprintf(log_msg, 256, "Received SASEs %s: %s %d\n", date.c_str(), call.c_str(), num_sases);
		that->append_batch_log(log_msg);
		that->update_rcv_sase();
		break;
	case DISPOSE_SASE:
		num_sases = atoi(that->ip_delta_[that->index_sase_]->value());
		that->data_->receive_sases(date, call, -(num_sases));
		snprintf(log_msg, 256, "Disposed %s: %s %d\n", date.c_str(), call.c_str(), num_sases);
		that->append_batch_log(log_msg);
		that->update_dispose_sase();
		break;
	case STUFF_CARDS: {
		// Send those in keep
		num_cards = atoi(that->ip_delta_[that->index_keep_]->value());
		sum_cards += num_cards;
		that->data_->stuff_cards(KEEP_BOX, date, call, num_cards);
		num_cards = atoi(that->ip_delta_[that->index_inbox_]->value());
		sum_cards += num_cards;
		that->data_->stuff_cards(IN_BOX, date, call, num_cards);
		// From HEAD to CURR - send cards
		int curr = that->data_->get_current();
		int head = that->data_->get_head();
		int box = head;
		int ix = that->index_head_;
		while (ix >= that->index_curr_ && box <= curr) {
			num_cards = atoi(that->ip_delta_[ix]->value());
			sum_cards += num_cards;
			that->data_->stuff_cards(box, date, call, num_cards);
			ix--;
			if (box < curr) box++;
		}
		// Use SASEs
		num_sases = atoi(that->ip_delta_[that->index_sase_]->value());
		that->data_->use_sases(date, call, num_sases);
		snprintf(log_msg, 256, "Stuffed cards %s: %s %d cards in %d SASEs\n", date.c_str(), call.c_str(), sum_cards, num_sases);
		that->append_batch_log(log_msg);
		that->update_stuff_cards();
		break;
	}
	case KEEP_CARDS: {
		// Move cards to keep box
		num_cards = atoi(that->ip_delta_[that->index_keep_]->value());
		that->data_->keep_cards(that->data_->get_current(), date, call, num_cards);
		snprintf(log_msg, 256, "Kept cards %s: %s %d cards\n", date.c_str(), call.c_str(), num_cards);
		that->append_batch_log(log_msg);
		that->update_keep_cards();
		break;
	}
	case SUMMARY_CALL:
		that->data_->display_call_summary(call);
		break;
	case HISTORY_CALL:
		that->data_->display_call_history(call);
		break;
	case EDIT_NOTES: {
		string date = that->op_note_date_->value();
		string name = that->ip_note_name_->value();
		string value = that->ip_note_value_->value();
		that->data_->ham_data(date, call, name, value);
		snprintf(log_msg, 256, "Added note %s: %s %s %s\n", date.c_str(), call.c_str(), name.c_str(),value.c_str());
		that->append_batch_log(log_msg);
		that->update_edit_notes();
		break;
	}
	case CORRECT_DATA: {
		// Send those in keep
		num_cards = atoi(that->ip_delta_[that->index_keep_]->value());
		if (num_cards != 0) {
			that->data_->adjust_cards(KEEP_BOX, date, call, num_cards);
			snprintf(log_msg, 256, "Correction %s: %s %d cards added to KEEP_BOX\n", date.c_str(), call.c_str(), num_cards);
			that->append_batch_log(log_msg);
		}
		num_cards = atoi(that->ip_delta_[that->index_inbox_]->value());
		if (num_cards != 0) {
			that->data_->adjust_cards(IN_BOX, date, call, num_cards);
			snprintf(log_msg, 256, "Correction %s: %s %d cards added to IN_BOX\n", date.c_str(), call.c_str(), num_cards);
			that->append_batch_log(log_msg);
		}
		// From HEAD to CURR - send cards
		int curr = that->data_->get_current();
		int head = that->data_->get_head();
		int box = head;
		int ix = that->index_head_;
		while (ix >= that->index_curr_ && box <= curr) {
			num_cards = atoi(that->ip_delta_[ix]->value());
			if (num_cards != 0) {
				that->data_->adjust_cards(box, date, call, num_cards);
				snprintf(log_msg, 256, "Correction %s: %s %d cards added to box %d\n", date.c_str(), call.c_str(), num_cards, box);
				that->append_batch_log(log_msg);
			}
			ix--;
			if (box < curr) box++;
		}
		// Use SASEs
		num_sases = atoi(that->ip_delta_[that->index_sase_]->value());
		if (num_sases != 0) {
			that->data_->adjust_cards(SASE_BOX, date, call, num_sases);
			snprintf(log_msg, 256, "Correction %s: %s %d SASEs added\n", date.c_str(), call.c_str(), num_sases);
			that->append_batch_log(log_msg);
		}
		that->update_correct_data();
		break;
	}

	}
}

// Call back - call input
// TODO: Check call exists etc
void QBS_window::cb_input_call(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	cb_value<Fl_Input, string>(w, v);
	string* call = (string*)v;
	*call = to_upper(*call);
	switch (that->action_) {
	case SORT_CARDS:
		that->update_sort_cards();
		break;
	case RECEIVE_CARD:
		that->update_rcv_card();
		break;
	case RECEIVE_SASE:
		that->update_rcv_sase();
		break;
	case DISPOSE_SASE:
		that->update_dispose_sase();
		break;
	case STUFF_CARDS:
		that->update_stuff_cards();
		break;
	case KEEP_CARDS:
		that->update_keep_cards();
		break;
	case EDIT_NOTES:
		that->update_edit_notes();
		break;
	case CORRECT_DATA:
		that->update_correct_data();
		break;
	}
}

// Return applicaton to reset state
void QBS_window::cb_reset(Fl_Widget* w, void* v) {
	clog << "+++++++++++++++++++++++ RESET ++++++++++++++++++++++++" << endl;
	QBS_window* that = ancestor_view<QBS_window>(w);
	delete that->data_;
	that->data_ = new QBS_data;
	// Now allow the data to be processed
	that->data_->set_window(that);
}

// Enter typed when in input value acts as if call exec button pressed
void QBS_window::cb_ip_enter(Fl_Widget* w, void* v) {
	QBS_window* that = ancestor_view<QBS_window>(w);
	cb_exec_call(that->bn_c_action_, v);
}

// Open batch log
void QBS_window::open_batch_log(string batch_name) {
	if (blog_file_ != nullptr) {
		blog_file_->close();
		delete blog_file_;
	}
	string name = string(ip_csv_dir_->value()) + "/" + batch_name + ".log";
	cout << "Opening " << name << endl;
	blog_file_ = new ofstream(name.c_str(), ios::out | ios::app);
}

// Append batch log - both to putput file and log display
void QBS_window::append_batch_log(const char* text) {
	*blog_file_ << text;
	td_log_->buffer()->append(text);
	td_log_->scroll(pos_batch_log_, 0);
	pos_batch_log_++;
}

// Populate batch choice
void QBS_window::populate_batch(bool enable_change) {
	ch_batch_->clear();
	for (int b = 0; b <= data_->get_current(); b++) {
		ch_batch_->add(data_->get_batch(b).c_str());
		if (!enable_change && b != selected_box_) {
			ch_batch_->mode(b, FL_MENU_INACTIVE);
		}
	}
}