#include "contest_writer.h"

#include "contest_data.h"
#include "contest_reader.h"
#include "status.h"

extern status* status_;

contest_writer::contest_writer() {
	data_ = nullptr;
}

contest_writer::~contest_writer() {

}

bool contest_writer::store_data(contest_data* d, ostream& os) {
	data_ = d;
    status_->misc_status(ST_NOTE, "CONTEST: Starting XML generation");

    if (!write_element(CT_NONE)) {
        status_->misc_status(ST_ERROR, "CONTEST: XML generation failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "CONTEST: XML Generation succeeded, now writing");
    if (!data(os)) {
        status_->misc_status(ST_ERROR, "CONTEST: XML Writing failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "CONTEST: XML Saved OK");
    return true;

}

bool contest_writer::write_element(ct_element_t element) {
    string name;
    string data;
    map<string, string>* attributes;
    switch (element) {
    case CT_NONE:
        // Prolog
        name = "xml";
        data = "version=\"1.0\" encoding=\"utf-8\" ";
        if (!process_instr(name, data)) return false;
        // else
        if (!write_element(CT_CONTESTS)) return false;
        return true;
    case CT_CONTESTS:
        name = "contests";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        // else
        for (auto it : data_->contests_) {
            contest_id_ = it.first;
            for (auto iu : it.second) {
                contest_ix_ = iu.first;
                contest_ = iu.second;
                if (!write_element(CT_CONTEST)) return false;
            }
        }
        if (!end_element(name)) return false;
        return true;
    case CT_CONTEST:
        name = "contest";
        attributes = new map<string, string>;
        (*attributes)["id"] = contest_id_;
        (*attributes)["index"] = contest_ix_;
        if (!start_element(name, attributes)) return false;
        if (!write_value("fields", contest_->fields)) return false;
        if (!write_element(CT_TIMEFRAME)) return false;
        if (!end_element(name)) return false;
        return true;
    case CT_TIMEFRAME:
        name = "timeframe";
        attributes = new map<string, string>;
        (*attributes)["start"] = convert_xml_datetime(chrono::system_clock::to_time_t(contest_->date.start));
        (*attributes)["finish"] = convert_xml_datetime(chrono::system_clock::to_time_t(contest_->date.finish));
        if (!start_element(name, attributes)) return false;
        if (!end_element(name)) return false;
        return true;
     default:
        status_->misc_status(ST_ERROR, "RIG DATA: Unsupported XML element");
        return false;
    }
}

// write an individual value - string, integer and double versions
bool contest_writer::write_value(string name, string data) {
    map<string, string>* attributes = new map<string, string>;
    (*attributes)["name"] = name;
    if (!start_element("value", attributes)) return false;
    if (!characters(data)) return false;
    if (!end_element("value")) return false;
    return true;
}

bool contest_writer::write_value(string name, int data) {
    return write_value(name, to_string(data));
}


