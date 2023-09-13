#include "sark_data.h"

#include <cmath>
#include <cstdio>

#include <FL/Fl.H>

sark_data::sark_data(sark_data::scan_params params) {
    readings_.clear();
    data_.clear();
    // Initialise data
    read_index_ = 0;
    params_ = params;
    // Calculate size of data
    int64_t number_points = (params_.end - params_.start) / params_.step;
    size_t sz_array = (size_t)number_points;
    // Initialise all data to NAN
    raw_data raw_dummy = { 0, 0, 0, 0 };
    readings_.assign(sz_array, raw_dummy);
    comp_data comp_dummy = { nan(""), nan(""), nan(""), nan("")};
    data_.assign(sz_array, comp_dummy);
    frequencies_.assign(sz_array, 0);
    // Initialise frequency list
    frequencies_[0] = params_.start;
    for (size_t ix = 1; ix < sz_array; ix++) {
        frequencies_[ix] = frequencies_[ix - 1] + params_.step;
        if (frequencies_[ix] > params_.end || frequencies_[ix] < params_.start) {
            printf("WARNING: Read frequency %d outwith range %d:%d\n",
                frequencies_[ix], params_.start, params_.end);
        }
    }
}

sark_data::~sark_data() {
    readings_.clear();
    data_.clear();
}

void sark_data::add_reading(sark_data::raw_data reading) {
    if (read_index_ >= readings_.size()) {
        printf("ERROR: Unexpected input reading %zu\n", read_index_);
        return;
    }
    readings_[read_index_] = reading;
    comp_data previous = data_[read_index_ - 1];
    double swr = (double)(reading.Vf + reading.Vr) / 
        (double)(reading.Vf - reading.Vr);
    double z = Z0 * (double)reading.Vz / (double)reading.Va;
    double r = ( ( (Z0 * Z0) + (z * z) ) * swr) /
                    (Z0 * ( (swr * swr) + 1));
    double abs_x = sqrt((z * z) - (r * r));
    double prev_x = abs(previous.X);
    double x;
    double delta_x = prev_x - abs_x;
    // Assume frequency increases for each reading
    if (delta_x >= 0.0) {
        // Increase reactance with frequency is inductive
        x = abs_x;
    } else {
        // Decrease reactance with frequency is capacitative
        x = -abs_x;
    }
    data_[read_index_] = { swr, r, z, x};
    read_index_++;
    Fl::check();
}

// Get frequency at the reading point
int64_t sark_data::get_frequency(size_t index) {
    return frequencies_[index];
}

// Get the data at the reading point
sark_data::comp_data sark_data::get_data(size_t index) {
    return data_[index];
}

// Get the parameters
sark_data::scan_params sark_data::get_params() {
    return params_;
}

// Get the size
size_t sark_data::size() {
    return readings_.size();
}

// get the current read position
size_t sark_data::index() {
    return read_index_;
}