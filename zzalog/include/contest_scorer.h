#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include <FL/Fl_Group.H>

class contest_algorithm;
class extract_data;
class record;
class Fl_Button;
class Fl_Choice;
class Fl_Counter;
class Fl_Input;
class Fl_Output;

struct ct_data_t;



// The records are kept in a container with size_t as index
typedef size_t item_num_t;    // Position of item within this book
typedef size_t qso_num_t;     // Position of item within book_ instance
typedef std::vector<std::string> field_list;

//! This class displays the running status of scoring during a contest activation.
class contest_scorer :
    public Fl_Group
{

    friend class contest_algorithm;

public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    contest_scorer(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~contest_scorer();

    //! Override Fl_Group::handle.

    //! Intercept click event to gain focus for keyboard F1 to open userguide.
    virtual int handle(int event);

    //! Load current contest identifiers and status from settings.
    void load_data();
    //! Instantiate component widgets.
    void create_form();
    //! Save current contest identifiers and status to settings.
    void save_data();
    //! Configure component widgets after data changes.
    void enable_widgets();

    //! Callback on selecting contest choice.
    
    //! Load specific data from contest database.
    static void cb_contest(Fl_Widget* w, void* v);
    //! Callback on clicking "Status" button
    
    //! This is used to pause and restart contest activation and to complete
    //! activation after processing the contest log.
    static void cb_bn_status(Fl_Widget* w, void* v);
    //! Callback on clicking "Parse" button.
    
    //! Parses the received exchange updating the relevant QSO fields.
    static void cb_parse(Fl_Widget* w, void* v);
    //! Callback on clicking serial number counter: saves new value.
    static void cb_serno(Fl_Widget* w, void* v);

    //! Add the current QSO to the contest log.
    void add_qso(record* qso, qso_num_t qso_number);
    //! Check to see what the impact on scorng would be for this QSO.
    void check_qso(record* qso, qso_num_t qso_number);

    //! Is the contest active?
    
    //! \return true if the contest is active, false if it is not.
    bool contest_active();

    //! What is the contest?
    
    //! \return identifier of the current contest.
    std::string contest_id();

    //! What fields are being used in the contest.
    
    //! \return std::list of fields used in contest exchanges.
    field_list fields();

    //! Serial number
    
    //! \return serial number as std::string
    std::string serial();

    //! Add score from this QSO, unless \p check_only is true when it checks what it would be.
    void score_qso(record* qso, bool check_only);
    //! Parse exchange and adds data to the \p QSO record.
    void parse_exchange(record* qso, std::string text);
    //! Generate exchange to send for this \p QSO.
    std::string generate_exchange(record* qso);

    //! Contest status value.
    enum  ct_status : unsigned char {
        NO_CONTEST,           //!< Not in a contest
        FUTURE,               //!< Selected contest has not yet started.
        ACTIVE,               //!< Contest has started and is being activated.
        PAUSED,               //!< Contest has started, but not currently activated.
        PAST                  //!< Contest has finished, but still interested in the scoring.
    } contest_status_;

protected:

    //! Populate the contest identifier menu from supported contests.
    void populate_contest();
    //! Display the start and finish times of the contest.
    void populate_timeframe();
    //! Display the current status of the contest.
    void populate_status();
    //! Attach the specified algorithm code to this widget.
    void create_algo();
    //! Update all widget values to support the new contest, or defualt values if not contest selected.
    void change_contest();

    //! Load current score after restarting ZZALOG in the middle of a contest activation.
    void resume_contest();

    //! Copy points to display
    void copy_points_to_display();

    // data
    std::string contest_id_;       //!< Identifier of the current contest.
    std::string contest_index_;    //!< Identifier of the current contest instance.
    std::string start_time_;       //!< Contest start time as text.
    std::string finish_time_;      //!< Contest finish time as text.
    bool active_;             //!< Contest is active.

    //! Contest description.
    ct_data_t* contest_;

    // Logged vallues
    int qso_points_;          //!< QSO points accumulated so far.
    int multiplier_;          //!< Multiplier points accumulated so far.
    int total_;               //!< Total points so far.
    // Check QSO deltas
    int d_qso_points_;        //!< &delta; points from the current QSO.
    int d_multiplier_;        //!< &delta; multiplier from the current QSO.
    // Potential values
    int qso_points_p_;        //!< Possible QSO points including the current QSO.
    int multiplier_p_;        //!< Possible multiplier including the current QSO.
    int total_p_;             //!< Possible total including the current QSO.

    // Next serial number
    int next_serial_;         //!< Next contest serial number to send.

   
    //! QSO records specifically in this contest.
    extract_data* qsos_;
    //! Current QSO record.
    record* qso_;
    //! Index of current QSO within the full logbook.
    qso_num_t qso_number_;
    //! Scoring algorithm for the current contest.
    contest_algorithm* algorithm_;

    //! Set of multipliers worked so far in this contest.
    std::set<std::string> multipliers_;

    // Widgets
    Fl_Choice* w_contest_;       //!< Choice to select contest from those supported.
    Fl_Output* w_start_time_;    //!< Displays contest start time
    Fl_Output* w_finish_time_;   //!< Displays contest finish time.
    Fl_Button* w_status_;        //!< Displays contest status and allows some chnage in status.
    Fl_Group* g_exch_;           
    Fl_Input* w_rx_exchange_;    //!< Input for received exchange.
    Fl_Output* w_tx_exchange_;   //!< Displays required transmit exchange.
    Fl_Counter* w_next_serno_;   //!< Display for current seral number: allows modification.
    Fl_Button* w_parse_;         //!< Button to parse received exchange.
    Fl_Group* g_scores_;    
    Fl_Output* w_number_qsos_;   //!< Displays number of QSOs so far in contest.
    Fl_Output* w_qso_points_;    //!< Displays total QSO points so far.
    Fl_Output* w_multiplier_;    //!< Displays total Multiplier so far.
    Fl_Output* w_total_;         //!< Displays grand total so far.
    Fl_Output* w_qso_points_d_;  //!< Displays &delta; QSO points from this QSO.
    Fl_Output* w_multiplier_d_;  //!< Displays &delta; multiplier from this QSO.
    Fl_Output* w_qso_points_2_;  //!< Displays possible QSO points with this QSO.
    Fl_Output* w_multiplier_2_;  //!< Displays possible multiplier with this QSO.
    Fl_Output* w_total_2_;       //!< Displays possible grand total with this QSO.

};

//! JSON serialisation for contest_scorer::ct_status
NLOHMANN_JSON_SERIALIZE_ENUM(contest_scorer::ct_status, {
    { contest_scorer::NO_CONTEST, "No contest" },
    { contest_scorer::FUTURE, "Upcoming contest"},
    { contest_scorer::ACTIVE, "Working contest"},
    { contest_scorer::PAUSED, "Paused contest"},
    { contest_scorer::PAST, "After contest "}
    }
)
