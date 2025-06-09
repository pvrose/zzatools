#include "qsl_writer.h"
#include "qsl_reader.h"
#include "qrz_handler.h"
#include "qsl_dataset.h"
#include "status.h"

extern status* status_;
extern string PROGRAM_VERSION;
extern uint32_t seed_;

qsl_writer::qsl_writer() {
    // Create the seed if it isn't already available
    if (seed_ == 0) {
        seed_ = time(nullptr);
    }
}

qsl_writer::~qsl_writer() {
}

bool qsl_writer::store_data(
    map<qsl_data::qsl_type, map<string, qsl_data*>* >* all_data, 
    map<string, server_data_t*>* servers, ostream& os) {
    data_ = all_data;
    servers_ = servers;
    status_->misc_status(ST_NOTE, "QSL: Starting XML generation");
   
 
    if (!write_element(QSL_NONE)) {
        status_->misc_status(ST_ERROR, "QSL: XML generation failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "QSL: XML Generation succeeded, now writing");
    if (!data(os)) {
        status_->misc_status(ST_ERROR, "QSL: XML Writing failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "QSL: XML Saved OK");
    return true;
   
}

bool qsl_writer::write_element(qsl_element_t element) {
    string name;
    string data;
    map<string, string>* attributes;
    char text[32];
    switch(element) {
        case QSL_NONE:
        // Prolog
        name = "xml";
        data = "version=\"1.0\" encoding=\"utf-8\" ";
        if (!process_instr(name, data)) return false;
        // else
        if (!write_element(QSL_QSL_DATA)) return false;
        return true;
    case QSL_QSL_DATA:
        name = "qsl_data";
        attributes = new map<string, string>;
        (*attributes)["version"] = PROGRAM_VERSION;
        if (!start_element(name, attributes)) return false;
        if (!write_element(QSL_QSLS)) return false;
        if (!write_element(QSL_SERVERS)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_QSLS:
        name = "qsls";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        // else
        for (auto itt : *data_) {
            type_ = itt.first;
            for (auto itc : *itt.second) {
                callsign_ = itc.first;
                current_ = itc.second;
                if (callsign_ != "") {
                    if (!write_element(QSL_QSL)) return false;
                }
            }
        }
        if (!end_element(name)) return false;
        return true;
    case QSL_QSL:
        name = "qsl";
        attributes = new map<string, string>;
        (*attributes)["call"] = callsign_;
        switch(type_) {
        case qsl_data::LABEL:
            (*attributes)["type"] = "label";
            break;
        case qsl_data::FILE:
            (*attributes)["type"] = "file";
            break;
        default:
            break;
        }
        if (!start_element(name, attributes)) return false;
        // Now the components
        if(!write_element(QSL_SIZE)) return false;
        if(!write_element(QSL_ARRAY)) return false;
        if(!write_element(QSL_FORMATS)) return false;
        if(!write_element(QSL_DESIGN)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_SIZE:
        name = "size";
        attributes = new map<string, string>;
        switch(current_->unit) {
        case qsl_data::INCH:
            (*attributes)["unit"] = "inch";
            break;
        case qsl_data::MILLIMETER:
            (*attributes)["unit"] = "mm";
            break;
        case qsl_data::POINT:
            (*attributes)["unit"] = "point";
            break;
        default:   
            break;
        }
        snprintf(text, sizeof(text), "%g", current_->width);
        (*attributes)["width"] = text;
        snprintf(text, sizeof(text), "%g", current_->height);
        (*attributes)["height"] = text;
        if (!start_element(name, attributes)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_ARRAY:
        name = "array";
        attributes = new map<string, string>;
        snprintf(text, sizeof(text), "%d", current_->rows);
        (*attributes)["rows"] = text;
        snprintf(text, sizeof(text), "%d", current_->columns);
        (*attributes)["columns"] = text;
        snprintf(text, sizeof(text), "%g", current_->col_left);
        (*attributes)["x_offset"] = text;
        snprintf(text, sizeof(text), "%g", current_->row_top);
        (*attributes)["y_offset"] = text;
        snprintf(text, sizeof(text), "%g", current_->col_width);
        (*attributes)["x_delta"] = text;
        snprintf(text, sizeof(text), "%g", current_->row_height);
        (*attributes)["y_delta"] = text;
        if (!start_element(name, attributes)) return false;
        if (!end_element(name)) return false;
        return true;   
    case QSL_FORMATS:
        name = "formats";
        attributes = new map<string, string>;
        switch(current_->f_date) {
            case qsl_data::FMT_Y4MD_ADIF:
                (*attributes)["date"] = now(false, "%Y%m%d");
                break;
            case qsl_data::FMT_Y4MD:
                (*attributes)["date"] = now(false, "%Y/%m/%d");
                break;
            case qsl_data::FMT_Y2MD:
                (*attributes)["date"] = now(false, "%y/%m/%d");
                break;
            default:
                break;
        }
        switch(current_->f_time) {
            case qsl_data::FMT_HMS_ADIF:
                (*attributes)["time"] = now(false, "%H%M%S");
                break;
            case qsl_data::FMT_HMS:
                (*attributes)["time"] = now(false, "%H:%M:%S");
                break;
            case qsl_data::FMT_HM_ADIF:
                (*attributes)["time"] = now(false, "%H%M");
                break;
            case qsl_data::FMT_HM:
                (*attributes)["time"] = now(false, "%H:%M");
                break;
            default:
                break;
        }
        snprintf(text, sizeof(text), "%d", current_->max_qsos);
        (*attributes)["max_qsos"] = text;
        if (!start_element(name, attributes)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_DESIGN:
        name = "design";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        for (auto it : current_->items) {
            item_ = it;
            switch(item_->type) {
            case qsl_data::TEXT:
                if (!write_element(QSL_TEXT)) return false;
                break;
            case qsl_data::FIELD:
                if (!write_element(QSL_FIELD)) return false;
                break;
            case qsl_data::IMAGE:
               if (!write_element(QSL_IMAGE)) return false;
                break;
            default:
                break;
            }
        }
        if (!end_element(name)) return false;
        return true;
    case QSL_TEXT:
        name = "text";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        if (!write_element(QSL_POSITION)) return false;
        if (!write_element(QSL_DATA)) return false;
        if (!write_element(QSL_OPTIONS)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_POSITION:
        name = "position";
        attributes = new map<string, string>;
        switch(item_->type) {
        case qsl_data::TEXT:
            snprintf(text, sizeof(text), "%d", item_->text.dx);
            (*attributes)["x"] = text;
            snprintf(text, sizeof(text), "%d", item_->text.dy);
            (*attributes)["y"] = text;
            break;
        case qsl_data::FIELD:
            snprintf(text, sizeof(text), "%d", item_->field.dx);
            (*attributes)["x"] = text;
            snprintf(text, sizeof(text), "%d", item_->field.dy);
            (*attributes)["y"] = text;
       break;
        case qsl_data::IMAGE:
            snprintf(text, sizeof(text), "%d", item_->image.dx);
            (*attributes)["x"] = text;
            snprintf(text, sizeof(text), "%d", item_->image.dy);
            (*attributes)["y"] = text;
       break;
        default:
            break;
        }
        if (!start_element(name, attributes)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_DATA:
        name = "data";
        attributes = new map<string, string>;
        switch(item_->type) {
        case qsl_data::TEXT:
            (*attributes)["font"] = font2text(item_->text.t_style.font);
            snprintf(text, sizeof(text), "%d", item_->text.t_style.size);
            (*attributes)["size"] = text;
            snprintf(text, sizeof(text), "%d", item_->text.t_style.colour);
            (*attributes)["colour"] = text;
            if(!start_element(name, attributes)) return false;
            if(!characters(item_->text.text)) return false;
            break;
        case qsl_data::FIELD:
            (*attributes)["font"] = font2text(item_->field.t_style.font);
            snprintf(text, sizeof(text), "%d", item_->field.t_style.size);
            (*attributes)["size"] = text;
            snprintf(text, sizeof(text), "%d", item_->field.t_style.colour);
            (*attributes)["colour"] = text;
            if(!start_element(name, attributes)) return false;
            if(!characters(item_->field.field)) return false;
            break;
        default: 
            break;
        }
        if (!end_element(name)) return false;
        return true;
    case QSL_LABEL:
        name = "label";
        attributes = new map<string, string>;
        switch(item_->type) {
        case qsl_data::FIELD:
            (*attributes)["font"] = font2text(item_->field.l_style.font);
            snprintf(text, sizeof(text), "%d", item_->field.l_style.size);
            (*attributes)["size"] = text;
            snprintf(text, sizeof(text), "%d", item_->field.l_style.colour);
            (*attributes)["colour"] = text;
            if(!start_element(name, attributes)) return false;
            if(!characters(item_->field.label)) return false;
            break;
        default:
            break;
        }
        if (!end_element(name)) return false;
        return true;
    case QSL_OPTIONS:
        name = "options";
        attributes = new map<string, string>;
        switch(item_->type) {
        case qsl_data::FIELD:
            (*attributes)["vertical"] = item_->field.vertical ? "yes" : "no";            
            (*attributes)["boxed"] = item_->field.box ? "yes" : "no";            
            (*attributes)["multi_qso"] = item_->field.multi_qso ? "yes" : "no";            
            (*attributes)["always"] = item_->field.display_empty ? "yes" : "no";            
            break;
        case qsl_data::TEXT:
            (*attributes)["vertical"] = item_->text.vertical ? "yes" : "no";
            break;
        default:
            break;
        }
        if (!start_element(name, attributes)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_FIELD:
        name = "field";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        if (!write_element(QSL_POSITION)) return false;
        if (!write_element(QSL_DATA)) return false;
        if (!write_element(QSL_LABEL)) return false; 
        if (!write_element(QSL_OPTIONS)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_IMAGE:
        name = "image";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        if (!write_element(QSL_POSITION)) return false;
        if (!write_element(QSL_FILE)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_FILE:
        name = "file";
        attributes = nullptr;
        if (!start_element(name, attributes)) return false;
        if (!characters(item_->image.filename)) return false;
        if (!end_element(name)) return false;
        return true;
    case QSL_SERVERS:
        name = "servers";
        attributes = new map<string, string>;
        snprintf(text, sizeof(text), "%d", seed_);
        (*attributes)["seed"] = text;
        if (!start_element(name, attributes)) return false;
        for (auto it : *servers_) {
            server_name_ = it.first;
            server_ = it.second;
            offset_ = hash8(server_name_.c_str());
            if (!write_element(QSL_SERVER)) return false;
        }
        if (!end_element(name)) return false;
        return true;
    case QSL_SERVER:
        name = "server";
        attributes = new map<string, string>;
        (*attributes)["name"] = server_name_;
        if (!start_element(name, attributes)) return false;
        if (!write_value("User", server_->user)) return false;
        if (!write_value("Password", encrypt(server_->password, offset_))) return false;
        if (server_name_ != "eMail") { 
            if (!write_value("Enable", server_->enabled)) return false;
            if (!write_value("Upload per QSO", server_->upload_per_qso)) return false;
             if (!write_value("Last Download", server_->last_downloaded)) return false;
        }
        if (server_name_ == "eqSL") {
            if (!write_value("Download Confirmed", server_->download_confirmed)) return false;
            if (!write_value("QSO Message", server_->qso_message)) return false;
            if (!write_value("SWL Message", server_->swl_message)) return false;
        }
        if (server_name_ == "LotW") {
            if (!write_value("Export File", server_->export_file)) return false;
        } 
        if (server_name_ == "QRZ") {
            if (!write_value("Use XML Database", server_->use_xml)) return false;
            if (!write_value("Use API", server_->use_api)) return false;
            for (auto it : server_->api_data) {
                logbook_name_ = it.first;
                api_data_ = it.second;
                if (!write_element(QSL_LOGBOOK)) return false;
            }
        }
        if (server_name_ == "eMail") {
            if (!write_value("eMail Server", server_->mail_server)) return false;
            if (!write_value("cc Address", server_->cc_address)) return false;
        }
        if (!end_element(name)) return false;
        return true;
    case QSL_LOGBOOK:
        name = "logbook";
        attributes = new map<string, string>;
        (*attributes)["call"] = logbook_name_;
        if (!start_element(name, attributes)) return false;
        if (!write_value("In Use", api_data_->used)) return false;
        if (!write_value("Last Download", api_data_->last_download)) return false;
        if (!write_value("Key", encrypt(api_data_->key, offset_))) return false;
        if (!end_element(name)) return false;
        return true;
    default:
        return false;
    }
    return false;
}

string qsl_writer::font2text(Fl_Font f) {
    string s;
    // Get the base font
    Fl_Font base = (int)f & -4;
    switch(base) {
    case FL_COURIER:
        s = "courier";
        break;
    case FL_HELVETICA:
        s = "helvetica";
        break;
    case FL_TIMES:
        s = "times";
        break;
    default:
        char msg[128];
        snprintf(msg, sizeof(msg), "QSL: Unsupported font %d: %s", f, Fl::get_font_name(f));
        status_->misc_status(ST_WARNING, msg);
        s = Fl::get_font_name(f);
        break;
    }
    if (f & FL_ITALIC) s += " italic";
    if (f & FL_BOLD) s += " bold";
    return s;
}

bool qsl_writer::write_value(string name, string data) {
    map<string, string>* attributes = new map<string, string>;
    (*attributes)["name"] = name;
    if (!start_element("value", attributes)) return false;
    if (!characters(data)) return false;
    if (!end_element("value")) return false;
    return true;
}

bool qsl_writer::write_value(string name, int data) {
    return write_value(name, to_string(data));
}

bool qsl_writer::write_value(string name, double data) {
    char text[32];
    snprintf(text, sizeof(text), "%g", data);
    return write_value(name, string(text));
}

bool qsl_writer::write_value(string name, bool data) {
    if (data) {
        return write_value(name, string("yes"));
    } else {
        return write_value(name, string("no"));
    }
}

// Encrypt data and convert to hex digits
string qsl_writer::encrypt(string s, uchar off) {
    return string_to_hex(xor_crypt(s, seed_, off));
}
