#include "qsl_writer.h"
#include "qsl_reader.h"
#include "status.h"

extern status* status_;
extern string PROGRAM_VERSION;

qsl_writer::qsl_writer() {
}

qsl_writer::~qsl_writer() {
}

bool qsl_writer::store_data(map<qsl_data::qsl_type, map<string, qsl_data*>* >* all_data, ostream& os) {
    data_ = all_data;
    status_->misc_status(ST_NOTE, "QSL DATA: Starting XML generation");
   
 
    if (!write_element(QSL_NONE)) {
        status_->misc_status(ST_ERROR, "QSL DATA: XML generation failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "QSL DATA: XML Generation succeeded, now writing");
    if (!data(os)) {
        status_->misc_status(ST_ERROR, "QSL DATA: XML Writing failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "QSL DATA: XML Writing done");
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
        if (!write_element(QSL_QSLS)) return false;
        return true;
    case QSL_QSLS:
        name = "qsls";
        attributes = new map<string, string>;
        (*attributes)["version"] = PROGRAM_VERSION;
        if (!start_element(name, attributes)) return false;
        // else
        for (auto itt : *data_) {
            type_ = itt.first;
            for (auto itc : *itt.second) {
                callsign_ = itc.first;
                current_ = itc.second;
                if (!write_element(QSL_QSL)) return false;
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
        snprintf(msg, sizeof(msg), "QSL DATA: Unsupported font %d: %s", f, Fl::get_font_name(f));
        status_->misc_status(ST_WARNING, msg);
        s = Fl::get_font_name(f);
        break;
    }
    if (f & FL_ITALIC) s += " italic";
    if (f & FL_BOLD) s += " bold";
    return s;
}
