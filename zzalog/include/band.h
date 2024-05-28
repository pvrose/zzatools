#pragma once

#include "spec_data.h"

#include <string>
#include <set>

using namespace std;

extern spec_data* spec_data_;

// Special comparison operator for bands
struct band_lt {
    bool operator() (string l, string r) const {
        return spec_data_->freq_for_band(l) < spec_data_->freq_for_band(r);
    }
};
typedef set<string, band_lt> band_set;
