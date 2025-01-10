#pragma once

#include "portaudio.h"

// This class generates the audio waveform representing the shaped MCW tone
// It uses portaudio as the platform-independent layer for sending the audio
enum shape_t : char {
	SHARP,      // Rise/fall is instantaneous
	RAMP,       // Steady rise or fall
	COSINE      // Shaped rise or fall - like a cosine curve
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

protected:
	// Instance dependant version of callback
	int pa_stream(const void* input,
		void* output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
	// Create the sine table
	void create_sine_table();
	// Create dependant parameters
	void process_params();


	// Parameters
	// Audio sample rate (samples/second)
	double sample_rate_;
	// Number of samples in 1 period at audio frequency
	int num_samples_;
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
	// Amount to increment envelope during rise time
	double rise_step_;
	// Amount to decrement envelope during fall time
	double fall_step_;
	// Sine table to generate audio waveform
	double* sine_table_;
	// Current index into sine table
	int sine_index_;


};

