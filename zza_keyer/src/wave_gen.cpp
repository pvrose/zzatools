#include "wave_gen.h"

#include <cmath>

using namespace std;


const double PI = 3.14159265358979323846264338327950288419716939937510;


wave_gen::wave_gen() {
	sine_table_ = nullptr;
	// Default parameters
	set_params(48000.0, 700.0, 0.005, 0.005, RAMP);
	set_buffer_depth(64);
}

wave_gen::~wave_gen() {
	delete[] sine_table_;
	// TODO - close portaudio
}

// Generate the derived parameters
void wave_gen::process_params() {
	// Number of samples needed to generate a sine wave closest to target frequency
	num_samples_ = (int)round(sample_rate_ / target_frequency_);
	// Calculate the actual frequency these samples will generate
	audio_frequency_ = sample_rate_ / num_samples_;
	// Calculate the steps to change the envelope value for each rise/fall sample
	rise_step_ = 1.0 / (rise_time_ * sample_rate_);
	fall_step_ = 1.0 / (fall_time_ * sample_rate_);
}

// Generate the sine table
void wave_gen::create_sine_table() {
	// Create the table with enough entries
	delete[] sine_table_;
	sine_table_ = new double[num_samples_];
	for (int ix = 0; ix < num_samples_; ix++) {
		sine_table_[ix] = sin(2.0 * PI * ix / num_samples_);
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
	process_params();
	create_sine_table();
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


