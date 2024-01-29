#include "my_fft.h"

#include <iostream>

using namespace std;


const unsigned int FFT_SIZE = 32;
const floatx sample_freq_ = 2000.0;
const floatx delta_t_ =  1.0f / sample_freq_;
const floatx test_freq_ = 688.0f;
const floatx angle_incr_ = 2 * PI * (delta_t_ * test_freq_);


cx data_[FFT_SIZE];

unsigned int sample_num_ = 0;
floatx current_angle_ = 0.0f;
floatx current_time_ = 0.0;



void generate_data() {
    for (unsigned int i = 0; i < FFT_SIZE; i++) {
        data_[i] = cx(sin(current_angle_));
        // cout << i << ": " << current_time_ << " " << current_angle_ << " " << real(data_[i]) << endl;
        current_angle_ += angle_incr_;
        current_time_ += delta_t_;
        if (current_angle_ > 2.0 * PI) {
            current_angle_ -= 2.0 * PI;
        }
    }
}

int main(int argc, char** argv) {
    my_fft f(FFT_SIZE);
    cout << "Starting program.." << endl;
    generate_data();
    // cout << "Generated data" << endl;
    // for (int i = 0; i < FFT_SIZE; i++) {
    //     cout << i << ": " << data_[i] << " " << abs(data_[i]) << endl;
    // }
    f.calculate(data_);
    cout << "FFT" << endl;
    floatx total = 0;
    for (int i = 0; i < FFT_SIZE; i++) {
        floatx fq = sample_freq_ / FFT_SIZE * i;
        total += abs(data_[i]);
        cout << fq << ": " << data_[i] << " " << abs(data_[i]) << endl;
    }
    cout << "Total sample is " << total << endl;
}