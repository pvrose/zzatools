#pragma once

#include <map>
#include <string>
#include <queue>
#include <thread>
#include <atomic>
#include <cstdint>
#include <list>

using namespace std;

struct signal_def;

// This is the main encoding engine

enum engine_type : char {
	KEYBOARD,     // Use keyboard
	STRAIGHT,     // Straight key
	SQUEEZE_A,    // Squeeze paddle - mode A
	SQUEEZE_B,    // Squeeze paddle - mode B
	FULL,         // Fully automatic
	SEMI,         // Semi automatic
};

const map<engine_type, string> engine_descriptors_ = {
	{ KEYBOARD, "Use keyboard entry"},
	{ STRAIGHT, "Straight key"},
	{ SQUEEZE_A, "Squeeze paddle - mode A"},
	{ SQUEEZE_B, "Squeeze paddle - mode B"},
	{ FULL, "Fully automatic"},
	{ SEMI, "Semi-automatic"}
};

class engine
{
public:
	engine();
	~engine();

	enum state_t : char {
		IDLE,           // Idle - nothing to process
		UT_DIT_DOWN,    // Untimed dit mark,
		UT_DASH_DOWN,   // Untimed dash mark,
		KB_DIT_UP,      // Keyboard dot space
		KB_DIT_DOWN,    // Keyborad dot mark
		KB_DASH_UP,     // Keyboard dash space
		KB_DASH_DOWN,   // Keyboard dash mark
		KB_SPACE,       // Keyboard space
		A_DIT_DOWN,     // Timed dit mark
		A_DIT_UP,       // Timed dit space
		A_DIT_DOWN_D,   // Timed dit mark - commmited to a dash next
		A_DIT_UP_D,     // Timed dit space - committed to a dash
		A_DASH_DOWN,    // Timed dash mark
		A_DASH_UP,      // Timed dash space
		A_DASH_DOWN_D,  // Timed dash mark - committed to a dit next
		A_DASH_UP_D,    // Timed dash space - committed to a dit
		A_SPACE,        // Timed space space
	};

	map<state_t, string> state_text_ = {
		{ IDLE,           "Idle" },
		{ UT_DIT_DOWN,    "Untimed dit mark" },
		{ UT_DASH_DOWN,   "Untimed dash mark" },
		{ KB_DIT_UP,      "Keyboard dot space" },
		{ KB_DIT_DOWN,    "Keyboard dot mark" },
		{ KB_DASH_UP,     "Keyboard dash space" },
		{ KB_DASH_DOWN,   "Keyboard dash mark" },
		{ KB_SPACE,       "Keyboard space" },
		{ A_DIT_DOWN,     "Timed dit mark" },
		{ A_DIT_UP,       "Timed dit space" },
		{ A_DIT_DOWN_D,   "Timed dit mark - commmited to a dash" },
		{ A_DIT_UP_D,     "Timed dit space - committed to a dash" },
		{ A_DASH_DOWN,    "Timed dash mark" },
		{ A_DASH_UP,      "Timed dash space" },
		{ A_DASH_DOWN_D,  "Timed dash mark - committed to a dit" },
		{ A_DASH_UP_D,    "Timed dash space - committed to a dit" },
		{ A_SPACE,        "Timed space space" }
	};

	// Set the type
	void type(engine_type t);
	// Get the type
	engine_type type();

	// Return idle state
	bool idle();

	// Set kb character to enqueue - allows non-ASCII to be sent
	bool send(unsigned int ch);

	// Set speed
	void set_speed(
		double wpm,        // Words per minute
		double weighting   // Dash to dot ratio
	);

	// Callback from wave_gen
	static void cb_signal(signal_def* data, void* user_data);

protected:

	// Return the current state of the input paddle/key/keyboard
	void get_signs(bool& dit, bool& dash);
	// Main state machine
	state_t next_state(state_t state, bool dit, bool dash, uint64_t& gap);
	// Drive key-out
	bool drive_key_out(state_t state);
	//// Core engine loop
	//void run_engine();
	//// Invoked by this threa
	//static void run_thread(engine* that);
	// Retunr true if the state si untimed - so check inputs
	bool is_untimed(state_t state);
	// Returns true if the state is a timed mark - as will be follwoed by a timed space
	bool is_timed_mark(state_t state);
	// Non-statis call
	void get_signal(signal_def* data);

	// The state
	state_t state_;
	// Engine type
	engine_type type_;
	// KB character queue
	queue<unsigned int> q_character_;
	//// The thread
	//thread* t_engine_;
	//// Stop processing loop
	//atomic<bool> close_;
	// Words per minute
	double wpm_;
	// Weighting
	double weighting_;
	// Dit time - milliseconds
	uint64_t dit_time_;
	// Dash time - seconds
	uint64_t dash_time_;
	// Space time
	uint64_t space_time_;
	// Current keyboard character
	char current_kb_[16];
	// Pointer within above
	char* next_sign_;
	// Old state
	static state_t old_state_;
	//// Debug history
	//list<signal_def> history_;

};

