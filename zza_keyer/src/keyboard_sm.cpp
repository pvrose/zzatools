#include "keyboard_sm.h"
#include "key_handler.h"

#include <map>

const map<unsigned int, const char*> encode_lut = {
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
	{ '\xdc', "..-- " }, // U-umlaut
	{ '\xC4', ".-.- " }, // A-umlaut
	{ '\xd6', "---. " }, // O-umlaut
	{ '\xc9', "-.-.. "}, // E-acute
};


keyboard_sm::keyboard_sm() : paddle_sm() {
	set_mode(FULL_BUG);
}

keyboard_sm::~keyboard_sm() {

}

// Add a character to the queue
bool keyboard_sm::add_char(unsigned int c) {
	if ((ix_current_ - ix_head_) % BUFFER_SIZE == 1) {
		printf("Keyboard buffer full");
		return false;
	} 
	buffer_[ix_head_++] = c;
	if (ix_head_ == BUFFER_SIZE) ix_head_ = 0;
	return true;
}

// Send the character
void keyboard_sm::send_next_char() {
	if (ix_current_ != ix_head_) {
		unsigned int c = buffer_[ix_current_++];
		if (encode_lut.find(c) != encode_lut.end()) {
			current_ = encode_lut.at(c);
			current_sign_ = current_;
			send_next_sign();
		}
	}
	// If we have become empty
	if (ix_current_ == ix_head_) {
		cb_empty_(cb_target_);
	}
}

// Send the sign
void keyboard_sm::send_next_sign() {
	char sign = *(current_sign_++);
	switch (sign) {
	case '.':
		update_dit_dash(key_state::LEFT);
		break;
	case '-':
		update_dit_dash(key_state::RIGHT);
		break;
	case ' ':
		update_dit_dash(key_state::NEITHER);
		break;
	}
}

// Intercept the next state transition to set up next character
paddle_sm::state_t keyboard_sm::next_state() {
	if (!enabled_) return IDLE;
	// Check if we need to send another sign, start the next character or what
	switch (the_state_) {
	case DIT_UP:
	case DASH_UP:
	case SPACE_UP:
	case IDLE:
	case GUARD_UP:
		if (*current_sign_ == '\0') {
			send_next_char();
		}
		else {
			send_next_sign();
		}
	}
	// Now actually do the next state decision
	return paddle_sm::next_state();
}

void keyboard_sm::callback(void(*cb_empty)(void* v), void* v) {
	cb_empty_ = cb_empty;
	cb_target_ = v;
}
