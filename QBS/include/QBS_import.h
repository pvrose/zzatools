#pragma once
/***********************************************************************
*  QBS_import
* Provides a means of importing spreadsheet .CSV files into the database
*  QBS_column_table
* Widget displaying mapping between CSV column and DB field
*/
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <set>

#include "QBS_data.h"

using namespace std;

class QBS_import 
{

public:

    QBS_import();
    virtual ~QBS_import();

    void clear_maps();

    bool load_data(QBS_data* data, const char* directory);

protected:

    // Read batch data
    bool read_batches();
    // Read card data
    bool read_card_data();
    // Read SASE data
    bool read_sase_data();
    // Read single batch
    bool read_batch(int box, string line);
    bool read_call(bool card, string line);

    // Copy internal data to QBS_data
    bool copy_data();

    // Directory name
    string directory_;
    // data
    QBS_data* data_;

    // Holding matrix - 3D: callsign x batch x direction
    // direction - fixed 4 entries
    // batches - defined after loading batches
    // callsign - dynamix increasing while loading cards or sases
    enum direction_t : char {
        RECEIVED = 0,
        SENT = 1,
        RECYCLED = 2,
        KEPT = 3,
        DIRN_INVALID
    };
    map<string, map<string, vector< int > > >  card_matrix_;
    map<string, map<string, vector< int > > >  sase_matrix_;
    int count(
        bool card,
        string call,
        string batch,
        direction_t dirn);

    // Holding batch data - 2D: data x batch + direction
    struct event_t {
        string batch;
        direction_t in_out{ RECEIVED };
    };
    map <string, event_t > dates_;
    // Map batch names to box number - initial batch is not a box (=-1)
    map <string, int> boxes_;
    map <int, string> batch_names_;
    // Weight of cards recycled from each batch (including initial)
    map <string, float> batch_weights_;
 
    // Holding call data
    call_info calls_;

    // File inputs
    ifstream in_;
    // Current file headers
    vector<string> columns_;
};

