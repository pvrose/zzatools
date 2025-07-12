#pragma once

#include <map>
#include <set>
#include <string>

using namespace std;

class contest_algorithm;
class contest_scorer;
class record;

struct qth_info_t;

struct score_result {
    int qso_points;
    int multiplier;
};

class contest_algorithm {

protected:
    contest_scorer* scorer_;

public:
    contest_algorithm();
    ~contest_algorithm();

    void attach(contest_scorer* cs);

    // Algorithm specific method to split text into a number of fields
    virtual void parse_exchange(record* qso, string text) = 0;
    // Algorithm specific method to generate text from a number of fieds
    virtual string generate_exchange(record* qso) = 0;
    // Algorithm specific method to score an individual QSO
    virtual score_result score_qso(record* qso, const set<string> &multipliers) = 0;

protected:

    // Default
    void set_default_rst(record* qso);
    // Update scorer widgts
    void update_rx_items(record* qso);
    void update_tx_items(record* qso);

    // List of field names to use in RX exchange and scoring
    set<string> rx_items_;
    // List of field names to use in TX exchange and scoring
    set<string> tx_items_;

    // My details
    const qth_info_t* my_info_;
};

