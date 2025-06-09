#include "stn_writer.h"
#include "stn_reader.h"
#include "stn_data.h"
#include "status.h"

using namespace std;

using namespace std;

extern status* status_;
extern string PROGRAM_VERSION;

stn_writer::stn_writer() :
	qth_name_(""),
	qth_(nullptr),
	oper_name_(""),
	oper_(nullptr)
{
}

stn_writer::~stn_writer() {
}

bool stn_writer::store_data(
    map<string, qth_info_t*>* qths, 
    map<string, oper_info_t*>* opers, 
    map<string, string>* scalls,
    ostream& os) {
    qths_ = qths;
    opers_ = opers;
    scalls_ = scalls;

    status_->misc_status(ST_NOTE, "STN DATA: Starting XML generation");

    if (!write_element(STN_NONE)) {
        status_->misc_status(ST_ERROR, "STN DATA: XML generation failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "STN DATA: XML Generation succeeded, now writing");
    if (!data(os)) {
        status_->misc_status(ST_ERROR, "STN DATA: XML Writing failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "STN DATA: XML Saved OK");
    return true;
}

map<qth_value_t, string> stn_writer::qth_value_map_ = {
    { STREET, "Street" },
    { CITY, "City" },
    { POSTCODE, "Postcode" },
    { LOCATOR, "Locator" },
    { DXCC_NAME, "Country" },
    { DXCC_ID, "DXCC" },
    { PRIMARY_SUB, "Primary Subdivision" },
    { SECONDARY_SUB, "Secondary Subdivision" },
    { CQ_ZONE, "CQ Zone" },
    { ITU_ZONE, "ITU Zone" },
    { CONTINENT, "Continent" },
    { IOTA, "IOTA" },
    { WAB, "WAB" }
};

map<oper_value_t, string> stn_writer::oper_value_map_ = {
    { NAME, "Name" },
    { CALLSIGN, "Callsign" }
};

bool stn_writer::write_element(stn_element_t element) {
    string name;
    string data;
    map<string, string>* attributes = nullptr;
    switch (element) {
    case STN_NONE:
        // Prolog
        name = "xml";
        data = "version=\"1.0\" encoding=\"utf-8\" ";
        if (!process_instr(name, data)) return false;
        // else
        if (!write_element(STN_STATION)) return false;
        return true;
    case STN_STATION:
        name = "station";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        if (!write_element(STN_LOCATION)) return false;
        if (!write_element(STN_OPERATOR)) return false;
        if (!write_element(STN_SCALLSIGN)) return false;
        if (!end_element(name)) return false;
        return true;
    case STN_LOCATION:
        for (auto it : *qths_) {
            qth_name_ = it.first;
            qth_ = it.second;
            name = "location";
            attributes = new map<string, string>;
            (*attributes)["id"] = qth_name_;
            (*attributes)["description"] = qth_->description;
            if (!start_element(name, attributes)) return false;
            for (auto iu : qth_->data) {
                if (!write_item(qth_value_map_.at(iu.first), iu.second)) return false;
            }
            if (!end_element(name)) return false;
        }
        return true;
    case STN_OPERATOR:
        for (auto it : *opers_) {
            oper_name_ = it.first;
            oper_ = it.second;
            name = "operator";
            attributes = new map<string, string>;
            (*attributes)["id"] = oper_name_;
            (*attributes)["description"] = oper_->description;
            if (!start_element(name, attributes)) return false;
            for (auto iu : oper_->data) {
                if (!write_item(oper_value_map_.at(iu.first), iu.second)) return false;
            }
            if (!end_element(name)) return false;
        }
        return true;
    case STN_SCALLSIGN:
        for (auto it: *scalls_) {
            name = "scallsign";
            attributes = new map<string, string>;
            (*attributes)["call"] = it.first;
            (*attributes)["description"] = it.second;
            if (!start_element(name, attributes)) return false;
            if (!end_element(name)) return false;
        }
        return true;
    default:
        status_->misc_status(ST_ERROR, "STN DATA: Unsupported XML element");
        return false;
    }
}

// Write an individual item
bool stn_writer::write_item(string name, string data) {
    map<string, string>* attributes = new map<string, string>;
    (*attributes)["name"] = name;
    if (!start_element("item", attributes)) return false;
    if (!characters(data)) return false;
    if (!end_element("item")) return false;
    return true;

}

