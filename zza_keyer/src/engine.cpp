#include "engine.h"
#include "key_handler.h"

#include <FL/fl_ask.H>

// Time keeping is in ms. Define a minute as seconds
const milliseconds MINUTE(60000);
// Number of dit times in the word "PARIS" - ".__. ._ ._. .. ...   "
const int DITS_PER_WORD = 50;
// Look up table from Unicode character to morse string
const map<unsigned int, const char*> LUT_MORSE = {
	{ 'A', ".- " },
	{ 'B', "-... " },
	{ 'C', "-.-. " },
	{ 'D', "-.. " },
	{ 'E', ". " },
	{ 'F', "..-. " },
	{ 'G', "--. " },
	{ 'H', ".... " },
	{ 'I', ".." },
	{ 'J', ".--- " },
	{ 'K', "-.- " },
	{ 'L', ".-.." },
	{ 'M', "-- " },
	{ 'N', "-. " },
	{ 'O', "--- " },
	{ 'P', ".--. " },
	{ 'Q', "--.- " },
	{ 'R', ".-. " },
	{ 'S', "... " },
	{ 'T', "- " },
	{ 'U', "..- " },
	{ 'V', "..._ " },
	{ 'W', ".-- " },
	{ 'X', "-..- " },
	{ 'Y', "-.-- " },
	{ 'Z', "--.. " },
	{ '0', "----- " },
	{ '1', ".---- " },
	{ '2', "..--- " },
	{ '3', "...-- " },
	{ '4', "....- " },
	{ '5', "..... " },
	{ '6', "-.... " },
	{ '7', "--... " },
	{ '8', "---.. " },
	{ '9', "----. " },
	{ ' ', "  " },
	{ '&', ".-... " },
	{ '+', ".-.-. " },
	{ '=', "-...- " },
	{ '/', "-..-. " },
	{ '(', "-.--. " },
	{ '?', "..--.. "},
	{ '_', "..--.- "},
	{ '\"', ".-..-. "},
	{ '.', ".-.-.- " },
	{ '@', ".--.-. " },
	{ ';', "-.-.-. " },
	{ '!', "-.--.. " },
	{ ')', "-.---. " },
	{ ',', "--..-- " },
	{ ':', "---... " },
	// TODO - add accented characters
	{ 0xdc, "..-- " }, // U-umlaut
	{ 0xc4, ".-.- " }, // A-umlaut
	{ 0xd6, "---. " }, // O-umlaut
	{ 0xc9, "-.-.. "}, // E-acute
};

extern key_handler* key_handler_;


engine::engine() :
	t_engine_(nullptr),
	close_(false)
{
	memset(current_kb_, 0, sizeof(current_kb_));
	next_sign_ = &current_kb_[0];
}

engine::~engine() {
	// Connect to thread and close it
	close_ = true;
	if (t_engine_) {
		t_engine_->join();
	}
}

// Set the type - if in a state to do so
void engine::type(engine_type t) {
	if (idle()) {
		type_ = t;
	}
	else {
		fl_message("KEYER - cannot change the type of keyer - still active");
	}
}

// Get the type
engine_type engine::type() {
	return type_;
}

// Return idle state
bool engine::idle() {
	return (state_ == IDLE);
}

// Return the keyout state
bool engine::key_out() {
	return key_out_;
}

// Set kb character to enqueue - allows non-ASCII to be sent
bool engine::send(unsigned int ch) {
	q_character_.push(ch);
}

// Set speed
void engine::set_speed(
	double wpm,        // Words per minute
	double weighting   // Dash to dot ratio
) {
	wpm_ = wpm;
	weighting_ = weighting;
	// duration values do not have * or / operators just *= and /=
	dit_time_ = MINUTE;
	dit_time_ /= (wpm * DITS_PER_WORD);
	dash_time_ = dit_time_;
	dash_time_ *= weighting_;
	space_time_ = dit_time_;
	space_time_ *= 2;
}

// Start processing
bool engine::start() {
	close_ = false;
	t_engine_ = new thread(run_engine);
}

// Return the current state of the input paddle/key/keyboard
bool engine::get_signs(bool& dit, bool& dash) {
	if (type_ == KEYBOARD) {
		// Get the next sign (dit or dash) to send
		if (*next_sign_ == 0) {
			// We do not have one - use the next in the character queue
			if (q_character_.empty()) {
				// None waiting
				dit = false;
				dash = false;
				return;
			}
			else {
				// Get the next in the queue
				unsigned int unichar = q_character_.front();
				q_character_.pop();
				if (LUT_MORSE.find(unichar) == LUT_MORSE.end()) {
					fl_message("The Unicode character U+%0x cannot be represented as morse", unichar);
					unichar = '?';
				}
				memset(current_kb_, 0, sizeof(current_kb_));
				memcpy(current_kb_, LUT_MORSE.at(unichar), strlen(LUT_MORSE.at(unichar)));
				next_sign_ = &current_kb_[0];
			}
		}
		switch (*next_sign_) {
		case '-':
			dit = false;
			dash = true;
			break;
		case '.':
			dit = true;
			dash = false;
			break;
		default:
			dit = false;
			dash = false;
			break;
		}
	}
	else {
		key_state ks = key_handler_->get_state();
		dit = (ks == LEFT) || (ks == BOTH);
		dash = (ks == RIGHT) || (ks == BOTH);
	}
}

// Main state machine
engine::state_t engine::next_state(state_t state, bool dit, bool dash) {
	state_t next = IDLE;
	milliseconds gap(milliseconds(0));
	high_resolution_clock::time_point now_time(high_resolution_clock::now());
	bool time_out = (next_edge_ <= now_time);
	switch (state) {
	case IDLE:           // Idle - nothing to process
	{
		switch (type_) {
		case KEYBOARD: {
			// dit and dash are exclusive
			if (dit) {
				next = KB_DIT_DOWN;
				gap = dit_time_;
			}
			else if (dash) {
				next = KB_DASH_DOWN;
				gap = dash_time_;
			}
			break;
		}
		case STRAIGHT: {
			if (dit) {
				next = UT_DIT_DOWN;
			}
			break;
		}
		case SQUEEZE_A:
		case SQUEEZE_B:
		case FULL: {
			if (dit) {
				next = A_DIT_DOWN;
				gap = dit_time_;
			}
			else if (dash) {
				next = A_DASH_DOWN;
				gap = dash_time_;
			}
			break;
		}
		case SEMI: {
			if (dit) {
				next = A_DIT_DOWN;
				gap = dit_time_;
			}
			else if (dash) {
				next = UT_DASH_DOWN;
			}
			break;
		}
		}
		break;
	case UT_DIT_DOWN:    // Untimed dit mark,
	{
		if (!dit) {
			next = IDLE;
		}
		break;
	}
	case UT_DASH_DOWN:   // Untimed dash mark,
	{
		if (!dash) {
			next = IDLE;
		}
	}
	case KB_DIT_UP:      // Keyboard dot space 
	{
		if (time_out) {
			if (dit) {
				next = KB_DIT_DOWN;
				gap = dit_time_;
			}
			else if (dash) {
				next = KB_DASH_DOWN;
				gap = dash_time_;
			}
			else {
				next = IDLE;
			}
		}
		break;
	}
	case KB_DIT_DOWN:    // Keyborad dot mark
	{
		if (time_out) {
			next = KB_DIT_UP;
			gap = dit_time_;
		}
		break;
	}
	case KB_DASH_UP:     // Keyboard dash space
		if (time_out) {
			if (dit) {
				next = KB_DIT_DOWN;
				gap = dit_time_;
			}
			else if (dash) {
				next = KB_DASH_DOWN;
				gap = dash_time_;
			}
			else {
				next = IDLE;
			}
		}
		break;
	}
	case KB_DASH_DOWN:   // Keyboard dash mark
	{
		if (time_out) {
			next = KB_DASH_UP;
			gap = dit_time_;
		}
		break;

	}
	case A_DIT_DOWN:     // Timed dit mark
	{
		if (type_ == SQUEEZE_B && dash) {
			next = A_DIT_DOWN_D;
		}
		else {
			if (time_out) {
				next = A_DIT_UP;
				gap = dit_time_;
			}
		}
		break;
	}
	case A_DIT_UP:       // Timed dit space
	{
		if (type_ == FULL && dit) {
			next = A_DIT_DOWN;
			gap = dit_time_;
		}
		else if (dash) {
			next = A_DASH_DOWN;
			gap = dash_time_;
		}
		else {
			next = A_SPACE;
			gap = space_time_;
		}
		break;
	}
	case A_DIT_DOWN_D:   // Timed dit mark - commmited to a dash next
	{
		if (time_out) {
			next = A_DIT_UP_D;
		}
		break;
	}
	case A_DIT_UP_D:     // Timed dit space - committed to a dash
	{
		if (time_out) {
			next = A_DASH_DOWN;
			gap = dash_time_;
		}
		break;
	}
	case A_DASH_DOWN:    // Timed dash mark 
	{
		if (type_ == SQUEEZE_B && dit) {
			next = A_DASH_DOWN_D;
		}
		else {
			if (time_out) {
				next = A_DASH_UP;
				gap = dit_time_;
			}
		}
		break;
	}
	case A_DASH_UP:      // Timed dash space
	{
		if (type_ == FULL && dash) {
			next = A_DASH_DOWN;
			gap = dash_time_;
		}
		else if (dit) {
			next = A_DIT_DOWN;
			gap = dit_time_;
		}
		else {
			next = A_SPACE;
			gap = space_time_;
		}
		break;
	}
	case A_DASH_DOWN_D:  // Timed dash mark - committed to a dit next
	{
		if (time_out) {
			next = A_DASH_UP_D;
		}
		break;
	}
	case A_DASH_UP_D:    // Timed dash space - committed to a dit
	{
		if (time_out) {
			next = A_DIT_DOWN;
			gap = dit_time_;
		}
		break;
	}
	case A_SPACE:        // Timed space space
	{
		if (time_out) {
			next = IDLE;
		}
		break;
	}
	}
	// Set next edge
	if (gap != milliseconds::zero()) {
		next_edge_ = last_edge_;
		next_edge_ += gap;
	}
	last_edge_ = now_time;
	return next;
}

// Drive key-out
void engine::drive_key_out(state_t state) {
	switch (state) {
	case IDLE:           // Idle - nothing to process
	case KB_DIT_UP:      // Keyboard dot space
	case KB_DASH_UP:     // Keyboard dash space
	case A_DIT_UP:       // Timed dit space
	case A_DIT_UP_D:     // Timed dit space - committed to a dash
	case A_DASH_UP:      // Timed dash space
	case A_DASH_UP_D:    // Timed dash space - committed to a dit
	case A_SPACE:        // Timed space space
		key_out_ = false;
		break;
	case UT_DIT_DOWN:    // Untimed dit mark,
	case UT_DASH_DOWN:   // Untimed dash mark,
	case KB_DIT_DOWN:    // Keyborad dot mark
	case KB_DASH_DOWN:   // Keyboard dash mark
	case A_DIT_DOWN:     // Timed dit mark
	case A_DIT_DOWN_D:   // Timed dit mark - commmited to a dash next
	case A_DASH_DOWN:    // Timed dash mark
	case A_DASH_DOWN_D:  // Timed dash mark - committed to a dit next
		key_out_ = true;
		break;
	}
}

// Core engine loop
void engine::run_engine() {
	bool dit;
	bool dash;
	state_ = IDLE;
	while (!close_) {
		get_signs(dit, dash);
		state_ = next_state(state_, dit, dash);
		drive_key_out(state_);
		this_thread::yield();
	}
}

// Invoked by this threa
void engine::run_thread(engine* that) {
	that->run_engine();
}
