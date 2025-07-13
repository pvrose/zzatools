#pragma once


#include <map>
#include <set>
#include <string>
#include <vector>

#include <FL/Fl_Group.H>

class contest_algorithm;
class extract_data;
class record;
class Fl_Button;
class Fl_Choice;
class Fl_Output;

struct ct_data_t;

using namespace std;

// The records are kept in a container with size_t as index
typedef size_t item_num_t;    // Position of item within this book
typedef size_t qso_num_t;     // Position of item within book_ instance
typedef vector<string> field_list;

const int NUMBER_FIELDS = 4;

class contest_scorer :
    public Fl_Group
{

    friend class contest_algorithm;

public:
    contest_scorer(int X, int Y, int W, int H, const char* L = nullptr);
    ~contest_scorer();

    void load_data();
    void create_form();
    void save_data();
    void enable_widgets();

    // callbacks
    static void cb_contest(Fl_Widget* w, void* v);

    static void cb_bn_status(Fl_Widget* w, void* v);

    // Add record 
    void add_qso(qso_num_t qso_number);
    // Check record
    void check_qso(record* qso);

    // Active
    bool contest_active();

    // Return contest ID
    string contest_id();

    // Increment next serial number
    void increment_serial();

    // Return list of fields
    field_list fields();

    // Return serial number as string
    string serial();

    // Score QSO
    void score_qso(record* qso, bool check_only);
    // Parse exchange
    void parse_exchange(record* qso, string text);
    // Create exchang
    string generate_exchange(record* qso);


protected:

    void populate_contest();

    void populate_timeframe();
    void populate_status();
    void create_algo();

    void change_contest();

    // Load QSOs - after restarting zzalog
    void resume_contest();

   // Copy points to display
    void copy_points_to_display();

    // data
    string contest_id_;
    string contest_index_;
    string start_time_;
    string finish_time_;
    bool active_;

    ct_data_t* contest_;

    // Logged vallues
    int qso_points_;
    int multiplier_;
    int total_;
    // Check QSO deltas
    int d_qso_points_;
    int d_multiplier_;
    // Potential values
    int qso_points_p_;
    int multiplier_p_;
    int total_p_;

    // Next serial number
    int next_serial_;

    enum  ct_status : unsigned char {
        NO_CONTEST,
        FUTURE,
        ACTIVE,
        PAUSED,
        PAST
    } contest_status_;

    // QSOs
    extract_data* qsos_;
    // Current QSO
    record* qso_;
    // Algorithm
    contest_algorithm* algorithm_;

    // Multipliers worked
    set<string> multipliers_;

    // Widgets
    Fl_Choice* w_contest_;
    Fl_Output* w_start_time_;
    Fl_Output* w_finish_time_;
    Fl_Button* w_status_;
    Fl_Group* g_scores_;
    Fl_Output* w_number_qsos_;
    Fl_Output* w_qso_points_;
    Fl_Output* w_multiplier_;
    Fl_Output* w_total_;
    Fl_Output* w_qso_points_d_;
    Fl_Output* w_multiplier_d_;
    Fl_Output* w_qso_points_2_;
    Fl_Output* w_multiplier_2_;
    Fl_Output* w_total_2_;
    Fl_Group* g_fields_;
    Fl_Output* w_rx_items_[NUMBER_FIELDS];
    Fl_Output* w_tx_items_[NUMBER_FIELDS];

};

