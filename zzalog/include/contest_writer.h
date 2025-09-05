#pragma once

#include "xml_writer.h"

#include <chrono>
#include <string>

using namespace std;

enum ct_element_t : char;
struct ct_data_t;
struct ct_date_t;
struct ct_exch_t;

class contest_data;

//! Class that writes the contest database to contest.xml.
class contest_writer :
    public xml_writer
{
public:
    //! Constructor.
    contest_writer();
    //! Destructor.
    ~contest_writer();

    //! Write data \p d to output stream \p os.
    bool store_data(contest_data* d, ostream& os);
    //! Write the single XML \p element.
    bool write_element(ct_element_t element);

protected:
    //! write an individual "<VALUE [nane]=[data]>" for string \p data. 
    bool write_value(string name, string data);
    //! Write an individual "<VALUE [name]=[data]>" for integer \p data.
    bool write_value(string name, int data);

    //! The contest database.
    contest_data* data_;
    //! The contest data being currently written.
    ct_data_t* contest_;
    //! The identifier of the contest being written.
    string contest_id_;
    //! The instance identifier of the contest being written.
    string contest_ix_;

};

