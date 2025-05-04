#pragma once

#include "xml_writer.h"

#include <map>
#include <string>

using namespace std;

enum rigs_element_t : char;
struct rig_data_t;
struct cat_data_t;

class rig_writer :
    public xml_writer
{
public:
    rig_writer();
    ~rig_writer();

    bool store_data(map<string, rig_data_t*>* data, ostream& os);

    bool write_element(rigs_element_t element);

protected:
    // write an individual value - string, integer and double versions
    bool write_value(string name, string data);
    bool write_value(string name, int data);
    bool write_value(string name, double data);
    // 
    string rig_name_;
    string app_name_;
    rig_data_t* rig_data_;
    cat_data_t* app_data_;

    map<string, rig_data_t*>* data_;


};