#pragma once

#include <map>
#include <string>
#include <queue>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std;
using namespace std::chrono;

// This is the main encoding engine

enum engine_type : char {
	KEYBOARD,     // Use keyboard
	STRAIGHT,     // Straight key
	SQUEEZE_A,    // Squeeze paddle - mode A
	SQUEEZE_B,    // Squeeze paddle - mode B
	FULL,         // Fully automatic
	SEMI,         // Semi automatic
};

const map<engine_type, string> engine_descriptors = {
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

	// Set the type
	void type(engine_type t);
	// Get the type
	engine_type type();

	// Return idle state
	bool idle();

	// Return the keyout state
	bool key_out();

	// Set kb character to enqueue - allows non-ASCII to be sent
	bool send(unsigned int ch);

	// Set speed
	void set_speed(
		double wpm,        // Words per minute
		double weighting   // Dash to dot ratio
	);

	// Start processing
	bool start();

protected:
	// Return the current state of the input paddle/key/keyboard
	bool get_signs(bool& dit, bool& dash);
	// Main state machine
	state_t next_state(state_t state, bool dit, bool dash);
	// Drive key-out
	void drive_key_out(state_t state);
	// Core engine loop
	void run_engine();
	// Invoked by this threa
	static void run_thread(engine* that);

	// The state
	state_t state_;
	// Engine type
	engine_type type_;
	// KB character queue
	queue<unsigned int> q_character_;
	// Time points
	high_resolution_clock::time_point last_edge_;
	high_resolution_clock::time_point next_edge_;
	// The thread
	thread* t_engine_;
	// Stop processing loop
	atomic<bool> close_;
	// Key out
	atomic<bool> key_out_;
	// Words per minute
	double wpm_;
	// Weighting
	double weighting_;
	// Dit time - seconds
	milliseconds dit_time_;
	// Dash time - seconds
	milliseconds dash_time_;
	// Space time
	milliseconds space_time_;
	// Current keyboard character
	char current_kb_[16];
	// Pointer within above
	char* next_sign_;

};

