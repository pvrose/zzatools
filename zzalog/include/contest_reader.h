#pragma once

#include "xml_wreader.h"

struct ct_data_t;
struct ct_date_t;
struct ct_exch_t;

class contest_data;

//! XML element identifiers for contest.xml
enum ct_element_t : char {
    CT_NONE,                //!< Not yet reading an element.
    CT_CONTESTS,            //!< Outermost element.
    CT_CONTEST,             //!< Individual contest definition.
    CT_TIMEFRAME,           //!< Timeframe.
    CT_ALGORITHM,           //!< Individual fields.
};

//! Class to process the XML file contest.XML.

/*! XML format:
* \code
*   <CONTESTS>
*     <CONTEST id=[CONTEST_ID] index=[n]>
*       <TIMEFRAME start=[start time] finish=[finish time] />
*       <ALGORITHM name=[algo id] />
*     </CONTEST>
*     <CONTEST....
*   </CONTESTS>
* \endcode
*/
class contest_reader :
    public xml_wreader
{
public:
    //! Constructor.
    contest_reader();
    //! Desctructor.
    ~contest_reader();

    //! Load data from input stream \p is to internal data \p d.
    bool load_data(contest_data* d, std::istream& is);

protected:
    //! Start "<CONTESTS>" element.
    static bool start_contests(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start "<CONTEST id= index=>" element.
    static bool start_contest(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start "<TIMEFRAME start= finish=>" elememnt.
    static bool start_timeframe(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start "<ALGORITHM name=>" element.
    static bool start_algorithm(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! End "</CONTESTS>" element.
    static bool end_contests(xml_wreader* that);
    //! End "</CONTEST>" element.
    static bool end_contest(xml_wreader* that);

    //! Maps the std::string to the element identifier for xml_wreader.
    const std::map<std::string, char> element_map_ = {
        { "CONTESTS", CT_CONTESTS },
        { "CONTEST", CT_CONTEST },
        { "TIMEFRAME", CT_TIMEFRAME },
        { "ALGORITHM", CT_ALGORITHM },
    };

    //! Map the element identifier to handler methods.
    const std::map<char, methods> method_map_ = {
        { CT_CONTESTS, { start_contests, end_contests, nullptr } },
        { CT_CONTEST, { start_contest, end_contest, nullptr } },
        { CT_TIMEFRAME, { start_timeframe, nullptr, nullptr }},
        { CT_ALGORITHM, { start_algorithm, nullptr, nullptr }},
    };

    //! The internal contest database
    contest_data* the_data_;

    //! Contest data beiing loaded.
    ct_data_t* contest_data_;
    //! Contest identifier being loaded.
    std::string contest_id_;
    //! Contest instance identifier being loaded.
    std::string contest_ix_;
    //! Data input stream.
    std::istream* in_file_;
};

