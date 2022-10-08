#pragma once

#include "QBS_data.h"
#include "QBS_reader.h"
#include "../zzalib/xml_writer.h"

using namespace zzalib;

class QBS_writer :
    public xml_writer
{

public:
    QBS_writer();
    ~QBS_writer();
    // Generate XML for the records in the database and write it to the stream
    bool store_data(QBS_data* data, ostream& os);
    // Generate the XML elemnt code
    bool write_element(QBS_reader::qbs_element_t element);

protected:
    // The data being written
    QBS_data* the_data_;

    bool write_item(string name, string value);

    bool write_batches();

    string callsign_;
    string batch_id_;
    string value_;
    string item_name_;
    item_type_t type_;

    //QBS_data::countsets_t* b_record_;
    //string b_index_;
    //QBS_data::countset_t* b_counts_;
    QBS_data::batch_data_t* b_times_;

    map<direction_t, string> dir_names_;

};


