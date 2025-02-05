#include "engine.h"
#include "key_handler.h"
#include "logger.h"
#include "wave_gen.h"

#include <FL/fl_ask.H>
#include <FL/fl_utf8.h> 

// Time keeping is in ms. Define a minute as seconds
const uint64_t MINUTE(60000);
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
	{ 'I', ".. " },
	{ 'J', ".--- " },
	{ 'K', "-.- " },
	{ 'L', ".-.. " },
	{ 'M', "-- " },
	{ 'N', "-. " },
	{ 'O', "--- " },
	{ 'P', ".--. " },
	{ 'Q', "--.- " },
	{ 'R', ".-. " },
	{ 'S', "... " },
	{ 'T', "- " },
	{ 'U', "..- " },
	{ 'V', "...- " },
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
	{ 0xc5, ".--.- "}, // A-rig
};

extern key_handler* key_handler_;
extern logger* logger_;
extern wave_gen* wave_gen_;

engine::state_t engine::old_state_ = IDLE;


engine::engine() 
{
	memset(current_kb_, 0, sizeof(current_kb_));
	next_sign_ = current_kb_;
	close_ = false;
	t_engine_ = nullptr;
}

engine::~engine() {
	close_ = true;
	if (t_engine_) t_engine_->join();
}

void engine::start() {
	t_engine_ = new thread(run_thread, this);
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

// Set kb character to enqueue - allows non-ASCII to be sent
bool engine::send(unsigned int ch) {

	q_character_.push(fl_toupper(ch));
	if (*next_sign_ == 0) next_keyboard();
	return true;
}

// Clear the send queue
void engine::cancel() {
	while (!q_character_.empty()) q_character_.pop();
}

// Set speed
void engine::set_speed(
	double wpm,        // Words per minute
	double weighting   // Dash to dot ratio
) {
	wpm_ = wpm;
	weighting_ = weighting;
	// duration values do not have * or / operators just *= and /=
	dit_time_ = (uint64_t)( MINUTE / (wpm * DITS_PER_WORD));
	dash_time_ = (uint64_t)(dit_time_ * weighting_);
	space_time_ = (uint64_t)(dit_time_ * 2);
}

// Get the next dit or dash to send from the keyboard entry
void engine::next_keyboard() {
	char msg[128];
	// Get the next sign (dit or dash) to send
	if (next_sign_ == nullptr || *next_sign_ == 0) {
		// We do not have one - use the next in the character queue
		if (q_character_.empty()) {
			// None waiting
			memset(current_kb_, 0, sizeof(current_kb_));
			next_sign_ = current_kb_;
			//memcpy(msg, "There is no keyboard data to send\n", sizeof(msg));
			//logger_->log_event(msg);
			return;
		}
		else {
			// Get the next in the queue
			unsigned int unichar = q_character_.front();
			if (LUT_MORSE.find(unichar) == LUT_MORSE.end()) {
				printf("The Unicode character U+%0x cannot be represented as morse\n", unichar);
				unichar = '?';
			}
			memset(current_kb_, 0, sizeof(current_kb_));
			memcpy(current_kb_, LUT_MORSE.at(unichar), strlen(LUT_MORSE.at(unichar)));
			snprintf(msg, sizeof(msg), "Getting next character from keyboard: %c: %s\n", (char)unichar, current_kb_);
			logger_->log_event(msg);
			next_sign_ = &current_kb_[0];
			q_character_.pop();
		}
	}
	else {
		next_sign_++;
	}
	if (*next_sign_) {
		snprintf(msg, sizeof(msg), "Selecting %c from keyboard\n", (*next_sign_));
		logger_->log_event(msg);
	}
}

static key_state pk = NEITHER;
// Return the current state of the input paddle/key/keyboard
void engine::get_signs(bool& dit, bool& dash) {
	key_state ks;
	if (type_ != KEYBOARD) {
		key_state ks = key_handler_->get_state();
		char ev[128];
		if (ks != pk) {
			snprintf(ev, sizeof(ev), "Engine sees keys %d\n", (int)ks);
			logger_->log_event(ev);
			pk = ks;
		}
		dit = (ks == LEFT) || (ks == BOTH);
		dash = (ks == RIGHT) || (ks == BOTH);
	}
}

// Main state machine
engine::state_t engine::next_state(state_t state, bool dit, bool dash, char* next_kb, uint64_t& gap) {
	state_t next = state;
	gap = 0;
	switch (state) {
	case IDLE:           // Idle - nothing to process
	{
		switch (type_) {
		case KEYBOARD: {
			// dit and dash are exclusive
			switch (*next_kb) {
			case '.':
				next = KB_DIT_DOWN;
				gap = dit_time_;
				break;
			case '-':
				next = KB_DASH_DOWN;
				gap = dash_time_;
				break;
			default:
				next_keyboard();
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
		break;
	}
	case KB_DIT_UP:      // Keyboard dot space 
	{
		switch (*next_kb) {
		case '.':
			next = KB_DIT_DOWN;
			gap = dit_time_;
			break;
		case '-':
			next = KB_DASH_DOWN;
			gap = dash_time_;
			break;
		case ' ':
			next = KB_SPACE;
			gap = space_time_;
			break;
		}
		
		break;
	}
	case KB_DIT_DOWN:    // Keyborad dot mark
	{
		next = KB_DIT_UP;
		gap = dit_time_;
		next_keyboard();
		break;
	}
	case KB_DASH_UP:     // Keyboard dash space
		switch (*next_kb) {
		case '.':
			next = KB_DIT_DOWN;
			gap = dit_time_;
			break;
		case '-':
			next = KB_DASH_DOWN;
			gap = dash_time_;
			break;
		case ' ':
			next = KB_SPACE;
			gap = space_time_;
			break;
		}
		break;
	}
	case KB_DASH_DOWN:   // Keyboard dash mark
	{
		next = KB_DASH_UP;
		gap = dit_time_;
		next_keyboard();
		break;

	}
	case KB_SPACE:
	{
		switch (*next_kb) {
		case '.':
			next = KB_DIT_DOWN;
			gap = dit_time_;
			break;
		case '-':
			next = KB_DASH_DOWN;
			gap = dash_time_;
			break;
		default:
			next = IDLE;
		}
		next_keyboard();
		break;
	}
	case A_DIT_DOWN:     // Timed dit mark
	{
		if (type_ == SQUEEZE_B && dash) {
			next = A_DIT_DOWN_D;
		}
		else {
			next = A_DIT_UP;
			gap = dit_time_;
		}
		break;
	}
	case A_DIT_UP:       // Timed dit space
	{
		if ((type_ == FULL || type_ == SEMI) && dit) {
			next = A_DIT_DOWN;
			gap = dit_time_;
		}
		else if (type_ != SEMI && dash) {
			next = A_DASH_DOWN;
			gap = dash_time_;
		}
		else if (type_ == SEMI && dash) {
			next = UT_DASH_DOWN;
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
	case A_DIT_DOWN_D:   // Timed dit mark - commmited to a dash next
	{
		next = A_DIT_UP_D;
		break;
	}
	case A_DIT_UP_D:     // Timed dit space - committed to a dash
	{
		next = A_DASH_DOWN;
		gap = dash_time_;
		break;
	}
	case A_DASH_DOWN:    // Timed dash mark 
	{
		if (type_ == SQUEEZE_B && dit) {
			next = A_DASH_DOWN_D;
		}
		else {
			next = A_DASH_UP;
			gap = dit_time_;
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
	case A_DASH_DOWN_D:  // Timed dash mark - committed to a dit next
	{
		next = A_DASH_UP_D;
		break;
	}
	case A_DASH_UP_D:    // Timed dash space - committed to a dit
	{
		next = A_DIT_DOWN;
		gap = dit_time_;
		break;
	}
	case A_SPACE:        // Timed space space
	{
		next = IDLE;
		break;
	}
	}
	if (next != state) {
		char msg[128];
		if (type_ == KEYBOARD) {
			snprintf(msg, sizeof(msg), "KB: %c Next state = %s(%d) Time = %I64d ms\n",
				*next_kb, state_text_[next], (int)next, gap); 
		}
		else {
			snprintf(msg, sizeof(msg), "Dit:%d Dash:%d Next state = %s(%d) Time = %I64d ms\n",
				dit, dash, state_text_[next], (int)next, gap);
		}
		logger_->log_event(msg);
	}
	return next;
}

// Drive key-out
bool engine::key_out(state_t state) {
	switch (state) {
	case IDLE:           // Idle - nothing to process
	case KB_DIT_UP:      // Keyboard dot space
	case KB_DASH_UP:     // Keyboard dash space
	case KB_SPACE:       // Keyboard space
	case A_DIT_UP:       // Timed dit space
	case A_DIT_UP_D:     // Timed dit space - committed to a dash
	case A_DASH_UP:      // Timed dash space
	case A_DASH_UP_D:    // Timed dash space - committed to a dit
	case A_SPACE:        // Timed space space
		return false;
	case UT_DIT_DOWN:    // Untimed dit mark,
	case UT_DASH_DOWN:   // Untimed dash mark,
	case KB_DIT_DOWN:    // Keyborad dot mark
	case KB_DASH_DOWN:   // Keyboard dash mark
	case A_DIT_DOWN:     // Timed dit mark
	case A_DIT_DOWN_D:   // Timed dit mark - commmited to a dash next
	case A_DASH_DOWN:    // Timed dash mark
	case A_DASH_DOWN_D:  // Timed dash mark - committed to a dit next
		return true;
	}
}

// Check if state is an untimed one
bool engine::is_untimed(state_t state) {
	switch (state) {
	case UT_DASH_DOWN:
	case UT_DIT_DOWN:
	case IDLE:
		return true;
	default:
		return false;
	}
}

// Check if state is a timed mark
bool engine::is_timed_mark(state_t state) {
	switch (state) {
	case KB_DIT_DOWN:
	case KB_DASH_DOWN:
	case A_DIT_DOWN:
	case A_DIT_DOWN_D:
	case A_DASH_DOWN:
	case A_DASH_DOWN_D:
		return true;
	default:
		return false;
	}
}

// KB space
bool engine::is_keyboard_space(state_t state) {
	switch (state) {
	case KB_DIT_UP:
	case KB_DASH_UP:
		return true;
	default:
		return false;
	}
}

// signal call back - return next signal
void engine::cb_signal(signal_def* data, void* user_data) {
	engine* that = (engine*)user_data;
	that->get_signal(data);
}

//// Get signal
//void engine::get_signal(signal_def* data) {
//	bool dit = false;
//	bool dash = false;
//	uint64_t gap = 0;
//	if (!is_timed_mark(state_)) get_signs(dit, dash);
//	state_t previous_state = state_;
//	state_ = next_state(state_, dit, dash, gap);
//	bool key = key_out(state_);
//	if (state_ != previous_state) {
//		char ev[128];
//		snprintf(ev, sizeof(ev), "State %s - sending %d for %d ms\n", state_text_[state_], key, gap);
//		logger_->log_event(ev);
//	}
//	*data = { key, gap };
//}

// get signal
void engine::get_signal(signal_def* data) {
	uint64_t gap;
	state_t next = next_state(state_, dit_, dash_, next_sign_, gap);
	data->value = key_out(next);
	data->durn_ms = gap;
	if (gap) {
		char msg[128];
		snprintf(msg, sizeof(msg), "Wave gen taking new signal %d for %lld ms\n", data->value, data->durn_ms);
		logger_->log_event(msg);
	}
	state_ = next;
}

// Core engine loop
void engine::run_engine() {
	bool previous_dit = false;
	bool previous_dash = false;
	char msg[128];
	uint64_t gap;
	while (!close_) {
		if (!is_timed_mark(state_)) get_signs(dit_, dash_);
		this_thread::yield();
	}

}

// Invoked by this thread
void engine::run_thread(engine* that) {
	that->run_engine();
}
