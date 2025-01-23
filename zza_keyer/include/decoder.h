#pragma once

#include <thread>
#include <atomic>
#include <cstdint>

struct signal_def;

using namespace std;

// This code decodes the generated CW

class decoder
{
public:
	decoder();
	~decoder();

	// Set initial speed
	void set_speed(double wpm, double weighting);

	// Get current speed
	void get_speed(double& wpm, double& weighting);

	// Get last character decoded
	char get_char();

	// Start the decode
	void start();

	enum decode_t : char {
		NO_CHANGE,                        // No change in monitor
		NOISE,                            // Change should be ignored
		DIT,                              // A dit has been decoded
		DASH,                             // A dash has been decoded
		SIGN,                             // A sign space has been decoded
		CHAR,                             // A character space has been decoded
		WORD,                             // A word space has been decoded
		STUCK_HIGH,                       // Signal has been high for a long time
		STUCK_LOW                         // Signal has been low for a long time
	};

protected:

	void do_key_change(signal_def signal);

	// Work out the sign
	decode_t decode_monitor(signal_def signal);
	// Adjust the speed
	void check_speed(decode_t decode, uint64_t duration);
	// Convert morse to letter
	char morse_to_letter();

	// Send character
	void send_char(char c);

	// Thread to run
	static void run_thread(decoder* that);
	// And non-static version
	void run_decoder();

	// The thread
	thread* t_decoder_;
	// Closing
	atomic<bool> close_;

	// Received values
	// Words per minute
	double wpm_;
	// Dots per dash
	double weighting_;
	// Dit time - seconds
	double dit_time_;
	// Dash time - seconds
	double dash_time_;

	// Previous value 
	bool last_key_;
	// The key has been idle for some time
	bool key_idle_;
	unsigned char code_;                 // Accumulated dits and dashes from output
	unsigned char len_code_;             // Number of valid dits and dashes in code
	bool char_in_progress_;              // Capturing a character is in progress

	char character_;                      // Last captured

};

