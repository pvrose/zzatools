#pragma once

#include "portaudio.h"

#include <cstdint>
#include <list>
#include <queue>

using namespace std;

// This class generates the audio waveform representing the shaped MCW tone
// It uses portaudio as the platform-independent layer for sending the audio

// Envelope shaping mode
enum shape_t : char {
	SHARP,      // Rise/fall is instantaneous
	RAMP,       // Steady rise or fall
	COSINE      // Shaped rise or fall - like a cosine curve
};

// Signal definition
struct signal_def {
	bool value;         // Mark (true) or space (false)
	uint64_t durn_ms;    // Duration in milliseconds - UINT64_MAX is close to indefinite
};

class wave_gen
{
public:
	wave_gen();
	~wave_gen();

	// Set the audio parameters
	void set_params(
		double sample_rate,            // Samples per second for the audio
		double target_freq,            // The target audio frequency - exact will differ slightly
		double rise_time,              // The time in seconds taken from 0 to 1
		double fall_time,              // The time in seconds taken from 1 to 0
		shape_t shape                  // How to shape the rise and fall edges
	);
	// Change the buffer depth
	void set_buffer_depth(
		int buffer_depth               // The depth of the port_audio data buffer
	);
	// Get the audio parameters
	double get_sample_rate();
	double get_rise_time();
	double get_fall_time();
	int get_buffer_depth();
	double get_audio_freq(bool actual);

	// Callback - portaudio requests data or reports errors
	static int cb_pa_stream(const void* input,
		void* output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);

	// Return current state of signal
	bool get_key();
	// Get current state of signal and how long
	signal_def get_signal();
	// Callback set callback
	void callback(void (*cb)(signal_def*, void*), void* user_data);
	// Initialise portaudio
	bool initialise_pa();

protected:
	// Instance dependant version of callback
	int pa_stream(const void* input,
		void* output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags);
	// Create the sine table
	void create_sine_table();
	// Create dependant parameters
	void process_params();
	// Create the edge tables
	void create_edge_tables();

	void (*get_next_signal)(signal_def*, void*);

	// Parameters
	// Audio sample rate (samples/second)
	double sample_rate_;
	// Number of samples in 1 period at audio frequency
	uint64_t cycle_samples_;
	// Rise time in seconds
	double rise_time_;
	// Fall time in seconds
	double fall_time_;
	// Edge shape
	shape_t shape_;
	// Buffer depth
	int buffer_depth_;
	// Audio frequency - calculated
	double audio_frequency_;
	// Target frequency
	double target_frequency_;
	// Sine table to generate audio waveform
	double* sine_table_;
	// Current index into sine table
	uint64_t sine_index_;
	// Edge table - rising ege
	double* rise_table_;
	// Edge table - falling edge
	double* fall_table_;
	// Number of samples in rising edge
	uint64_t rise_samples_;
	// ... and in falling edge
	uint64_t fall_samples_;
	// Number of samples in signal
	uint64_t samples_in_signal_;
	// Current sample number
	uint64_t sample_number_;
	// Current signal
	signal_def current_signal_;
	// Previous signal value
	bool previous_signal_;
	// Port audio stream
	PaStream* stream_;

	// Engine
	void* engine_;
	
};

