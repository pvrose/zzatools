#pragma once

#include "spec_data.h"

#include <string>
#include <set>
#include <map> 
#include <cmath>

using namespace std;

extern spec_data* spec_data_;

// Special comparison operator for bands allowing the bands to 
// be sorted by frequency not namev
struct band_lt {
    bool operator() (string l, string r) const {
        // If the supploed string is not a real band name - return assume 0.0Hz
        double lv = spec_data_->freq_for_band(l);
        double rv = spec_data_->freq_for_band(r);
        if (isnan(lv)) lv = 0.0;
        if (isnan(rv)) rv = 0.0;
        return lv < rv;
    }
};

// The set container sorting on frequency
class band_set : public set<string, band_lt>{};

// The map container sorting on frequency
template <class T>
class band_map : public map<string, T, band_lt>{
};
