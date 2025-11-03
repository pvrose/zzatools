#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

class contest_algorithm;
class contest_scorer;
class record;

struct qth_info_t;
// List of fields
typedef std::vector <std::string> field_list;

extern std::map<std::string, contest_algorithm*>* algorithms_;

//! Basic contest scoring element
struct score_result {
    //! Points scored from the QSO
    int qso_points;
    //! Additional multiplier caused by QSO
    int multiplier;
};

//! This class is the base class for contest algorithms. 
/*!
  It should be possible to overload the methods to allow
  the scoring of each QSO and handling of the received
  and transmitted reports.
*/
class contest_algorithm {

protected:
    //! Pointer to the pane within the dashboard showing the running score.
    contest_scorer* scorer_;

public:
    //! Constructor.
    
    //! This should initialise the attributes rx_fields and tx_fields.
    //! This should add itself to the global attribute algorithms_.
    contest_algorithm();
    //! Destructor.
    ~contest_algorithm();

    //! This tells the algorithm the dashboard pane.
    void attach(contest_scorer* cs);

    //! Algorithm specific method to split text into a number of fields.
   
    //! This method takes the received report and updates the 
    //! supplied record with information.
    //! \param qso QSO record to update.
    //! \param text The received contest exchange.
    virtual void parse_exchange(record* qso, std::string text) = 0;
    //! Algorithm specific method to generate text from a number of fields.
    
    //! This method generates the report to be sent from information in the QSO record
    //! \param qso QSO record to provide exchange.
    //! \result The exchange to be sent.
    virtual std::string generate_exchange(record* qso) = 0;
    //! Algorithm specific method to score an individual QSO.
    
    //! This method calculates how many points the QSO scores and
    //! whether it provides a multiplier. 
    //! \param qso QSO record to score
    //! \param multipliers Set of multipliers currently worked: the method should
    //! update this if this is a new multiplier.
    //! \result The points scored by the QSO and the number of multipliers added.
    virtual score_result score_qso(record* qso, std::set<std::string> &multipliers) = 0;
    //! Algorithm uses serial number.
    
    //! \result Return true if the contest requires a serial number. Return false if it does not.
    virtual bool uses_serno() = 0;

    //! Return all fields used in algorithm.
    
    //! \result This method should return a std::vector<std::string> comprising all the
    //! fields referenced by the algorithm.
    field_list fields();

protected:

    //! Set the default RS(T) value required by the algorithm.
    
    //! The default RS(T) dependes on the mode - whether it is 59 or 599
    //! \param qso the record to update with default RS(T)
    void set_default_rst(record* qso);

    //! List of field names to use in RX exchange and scoring
    field_list rx_items_;
    //! List of field names to use in TX exchange and scoring
    field_list tx_items_;

    //! Information required to generate exchange to send.
    
    //! This data includes data such as Locator or IOTA reference.
    const qth_info_t* my_info_;
};

