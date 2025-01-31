#include "wave_gen.h"

#include <cmath>
#include <cstdio>

using namespace std;


const double PI = 3.14159265358979323846264338327950288419716939937510;

const int BUFFER_DEPTH = 64;


wave_gen::wave_gen() {
	sine_table_ = nullptr;
	rise_table_ = nullptr;
	fall_table_ = nullptr;
	// Default parameters
	set_params(48000.0, 700.0, 0.005, 0.005, RAMP);
	set_buffer_depth(BUFFER_DEPTH);
	previous_signal_ = false;
}

wave_gen::~wave_gen() {
	if (pa_initialised_) {
		Pa_Terminate();
	}
	delete[] sine_table_;
	delete[] rise_table_;
	delete[] fall_table_;
	// TODO - close portaudio
}

// Generate the derived parameters
void wave_gen::process_params() {
	// Number of samples needed to generate a sine wave closest to target frequency
	cycle_samples_ = (int)round(sample_rate_ / target_frequency_);
	// Calculate the actual frequency these samples will generate
	audio_frequency_ = sample_rate_ / cycle_samples_;
	// And the number of samples required 
	rise_samples_ = (int)floor(sample_rate_ * rise_time_) + 1;
	fall_samples_ = (int)floor(sample_rate_ * fall_time_) + 1;
}

// Generate the sine table
void wave_gen::create_sine_table() {
	// Create the table with enough entries
	delete[] sine_table_;
	sine_table_ = new double[cycle_samples_];
	for (int ix = 0; ix < cycle_samples_; ix++) {
		sine_table_[ix] = sin(2.0 * PI * ix / cycle_samples_);
	}
}

// Generate edge tables
void wave_gen::create_edge_tables() {
	// delete the current data
	delete[] rise_table_;
	delete[] fall_table_;
	double d_ampl;
	double d_radian;
	switch (shape_) {
	case SHARP:
		// Sharp 0->1 and 1->0 transition
		rise_table_ = new double[2];
		rise_table_[0] = 0.0;
		rise_table_[1] = 1.0;
		fall_table_ = new double[2];
		fall_table_[0] = 1.0;
		fall_table_[1] = 0.0;
		break;
	case RAMP:
		// Linear rise and fall
		rise_table_ = new double[rise_samples_];
		// delta amplitude for each sample
		d_ampl = 1.0 / rise_samples_;
		rise_table_[0] = 0.0;
		for (int ix = 1; ix < rise_samples_; ix++) {
			rise_table_[ix] = fmin(1.0, rise_table_[ix - 1] + d_ampl);
		}
		fall_table_ = new double[fall_samples_];
		d_ampl = 1.0 / fall_samples_;
		fall_table_[0] = 1.0;
		for (int ix = 1; ix < fall_samples_; ix++) {
			fall_table_[ix] = fmax(0.0, fall_table_[ix - 1] - d_ampl);
		}
		break;
	case COSINE:
		// Raised cosine 
		rise_table_ = new double[rise_samples_];
		// delta angle for each sample
		d_radian = PI / (rise_samples_);
		for (int ix = 0; ix < rise_samples_; ix++) {
			rise_table_[ix] = 0.5 * (1.0 - cos(ix * d_radian));
		}
		fall_table_ = new double[fall_samples_];
		d_radian = PI / (fall_samples_ - 1);
		for (int ix = 0; ix < fall_samples_; ix++) {
			fall_table_[ix] = 0.5 * (1.0 + cos(ix * d_radian));
		}
		break;
	}
}

// Set the audio parameters
void wave_gen::set_params(
	double sample_rate,            // Samples per second for the audio
	double target_freq,            // The target audio frequency - exact will differ slightly
	double rise_time,              // The time in seconds taken from 0 to 1
	double fall_time,              // The time in seconds taken from 1 to 0
	shape_t shape                  // The edge shape
) {
	sample_rate_ = sample_rate;
	target_frequency_ = target_freq;
	rise_time_ = rise_time;
	fall_time_ = fall_time;
	shape_ = shape;
	pa_initialised_ = false;
	process_params();
	create_sine_table();
	create_edge_tables();
}

double wave_gen::get_sample_rate() {
	return sample_rate_;
}

double wave_gen::get_rise_time() {
	return rise_time_;
}

double wave_gen::get_fall_time() {
	return fall_time_;
}

int wave_gen::get_buffer_depth() {
	return buffer_depth_;
}

void wave_gen::set_buffer_depth(int buffer_depth) {
	buffer_depth_ = buffer_depth;
	// TODO: Reconfigure portaudio
}

double wave_gen::get_audio_freq(bool actual) {
	if (actual) return audio_frequency_;
	else return target_frequency_;
}

// Initialise PortAudio
bool wave_gen::initialise_pa() {
	PaError err;

	/* Initialize library before making any other calls. */
	err = Pa_Initialize();
	if (err != paNoError) return false;

	pa_initialised_ = true;

	/* Open an audio I/O stream. */
	// TODO Replace with specific stream selection
	err = Pa_OpenDefaultStream(&stream_,
		0,          /* no input channels */
		1,          /* stereo output */
		paFloat32,  /* 32 bit floating point output */
		sample_rate_,
		buffer_depth_,        /* frames per buffer */
		cb_pa_stream,
		this);      // Pointer to this for callback
	if (err != paNoError) return false;

	err = Pa_StartStream(stream_);
	if (err != paNoError) return false;

	return true;
}

// Instance dependant version of callback
// Note this is the callback path from Portaudio.
int wave_gen::pa_stream(const void* input,
	void* output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags) {
	// Pointer to output buffer 
	float* out = (float*)output;
	signal_def last_signal;
	// Now send the data
	for (unsigned int ix = 0; ix < frameCount; ix++) {;
		if (current_signal_.durn_ms == 0) {
			signal_def next_signal;
			get_next_signal(&next_signal, engine_);
			last_signal.value = current_signal_.value;
			last_signal.durn_ms = sample_number_ / (sample_rate_ / 1000.0);
			if (next_signal.value == current_signal_.value) {
				// We are continuing to send the same signal
				if (next_signal.durn_ms == 0) {
					current_signal_.durn_ms = 0;
					samples_in_signal_ = 0;
				}
				else {
					current_signal_.durn_ms += next_signal.durn_ms;
					samples_in_signal_ = (uint64_t)(current_signal_.durn_ms * (sample_rate_ / 1000.0));
				}
			}
			else {
				current_signal_ = next_signal;
				samples_in_signal_ = (uint64_t)(current_signal_.durn_ms * (sample_rate_ / 1000.0));
				sample_number_ = 0;
			}
		}
		if (current_signal_.value) {
			// If we haven't changed value then keep at the sine wave
			if (previous_signal_) {
				*out = (float)(sine_table_[sine_index_]);
			}
			// Otherwise apply the shape for the rise period
			else if (sample_number_ < rise_samples_) {
				*out = (float)(rise_table_[sample_number_] * sine_table_[sine_index_]);
			}
			// And then keep the sine wave
			else {
				*out = (float)(sine_table_[sine_index_]);
			}
		}
		else {
			if (!previous_signal_) {
				*out = 0.0F;
			}
			else if (sample_number_ < fall_samples_) {
				*out = (float)(fall_table_[sample_number_] * sine_table_[sine_index_]);
			}
			else {
				*out = 0.0F;
			}

		}
		// Increment pointers
		out++;
		// Increment index into sine table and reset to 0 when complete
		sine_index_++;
		if (sine_index_ == cycle_samples_) sine_index_ = 0;
		// Increment index into signal and when complete get next signal
		sample_number_++;
		if (sample_number_ >= samples_in_signal_ && samples_in_signal_ != 0) {
			signal_def next_signal;
			get_next_signal(&next_signal, engine_);
			last_signal.value = current_signal_.value;
			last_signal.durn_ms = sample_number_ / (sample_rate_ / 1000.0);
			//printf("Signal complete %d for %d ms (%d samples) \n", last_signal.value, last_signal.durn_ms, sample_number_);
			if (next_signal.value == current_signal_.value) {
				// We are continuing to send the same signal
				if (next_signal.durn_ms == 0) {
					current_signal_.durn_ms = 0;
					samples_in_signal_ = 0;
				}
				else {
					current_signal_.durn_ms += next_signal.durn_ms;
					samples_in_signal_ = (uint64_t)(current_signal_.durn_ms * (sample_rate_ / 1000.0));
				}
			} else {
				current_signal_ = next_signal;
				samples_in_signal_ = (uint64_t)(current_signal_.durn_ms * (sample_rate_ / 1000.0));
				sample_number_ = 0;
			}
		}
	}
	return paContinue;
}

// Callback - portaudio requests data or reports errors
int wave_gen::cb_pa_stream(const void* input,
	void* output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData) {
	wave_gen* that = (wave_gen*)userData;
	// call the non-static version
	return that->pa_stream(input, output, frameCount, timeInfo, statusFlags);
}

// Return current state of the signal
bool wave_gen::get_key() {
	return current_signal_.value;
}

// Return current signal and its duration
// Return current signal and its duration
signal_def wave_gen::get_signal() {
	signal_def result;
	// Return current signal value
	result.value = current_signal_.value;
	// Return number of milliseconds current duration
	result.durn_ms = (uint64_t)(sample_number_ / (sample_rate_ / 1000.));
	return result;
}

void wave_gen::callback(void (*cb)(signal_def*, void*), void* user_data) {
	get_next_signal = cb;
	engine_ = user_data;
}