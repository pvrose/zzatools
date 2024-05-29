#pragma once

#include "spec_data.h"

#include <string>
#include <set>
#include <map> 

using namespace std;

extern spec_data* spec_data_;

// Special comparison operator for bands
struct band_lt {
    bool operator() (string l, string r) const {
        return spec_data_->freq_for_band(l) < spec_data_->freq_for_band(r);
    }
};

class band_set : public set<string, band_lt>{};

template <class T>
class band_map : public map<string, T, band_lt>{
};
