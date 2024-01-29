#pragma once
#include <complex.h>

using namespace std;

// Allow simple testing with double or long double
typedef float floatx;
typedef complex<floatx> cx;
const floatx PI = 3.14159265358979323846264338327950288419716939937510;

class my_fft {
    public:
    // Constructor
    my_fft(unsigned int size);

    // Calculate
    void calculate(cx* data);

    //
    cx* data_;
    // bitswap lookup table
    unsigned int* bitswap_lut_;
    // Size
    unsigned int size_;
    unsigned int log2_size_;
    // Omega factors
    cx* omega_factor_;
 
};