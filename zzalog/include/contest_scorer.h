#pragma once



#include <set>
#include <string>

#include <FL/Fl_Group.H>

class extract_data;
class record;
class Fl_Button;
class Fl_Choice;
class Fl_Output;

struct ct_data_t;
struct ct_exch_t;

// The records are kept in a container with size_t as index
typedef size_t item_num_t;    // Position of item within this book
typedef size_t qso_num_t;     // Position of item within book_ instance


using namespace std;

class contest_scorer :
    public Fl_Group
{

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
    void check_qso(qso_num_t qso_number);

    // Active
    bool contest_active();

    // Return contest ID
    string contest_id();

    // Increment next serial number
    void increment_serial();

    // Return field collection name
    string collection();

    // Return serial number as string
    string serial();
    // Return exchange
    string exchange();



protected:

    void populate_contest();

    void populate_timeframe(ct_data_t* ct);
    void populate_status(ct_data_t* ct, bool previous);

    // Load QSOs - after restarting zzalog
    void resume_contest();

    // Score QSP
    void score_qso(record* qso, bool check_only);

    // Individual algorithms
    void score_basic(record* qso, bool check_only);

    // data
    string contest_id_;
    string contest_index_;
    string start_time_;
    string finish_time_;

    ct_data_t* contest_;
    ct_exch_t* exchange_;

    // scoring algorithm
    string scoring_id_;

    // Logged vallues
    int qso_points_;
    int multiplier_;
    int total_;
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
    Fl_Output* w_qso_points_2_;
    Fl_Output* w_multiplier_2_;
    Fl_Output* w_total_2_;

};

