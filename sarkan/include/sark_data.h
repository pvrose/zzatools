#pragma once

#include <vector>
#include <cinttypes>

using namespace std;

const double Z0 = 50.0;

class sark_data {

public:
    // Raw data read from analyser
    struct raw_data {
        int64_t Vf;
        int64_t Vr;
        int64_t Vz;
        int64_t Va;
    };

    // Computed data 
    struct comp_data {
        double SWR;
        double R;
        double Z;
        double X;
    };

    // Scan params
    struct scan_params {
        int64_t start;
        int64_t end;
        int64_t step;
    };

    sark_data(scan_params params);
    ~sark_data();

protected:
    vector<raw_data> readings_;
    vector<comp_data> data_;
    scan_params params_;
    vector<int64_t> frequencies_;

    size_t read_index_;

public:

    void add_reading(raw_data reading);

    // Get frequency at the reading point
    int64_t get_frequency(size_t index);
    // Get the data at the reading point
    comp_data get_data(size_t index);
    // get the parameters
    scan_params get_params();
    // Get the maximum index
    size_t size();
    // Get the current index
    size_t index();

};