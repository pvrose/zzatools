#pragma once

#include "xml_wreader.h"
#include "qsl_data.h"

#include<istream>
#include <list>



struct server_data_t;
struct qsl_call_data;


//! XML element used in qsls.xml
enum qsl_element_t : char {
    QSL_NONE,         //!< Not yet processing an element.
    QSL_QSL_DATA,     //!< Top level element QSL_DATA
    QSL_QSLS,         //!< start of element QSLS
    QSL_QSL,          //!< Individual design QSL call="GM3ZZA" type="label".
    QSL_SIZE,         //!< Size of overall image SIZE unit="mm" width="nnn" height="nnn"/.
    QSL_ARRAY,        //!< Position of images on a printing page - 6 attributes.
    QSL_DESIGN,       //!< Start of image design.
    QSL_FORMATS,      //!< Date & time formats, max number of QSOs per image.
    QSL_TEXT,         //!< Start of text item.
    QSL_POSITION,     //!< Position of text, field or image item (x=, y=).
    QSL_LABEL,        //!< Text to write.
    QSL_FIELD,        //!< Start of field item.
    QSL_DATA,         //!< Field data.
    QSL_OPTIONS,      //!< Options - vertical=y/n box=y/n multi=y/n always=y/n.
    QSL_IMAGE,        //!< Start of image item.
    QSL_FILE,         //!< Filename.
    QSL_SERVERS,      //!< Start of data for servers.
    QSL_SERVER,       //!< Start of individual server data.
    QSL_VALUE,        //!< Individual value item.
    QSL_LOGBOOK,      //!< Start of individual QRZ.com log book credentials.
};

//! This class reads XML data to load the QSL and QSL server configuration database.

//! \code
//! <QSL_DATA>
//!   <QSLS>
//!     <QSL call="[callsign] type="[label|file"]>
//!       <SIZE unit="mm|inch|point" width=[width] height=[height]/>
//!       <ARRAY rows="n" columns="n" x_offset="n" y_offset="n" x_delta="n" y_delta="n"/>
//!       <FORMATS data=".." time=".." max_qsos="nn" />
//!       <DESIGN>
//!         <TEXT>
//!           <POSITION x="nn" y="nn" />
//!           <DATA size="nn" font="nn" colour="nn"> text </DATA>
//!           <OPTIONS vertical = "y|n" boxed = "y|n" multi_qso="y|n" always="y|n" />
//!         </TEXT>
//!         :
//!         <FIELD>
//!           <POSITION x="nn" y="nn" />
//!           <LABEL size="nn" font="nn" colour="nn"> text </LABEL>
//!           <DATA size="nn" font="nn" colour="nn"> text  </DATA>
//!           <OPTIONS vertical = "y|n" boxed = "y|n" multi_qso="y|n" always="y|n" />
//!         </FIELD>
//!         :
//!         <IMAGE>
//!           <POSITION x="nn" y="nn" />
//!           <FILE> filename </FILE>
//!         </IMAGE>
//!         :
//!       </DESIGN>
//!     </QSL>
//!     :
//!   </QSLS>
//!   <SERVERS>
//!     <SERVER name="server">
//!       <VALUE name="name">value</VALUE>
//!       <LOGBOOK call="callsign">
//!         <VALUE name="name">value</VALUE>
//!       </LOGBOOK>
//!     </SERVER>
//!     :
//!   </SERVER>
//! </QSL_DATA>
//! 
//! \endcode    
class qsl_reader :
    public xml_wreader
{
public:
    //! Constructor.
    qsl_reader();
    //! Destructor.
    ~qsl_reader();

    //! Load data
    
    //! \param data Receives QSL design data.
    //! \param servers Receives QSL server credentials.
    //! \param in input data stream.
    //! \return true if successful.
    bool load_data(std::map<qsl_data::qsl_type, std::map<std::string, qsl_data*>* >* data, 
        std::map<std::string, server_data_t*>* servers,
        std::istream& in);

protected:
    // Start the specific elementys
    //! Start QSL_DATA element.
    static bool start_qsl_data(xml_wreader* w, std::map<std::string, std::string>* attributes);
    //! Start QSLS element.
    static bool start_qsls(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start QSL element.
    static bool start_qsl(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start SIZE element.
    static bool start_size(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start ARRAY element.
    static bool start_array(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start DESIGN element.
    static bool start_design(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start FORMATS element.
    static bool start_formats(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start TEXT element.
    static bool start_text(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start POSITION element.
    static bool start_position(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start LABEL element.
    static bool start_label(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start FIELD element.
    static bool start_field(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start DATA element.
    static bool start_data(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start OPTIONS element.
    static bool start_options(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start IMAGE element.
    static bool start_image(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start FILE element.
    static bool start_file(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start SERVERS element.
    static bool start_servers(xml_wreader* w, std::map<std::string, std::string>* attributes);
    //! Start SERVER element.
    static bool start_server(xml_wreader* w, std::map<std::string, std::string>* attributes);
    //! Start LOGBOOK element.
    static bool start_logbook(xml_wreader* w, std::map<std::string, std::string>* attributes);
    //! Start VALUE element.
    static bool start_value(xml_wreader* w, std::map<std::string, std::string>* attributes);

    // End the specific elements
    //! End QSL_DATA element.
    static bool end_qsl_data(xml_wreader* that);
    //! End QSL element.
    static bool end_qsl(xml_wreader* that);
    //! End VALUE element.
    static bool end_value(xml_wreader* w);

    // Specific character processing
    //! Process characters for FILE element.
    static bool chars_file(xml_wreader* that, std::string content);
    //! Process characters for LABEL element.
    static bool chars_label(xml_wreader* that, std::string content);
    //! Process characters for DATA element.
    static bool chars_data(xml_wreader* that, std::string content);
    //! Process characters for VALUE element.
    static bool chars_value(xml_wreader* w, std::string content);
    

    //! Check version
    bool check_version(std::string v);

    //! Parse date format into enumerated type
    static qsl_data::date_format parse_date(std::string s);
    //! Parse time format into enumerated type
    static qsl_data::time_format parse_time(std::string s);

    //! Convert font description to font number
    static Fl_Font parse_font(std::string s);
    //! Convert yes/no into bool
    static bool parse_bool(std::string s);
    //! Decode encrypted std::string \p s using \p offset into key-chain.
    static std::string decrypt(std::string s, uchar offset);


    //! Name to element enumration mapping
    const std::map<std::string, char> element_map_ = {
        { "QSL_DATA", QSL_QSL_DATA },
        { "QSLS", QSL_QSLS },
        { "QSL", QSL_QSL },
        { "SIZE", QSL_SIZE },
        { "ARRAY", QSL_ARRAY },
        { "DESIGN", QSL_DESIGN },
        { "FORMATS", QSL_FORMATS },
        { "TEXT", QSL_TEXT },
        { "POSITION", QSL_POSITION },
        { "LABEL", QSL_LABEL },
        { "DATA", QSL_DATA },
        { "OPTIONS", QSL_OPTIONS },
        { "FIELD", QSL_FIELD },
        { "IMAGE", QSL_IMAGE },
        { "FILE", QSL_FILE },
        { "SERVERS", QSL_SERVERS },
        { "SERVER", QSL_SERVER },
        { "LOGBOOK", QSL_LOGBOOK },
        { "VALUE", QSL_VALUE },
    };

    //! Map element type into the hander methods.
    const std::map<char, methods> method_map_ = {
        { QSL_QSL_DATA, { start_qsl_data, end_qsl_data, nullptr }},
        { QSL_QSLS, { start_qsls, nullptr, nullptr }},
        { QSL_QSL, { start_qsl, end_qsl, nullptr }},
        { QSL_SIZE, { start_size, nullptr, nullptr }},
        { QSL_ARRAY, { start_array, nullptr, nullptr }},
        { QSL_DESIGN, { start_design, nullptr, nullptr }},
        { QSL_FORMATS, { start_formats, nullptr, nullptr }},
        { QSL_TEXT, { start_text, nullptr, nullptr }},
        { QSL_POSITION, { start_position, nullptr, nullptr }},
        { QSL_LABEL, { start_label, nullptr, chars_label }},
        { QSL_FIELD, { start_field, nullptr, nullptr }},
        { QSL_DATA, { start_data, nullptr, chars_data }},
        { QSL_OPTIONS, { start_options, nullptr, nullptr }},
        { QSL_IMAGE, { start_image, nullptr, nullptr }},
        { QSL_FILE, { start_file, nullptr, chars_file }}, 
        { QSL_SERVERS, { start_servers, nullptr, nullptr }},
        { QSL_SERVER, { start_server, nullptr, nullptr }},
        { QSL_LOGBOOK, { start_logbook, nullptr, nullptr }},
        { QSL_VALUE, { start_value, end_value, chars_value }},

    };

    //! The QSL design data.
    std::map<qsl_data::qsl_type, std::map<std::string, qsl_data*>* >* data_;
    //! The QSL server credentials.
    std::map<std::string, server_data_t*>* servers_;
   // Attributes
    //! Station callsign.
    std::string callsign_;
    //! type of QSL - file for e-mail, label for printing.
    qsl_data::qsl_type type_;

    //! Current qsl_data.
    qsl_data* current_;
    //! Current item definition.
    qsl_data::item_def* item_;
    //! Current server data.
    server_data_t* server_;
    //! Current QRZ logbook api credentials.
    qsl_call_data* api_data_;
    //! Current value name.
    std::string value_name_;
    //! Current value data.
    std::string value_data_;
    //! Current encryption keychain offset.
    uchar offset_;
    //! Name of parent.
    std::string parent_name_;

};