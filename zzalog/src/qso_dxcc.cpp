#include "qso_dxcc.h"
#include "qso_entry.h"
#include "pfx_data.h"
#include "book.h"
#include "drawing.h"
#include "utils.h"

extern pfx_data* pfx_data_;
extern book* book_;

qso_dxcc::qso_dxcc(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L) ,
	prefixes_(nullptr),
	source_(NONE)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	create_form();
	enable_widgets();
}

qso_dxcc::~qso_dxcc() {

}

void qso_dxcc::create_form() {
	int avail_width = w() - GAP - GAP;
	int avail_height = h() - GAP;
	int curr_x = x() + GAP;
	int curr_y = y() + 1;

	// "Title" is callsign
	op_call_ = new Fl_Output(curr_x, curr_y, avail_width, HBUTTON);
	op_call_->box(FL_FLAT_BOX);
	op_call_->color(FL_BACKGROUND_COLOR);
	op_call_->textfont(FL_BOLD);
	op_call_->textsize(FL_NORMAL_SIZE + 2);
	op_call_->textcolor(COLOUR_CLARET);

	curr_y += op_call_->h() + HTEXT;
	tree_ = new tree(curr_x, curr_y, avail_width, avail_height, "Possible prefixes");
	tree_->align(FL_ALIGN_TOP);
	tree_->callback(cb_tree);
	tree_->when(FL_WHEN_NEVER);

	const int min_tree_h = (3 * ROW_HEIGHT) + GAP;
	curr_y += min_tree_h;
	avail_height -= min_tree_h;

	g_wb4_ = new wb4_buttons(curr_x, curr_y, avail_width, avail_height, "DXCC worked before");
	g_wb4_->align(FL_ALIGN_TOP);

	end();
	enable_widgets();

}

// Enable the widgets
void qso_dxcc::enable_widgets() {
	op_call_->value(callsign_.c_str());
	tree_->enable_widgets();
	if (prefixes_ == nullptr || prefixes_->size() == 0) {
		// Only display tree with suitable notice
		tree_->size(tree_->w(), 5 * ROW_HEIGHT);
		tree_->when(FL_WHEN_NEVER);
		g_wb4_->hide();
	}
	else if (prefixes_->size() == 1) {
		// Show single prefix and display worked before lights
		tree_->size(tree_->w(), 5 * ROW_HEIGHT);
		tree_->when(FL_WHEN_NEVER);
		g_wb4_->show();
		g_wb4_->position(g_wb4_->x(), tree_->y() + tree_->h() + HTEXT);
		g_wb4_->size(g_wb4_->w(), h() + y() - GAP - g_wb4_->y());
		g_wb4_->enable_widgets();
	}
	else {
		tree_->size(tree_->w(), h() + y() - GAP - tree_->y());
		tree_->when(FL_WHEN_RELEASE);
		g_wb4_->hide();
	}
}

// Set the data - parse the callsign in the current qso
void qso_dxcc::set_data() {
	qso_entry* qe = ancestor_view<qso_entry>(this);
	record* qso = qe->qso();
	callsign_ = "";
	delete(prefixes_);
	bands_worked_ = nullptr;
	modes_worked_ = nullptr;
	prefixes_ = new vector<prefix*>;
	source_ = NONE;
	if (qso) {
		callsign_ = qso->item("CALL");
		// Is a prefix supplied
		string nickname = qso->item("APP_ZZA_PFX");
		if (nickname.length()) {
			prefixes_->push_back(pfx_data_->get_prefix(nickname));
			source_ = RECORD_PFX;
		}
		else {
			// Is the DXCC supplied
			int dxcc = -1;
			if (qso->item("DXCC").length())	qso->item("DXCC", dxcc);
			if (dxcc >= 0) {
				prefixes_->push_back(pfx_data_->get_prefix(dxcc));
				source_ = RECORD_DXCC;
			}
			else {
				// Check execptions
				exc_entry* exception = pfx_data_->get_exceptions()->is_exception(qso);
				if (exception) {
					dxcc = exception->adif_id;
					qso->item("DXCC", to_string(dxcc));
					prefixes_->push_back(pfx_data_->get_prefix(dxcc));
					source_ = EXCEPTION;
				}
				else {
					// Get all prefixes that could provide this callsign
					pfx_data_->all_prefixes(qso, prefixes_, false);
					source_ = PREFIX_LIST;
				}
			}
		}
		// If only one that get the bands/modes worked for that one
		if (prefixes_->size() == 1) {
			// Get the root prefix
			prefix* parent = (*prefixes_)[0];
			while (parent->parent_) parent = parent->parent_;
			int dxcc = parent->dxcc_code_;
			bands_worked_ = book_->used_bands(dxcc);
			modes_worked_ = book_->used_submodes(dxcc);
		}
	}
	tree_->set_data(prefixes_);
	g_wb4_->set_data(bands_worked_, modes_worked_);

}

void qso_dxcc::cb_tree(Fl_Widget* w, void* v) {
	qso_dxcc* that = ancestor_view<qso_dxcc>(w);
	tree* that_tree = (tree*)w;
	Fl_Tree_Item* item = that_tree->callback_item();
	prefix* pfx = (prefix*)item->user_data();
	qso_entry* qe = ancestor_view<qso_entry>(that);
	record* qso = qe->qso();
	if (qso) {
		qso->item("DXCC", pfx->dxcc_code_);
		qso->item("STATE", pfx->state_);
		qso->item("APP_ZZA_PFX", pfx->nickname_);
	}
	qe->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
}

qso_dxcc::source_t qso_dxcc::source() {
	return source_;
}

qso_dxcc::tree::tree(int X, int Y, int W, int H, const char* L) :
	Fl_Tree(X, Y, W, H, L),
	prefixes_(nullptr)
{
	clear();
}

qso_dxcc::tree::~tree() {}

// Draw the tree bottom up
void qso_dxcc::tree::enable_widgets() {
	// Define the root item
	Fl_Tree_Item* root_item = new Fl_Tree_Item(this);
	root(root_item);
	root_item->labelfont(item_labelfont() | FL_BOLD);
	root_item->labelcolor(FL_BLACK);
	char root_label[128];
	source_t source = ancestor_view<qso_dxcc>(this)->source();
	switch (source) {
	case EXCEPTION:
		strcpy(root_label, "Found in exception file.");
		break;
	case PREFIX_LIST:
		snprintf(root_label, sizeof(root_label), "%d Prefixes found.", prefixes_->size());
		break;
	case RECORD_DXCC:
		strcpy(root_label, "'DXCC' field in QSO.");
		break;
	case RECORD_PFX:
		strcpy(root_label, "'APP_ZZA_PFX' field in QSO.");
		break;
	}
	root_item->label(root_label);
	if (prefixes_) {
		for (auto it = prefixes_->begin(); it != prefixes_->end(); it++) {
			// Reuse code in pfx_tree
			Fl_Tree_Item* item = hang_item(*it);
		}
	}
}

void qso_dxcc::tree::set_data(vector<prefix*>* data) {
	prefixes_ = data;
}

// Hang the item on the tree where it should go
Fl_Tree_Item* qso_dxcc::tree::hang_item(prefix* pfx) {
	// Existing item or created one
	Fl_Tree_Item* item;
	// Label for the item
	char text[128];
	snprintf(text, sizeof(text), "%s: %s",
		pfx->nickname_.c_str(),
		pfx->name_.c_str());
	// Find where to hang it
	prefix* parent = pfx->parent_;
	if (parent) {
		Fl_Tree_Item* hang_pt = hang_item(parent);
		item = hang_pt->find_child_item(text);
		if (item == nullptr) {
			item = hang_pt->add(prefs(), text);
			item->user_data(pfx);
		} 
	}
	else {
		item = find_item(text);
		if (item == nullptr) {
			item = add(text);
			item->user_data(pfx);
			item->labelfont(FL_BOLD);
		}
	}
	return item;
}


// The "worked before" lights
qso_dxcc::wb4_buttons::wb4_buttons(int X, int Y, int W, int H, const char* L) :
	Fl_Scroll(X, Y, W, H, L)
{
	create_form();
}

qso_dxcc::wb4_buttons::~wb4_buttons() {}

void qso_dxcc::wb4_buttons::create_form() {
	const int NUM_WIDE = 5;
	const int BWIDTH = w() / NUM_WIDE;
	int bn_number = 0;
	clear();
	begin();
	int curr_x = x();
	int curr_y = y();
	set<string>* bands = book_->used_bands();
	for (auto it = bands->begin(); it != bands->end(); it++) {
		Fl_Button* bn = new Fl_Button(curr_x, curr_y, BWIDTH, HBUTTON);
		bn->copy_label((*it).c_str());
		map_[*it] = bn;
		curr_x += BWIDTH;
		bn_number++;
		if (bn_number % NUM_WIDE == 0) {
			curr_x = x();
			curr_y += HBUTTON;
		}
	}
	if (curr_x != x()) {
		curr_x = x();
		curr_y += HBUTTON;
	}
	set<string>* modes = book_->used_submodes();
	bn_number = 0;
	for (auto it = modes->begin(); it != modes->end(); it++) {
		Fl_Button* bn = new Fl_Button(curr_x, curr_y, BWIDTH, HBUTTON);
		bn->copy_label((*it).c_str());
		map_[*it] = bn;
		curr_x += BWIDTH;
		bn_number++;
		if (bn_number % NUM_WIDE == 0) {
			curr_x = x();
			curr_y += HBUTTON;
		}
	}
}

// Light the buttons that have been worked: 
void qso_dxcc::wb4_buttons::enable_widgets() {
	qso_entry* qe = ancestor_view<qso_entry>(this);
	record* qso = qe->qso();
	string qso_band = "";
	string qso_mode = "";
	if (qso) {
		qso_band = qso->item("BAND");
		qso_mode = qso->item("MODE", false, true);
	}
	// Clear all widgets
	for (int ix = 0; ix < children(); ix++) {
		Fl_Button* bn = (Fl_Button*)child(ix);
		bn->color(FL_BACKGROUND_COLOR);
		bn->labelcolor(fl_inactive(FL_BLACK));
	}
	// Set band colours - CLARET this QSO, MAUVE other QSOs
	for (auto ix = bands_->begin(); ix != bands_->end(); ix++) {
		Fl_Button* bn = (Fl_Button*)map_.at(*ix);
		if ((*ix) == qso_band) {
			bn->color(COLOUR_CLARET);
		}
		else {
			bn->color(COLOUR_MAUVE);
		}
		bn->labelcolor(fl_contrast(FL_BLACK, bn->color()));
	}
	// Set mode colours - CALRET this QSO, APPLE other QSOs
	for (auto ix = modes_->begin(); ix != modes_->end(); ix++) {
		Fl_Button* bn = (Fl_Button*)map_.at(*ix);
		if ((*ix) == qso_mode) {
			bn->color(COLOUR_CLARET);
		}
		else {
			bn->color(COLOUR_APPLE);
		}
		bn->labelcolor(fl_contrast(FL_BLACK, bn->color()));
	}
}

void qso_dxcc::wb4_buttons::set_data(set<string>* bands, set<string>* modes) {
	bands_ = bands;
	modes_ = modes;
}
