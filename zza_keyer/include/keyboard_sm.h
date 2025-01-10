#pragma once

#include "paddle_sm.h"

#include <queue>
#include <map>

using namespace std;

// This class generates dits and dashes corresponding to the typed in letters
class keyboard_sm : public paddle_sm
{
public:
	keyboard_sm();
	~keyboard_sm();

	// Add a character to the queue
	bool add_char(unsigned int c);
	// Callback
	void callback(void(*cb_empty)(void* v), void* v);



protected:
	// Send the character
	void send_next_char();
	// Send the next sign
	void send_next_sign();
	// Intercept the next state transition to set up next character
	virtual state_t next_state();
	// Instance of call back
	void (*cb_empty_)(void* v);
	// User data
	void* cb_target_;
	// Buffer of characters to send
	static const int BUFFER_SIZE = 1024;
	// Use unsigned int as some characters are non-ASCII
	unsigned int buffer_[BUFFER_SIZE];
	// Index of current character
	int ix_current_;
	// Index of head
	int ix_head_;
	// Current character
	const char* current_;
	// Current sign
	const char* current_sign_;
	// 
};

