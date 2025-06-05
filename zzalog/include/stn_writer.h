#pragma once

#include "xml_writer.h"
#include "stn_data.h"

#include <map>
#include <string>

using namespace std;

enum stn_element_t : char;

class stn_writer :
    public xml_writer
{
public:
    stn_writer();
    ~stn_writer();

    bool store_data(
        map<string, qth_info_t*>* qths, 
        map<string, oper_info_t*>* opers,
        map<string, string>* scalls,
        ostream& os);

    bool write_element(stn_element_t element);

protected:
    // Write an individual item
    bool write_item(string name, string data);
    // data
    map<string, qth_info_t*>* qths_;
    map<string, oper_info_t*>* opers_;
    map<string, string>* scalls_;

    string qth_name_;
    string oper_name_;
    qth_info_t* qth_;
    oper_info_t* oper_;

    static map<qth_value_t, string> qth_value_map_;
    static map<oper_value_t, string> oper_value_map_;

};

