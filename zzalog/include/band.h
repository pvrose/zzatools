#pragma once

#include "spec_data.h"

#include <string>
#include <set>
#include <map> 
#include <cmath>

using namespace std;

extern spec_data* spec_data_;

//! Structure to provide a special means of sorting bands known by their
//! wavelength to be sorted on their frequency.
struct band_lt {
    //! Special &lt; operator.
    //! \param l LHS of operation. A string containing wavelength as eg. 10M (28~29.7 MHz).
    //! \param r RHS of operation. A string containing wavelength.
    //! \return frequency of LHS is less than frequency of RHS.
    bool operator() (string l, string r) const {
        // If the supploed string is not a real band name - return assume 0.0Hz
        double lv = spec_data_->freq_for_band(l);
        double rv = spec_data_->freq_for_band(r);
        if (isnan(lv)) lv = 0.0;
        if (isnan(rv)) rv = 0.0;
        return lv < rv;
    }
};

//! A version of set<string> sorting on the frequency represented by the string value.
class band_set : public set<string, band_lt>{};

//! A version of map<string, T> sorting on the frequency represented by the string value.
template <class T>
class band_map : public map<string, T, band_lt>{
};
