#pragma once

#include <map>
#include <string>

using namespace std;

// This class interprets the paddle switches and turns them into a sequence
// of dots, dashes or spaces

// Forward declaration
class key_out;

// Paddle types:
enum paddle_t : char {
	NONE,
	STRAIGHT_LEFT,   // Left paddle acts as straight key
	STRAIGHT_RIGHT,  // Right paddle acts as straight key
	STRAIGHT_EITHER, // Either paddle acts as straight key
	SQUEEZE_A,       // Mode A squeeze paddle
	SQUEEZE_A_REV,   // do. with paddles reversed
	SQUEEZE_B,       // Mode B squeeze paddle
	SQUEEZE_B_REV,   // do. with paddles reversed
	FULL_BUG,        // Full bug, left strean of dits, right of dots - not squeeze
	FULL_BUG_REV,    // do with paddles reversed
	SEMI_BUG,        // Semi-automatic bug, only dits created
	SEMI_BUG_REV,    // do. with paddles reversed
};

const map<paddle_t, string> paddle_types_ = {
	{ NONE, "" },
	{ STRAIGHT_LEFT, "Straight key - tip or left" },
	{ STRAIGHT_RIGHT, "Straight key - ring or right" },
	{ STRAIGHT_EITHER, "Straight key - either" },
	{ SQUEEZE_A, "Squeeze - iambic mode A" },
	{ SQUEEZE_A_REV, "Squeeze - iambic mode A (reversed)" },
	{ SQUEEZE_B, "Squeeze - iambic mode B" },
	{ SQUEEZE_B_REV, "Squeeze - iambic mode B (reversed)" },
	{ FULL_BUG, "Fully-auto bug key" },
	{ FULL_BUG_REV, "Full... - reversed" },
	{ SEMI_BUG, "Semi-auto bug key" },
	{ SEMI_BUG_REV, "Semi... - reversed" }
};

enum key_state : char;

class paddle_sm
{
public:
	paddle_sm();
	~paddle_sm();

	// Set paddle type
	void set_mode(paddle_t type);
	// Set speed
	void set_speed(
		double wpm,        // Words per minute
		double weighting   // Dash to dot ratio
	);

	enum state_t : char {
		IDLE,                             // Not sending any character
		DIT_UP,                           // Non-sending dit
		DIT_DOWN,                         // Sending dit
		DASH_UP,                          // Non-sending dash
		DASH_DOWN,                        // Sending dash
		GUARD_UP,                         // Guard period (not committed to a space)
		SPACE_UP,                         // Non-sending space
		PASS_DIT,                         // Pass dit paddle through  - no timing control
		PASS_DASH,                        // Pass dash paddle through - no tmining control
		PASS_SPACE,                       // Pass through no signal
	};

	// Access key out
	void set_key_out(key_out* k);

	// Enable the paddle SM
	void enable(bool en);

	// Call back to receive paddle state change
	static void cb_paddle(void* v, key_state state);
	// Timer callback
	static void cb_timer(void* v);

protected:

	// Check next state - called on timer or paddle call backs
	state_t next_state();
	// Straight key has special states (ON (DIT) of OFF (!DIT))
	state_t ns_straight();
	state_t ns_squeeze();
	state_t ns_full_bug();
	state_t ns_semi_bug();
	// Update dit and dash values
	void update_dit_dash(key_state ps);
	// Process dit and dash is type dependent
	void udd_squeeze(bool mode_a);
	// Send key
	void send_key(state_t state, double duration);
	// Paddle mode
	paddle_t type_;
	// Dit paddle
	key_state dit_paddle_;
	// Current state
	state_t the_state_;
	// Dit pending whie sending a dash
	bool dit_pending_;
	// Dash pending while sending a dit.
	bool dash_pending_;
	// Words per minute
	double wpm_;
	// Dots per dash
	double weighting_;
	// Dit time - seconds
	double dit_time_;
	// Dash time - seconds
	double dash_time_;
	// Guard time
	double guard_time_;
	// Space time
	double space_time_;
	// Current dit state
	bool dit_;
	// Current dash state
	bool dash_;


	// Key out
	key_out* key_out_;
	// Enabled
	bool enabled_;
};

