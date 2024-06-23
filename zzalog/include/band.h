#pragma once

#include "spec_data.h"

#include <string>
#include <set>
#include <map> 

using namespace std;

extern spec_data* spec_data_;

// Special comparison operator for bands allowing the bands to 
// be sorted by frequency not name
struct band_lt {
    bool operator() (string l, string r) const {
        return spec_data_->freq_for_band(l) < spec_data_->freq_for_band(r);
    }
};

// The set container sorting on frequency
class band_set : public set<string, band_lt>{};

// The map container sorting on frequency
template <class T>
class band_map : public map<string, T, band_lt>{
};
