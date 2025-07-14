#include "intl_dialog.h"
#include "utils.h"
#include "status.h"
#include "callback.h"
#include "menu.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Native_File_Chooser.H>

extern status* status_;
extern menu* menu_;
extern string CONTACT;
extern string COPYRIGHT;
extern string VENDOR;
extern string PROGRAM_ID;
extern string default_data_directory_;

// Majo
string DEFAULT_INTL = "";

// Constructs a window 
intl_dialog::intl_dialog() :
	win_dialog(640, 480, "International character set")
{
	editor_ = nullptr;
	// Get the file name
	string directory = get_path();
	filename_ = directory + "intl_chars.txt";
	// Load the data
	if (load_data()) {
		// create the dialog
		create_form();
	} 
	end();
}

// Create the form
void intl_dialog::create_form() {

	// Delete the existing form and re-build it
	int curr_x = EDGE;
	int curr_y = EDGE;
	clear();
	begin();
	// Button - save the displayed characters into a the non-ASCII character file
	Fl_Button* bn_save = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Save");
	bn_save->callback(cb_bn_save);
	bn_save->tooltip("Save the currently displated characters");
	curr_x += bn_save->w();
	// Button - restore the displayed characters from the file
	Fl_Button* bn_restore = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Restore");
	bn_restore->callback(cb_bn_restore);
	bn_restore->tooltip("Reload the saved character set");
	curr_x += bn_restore->w();
	// Button - add the characters in the adjacent input widget to the displayed characters
	Fl_Button* bn_add = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Add");
	bn_add->callback(cb_bn_add);
	bn_add->tooltip("Add the characters in the adjacent display to the displayed set");
	curr_x += bn_add->w();
	// Input - allows additional characters to be defined.
	Fl_Input* ip_new = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON);
	ip_new->callback(cb_value<Fl_Input, string>, (void*)&new_char_);
	ip_new->tooltip("Paste in desired additional characters");
	// Get the size of the button array
	int width = curr_x + ip_new->w() - EDGE;
	curr_y += ip_new->h() + GAP;
	curr_x = EDGE;
	buttons_ = new Fl_Group(curr_x, curr_y, 10, 10);
	end();
	resizable(nullptr);
	add_buttons(width);
}

// Add buttons with all the wanted characters
void intl_dialog::add_buttons(int width) {
	// Remove existing buttons and start adding again
	buttons_->clear();
	buttons_->begin();
	// Get size of array
	int num_cols = width / HBUTTON;
	int num_rows;
	// Have to do it this way as mixed signed/unsigned arithmetic
	if (symbols_.size()) {
		num_rows = ((symbols_.size() - 1) / num_cols) + 1;
	}
	else {
		num_rows = 0;
	}
	// Max 4 bytes per UTF character plus terminal \0
	char utf8[5];
	int len;
	// position of first button
	int curr_x = buttons_->x();
	int curr_y = buttons_->y();
	auto ucs = symbols_.begin();
	// For each row and column
	for (int R = 0; R < num_rows; R++) {
		for (int C = 0; C < num_cols && ucs != symbols_.end(); C++) {
			// Get the next character from the list 
			len = fl_utf8encode(*ucs, utf8);
			utf8[len] = 0;
			// Button - copy and paste the label on the button to the current editor
			Fl_Button* bn = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON);
			// Fit the label font to approx 70%
			bn->labelsize(HBUTTON * 14 / 20);
			bn->copy_label(utf8);
			bn->callback(cb_bn_use);
			bn->tooltip("Copy and paste this character to the currently open editor");
			curr_x += bn->w();
			ucs++;
		}
		curr_x = EDGE;
		curr_y += HBUTTON;
	}
	int height = curr_y + HBUTTON;

	buttons_->end();
	// Adjust the size of the buttons group
	buttons_->resizable(nullptr);
	buttons_->size(width, height);
	// Adjust the size of  the window to fit
	Fl_Box* b_cr = new Fl_Box(x(), buttons_->y() + buttons_->h(), width + EDGE, FOOT_HEIGHT * 2);
	b_cr->copy_label(string(COPYRIGHT + "     \n" + CONTACT + "     ").c_str());
	b_cr->labelsize(FL_NORMAL_SIZE - 1);
	b_cr->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

	size(buttons_->x() + buttons_->w() + EDGE, buttons_->y() + buttons_->h() + b_cr->h());
}

// Desctructor
intl_dialog::~intl_dialog()
{
	clear();
}

// Handle FL_HIDE and FL_SHOW to get menu to update otself
int intl_dialog::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		menu_->update_windows_items();
		break;
	}

	return win_dialog::handle(event);
}

// Callback - save button - save data to a file
void intl_dialog::cb_bn_save(Fl_Widget* w, void* v) {
	intl_dialog* that = ancestor_view<intl_dialog>(w);
	if (!that->save_data()) {
		status_->misc_status(ST_ERROR, "INTL: Unable to save data");
	}
}

// Callback - restore button - reload data
void intl_dialog::cb_bn_restore(Fl_Widget* w, void* v) {
	intl_dialog* that = ancestor_view<intl_dialog>(w);
	if (!that->load_data()) {
		status_->misc_status(ST_ERROR, "INTL: Unable to load data");
	}
	else {
		that->add_buttons(that->buttons_->w());
	}
}

// Callback - Add characters to the displayed buttons
void intl_dialog::cb_bn_add(Fl_Widget* w, void* v) {
	intl_dialog* that = ancestor_view<intl_dialog>(w);
	// Get the string from the input
	that->add_symbols(that->new_char_);
	that->add_buttons(that->buttons_->w());
}

// Paste the buttons label into the editor widget
void intl_dialog::cb_bn_use(Fl_Widget* w, void* v) {
	const char* utf8 = ((Fl_Button*)w)->label();
	intl_dialog* that = ancestor_view<intl_dialog>(w);
	if (that->editor_) {
		int len = strlen(utf8);
		// Copy to clipboard
		Fl::copy(utf8, len);
		// Paste to currently open editor
		Fl::paste(*that->editor_);
		// Set focus to that widget
		Fl::focus(that->editor_);
	}
}

// Get the data path to the files - returns directory name
string intl_dialog::get_path() {
	return default_data_directory_;
}

// Set the widget to receive the pasted character
void intl_dialog::editor(Fl_Widget* w) {
	editor_ = w;
}

// Returns the editing widget
Fl_Widget* intl_dialog::editor() {
	return editor_;
}

// Add the characters in the input text to the list of characters displayed
void intl_dialog::add_symbols(string text) {
	// Create a copy of the input text and set a pointer to the end
	char* utf8 = new char[text.length() + 1];
	char* end = utf8 + text.length();
	unsigned int ucs;
	int len;
	strcpy(utf8, text.c_str());
	// now scan the string for new symbols
	while (utf8 < end) {
		if (*utf8 & 0x80) {
			// non-ASCII character - get it and how many bytes it is, 
			ucs = fl_utf8decode(utf8, end, &len);
			symbols_.insert(ucs);
			utf8 += len;
		}
		else {
			utf8++;
		}
	}
}

// Load the initial character data
bool intl_dialog::load_data() {
	symbols_.clear();
	ifstream is(filename_.c_str());
	string line;
	if (!is.good()) {
		add_symbols(DEFAULT_INTL);
		return true;
	}
	else {
		while (is.good()) {
			getline(is, line);
			if (is.good()) {
				add_symbols(line);
			}
		}
		if (is.eof()) {
			return true;
		}
		else {
			return false;
		}
	}
}

// Store the current character data
bool intl_dialog::save_data() {
	ofstream os(filename_.c_str());
	int col = 0;
	for (auto it = symbols_.begin(); it != symbols_.end() && os.good(); it++) {
		// Convert the UCS code to the appropriate UTF-8 bytes
		unsigned int ucs = *it;
		char utf8[5];
		int len = fl_utf8encode(ucs, utf8);
		for (int i = 0; i < len; i++) {
			os << utf8[i];
			col++;
		}
		// If we hit an arbitrary break point add a new line
		if (col > 40) {
			os << '\n';
			col = 0;
		}
	}
	bool ok = os.good();
	os.close();
	return ok;
}