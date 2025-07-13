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

class contest_writer :
    public xml_writer
{
public:
    contest_writer();
    ~contest_writer();

    bool store_data(contest_data* d, ostream& os);

    bool write_element(ct_element_t element);

protected:
    // write an individual value - string, integer and double versions
    bool write_value(string name, string data);
    bool write_value(string name, int data);

    contest_data* data_;

    ct_data_t* contest_;
    // Current contest ID
    string contest_id_;
    // Cureent contest index
    string contest_ix_;

};

