#pragma once

#include <fstream>
#include <istream>
#include <map>
#include <ostream>
#include <string>

//! Reset configuration files
extern uint16_t DEBUG_RESET_CONFIG;
const uint16_t DEBUG_RESET_ADIF = 1;        //!< Reset all.json (ADIF)
const uint16_t DEBUG_RESET_BAND = 1 << 1;   //!< Reset band_plan.json
const uint16_t DEBUG_RESET_CTY = 1 << 2;    //!< Reset cty.json
const uint16_t DEBUG_RESET_INTL = 1 << 3;   //!< Reset intl_chars.txt
const uint16_t DEBUG_RESET_APPS = 1 << 4;   //!< Reset apps.json
const uint16_t DEBUG_RESET_SETT = 1 << 5;   //!< Reset settings.json
const uint16_t DEBUG_RESET_RIGS = 1 << 6;   //!< Reset rigs.json
const uint16_t DEBUG_RESET_FLDS = 1 << 7;   //!< Reset fields.json
const uint16_t DEBUG_RESET_TEST = 1 << 8;   //!< Reset contests.json
const uint16_t DEBUG_RESET_ICON = 1 << 9;   //!< Reset Icons
const uint16_t DEBUG_RESET_STN = 1 << 10;   //!< Reset station.json
const uint16_t DEBUG_RESET_CTY1 = 1 << 11;    //!< Reset cty.xml
const uint16_t DEBUG_RESET_CTY2 = 1 << 12;    //!< Reset cty.csv
const uint16_t DEBUG_RESET_CTY3 = 1 << 13;    //!< Reset prefix.lst
const uint16_t DEBUG_RESET_ALL = 0xffff;    //!< Reset all
const uint16_t DEBUG_RESET_CALL =
	DEBUG_RESET_CTY |
	DEBUG_RESET_CTY1 |
	DEBUG_RESET_CTY2 |
	DEBUG_RESET_CTY3;                       //!< Reset all country files

//! File control structore
struct file_control_t {
	std::string filename;                   //!< Filename
	bool reference;                         //!< file is reference - from source
	bool read_only;                         //!< Not written - do not copy from source
	uint16_t reset_mask;                    //!< Reset mask
	bool fatal{ true };                     //!< Fatal error if not present
};

//! File contents
enum file_contents_t : uint8_t {
	FILE_ADIF,                              //!< ADIF Specification
	FILE_BANDPLAN,                          //!< Band-plan data
	FILE_COUNTRY_CLUB,                      //!< Country data from Clublog.org
	FILE_COUNTRY_CFILES,                    //!< Country data from country-files.com
	FILE_COUNTRY_DXATLAS,                   //!< Country data from DxAtlas
	FILE_COUNTRY,                           //!< Collated country data
	FILE_INTLCHARS,                         //!< International chatacter set
	FILE_ICON_GMAPS,                        //!< Icon for google maps
	FILE_ICON_PDF,                          //!< Icon for PDF
	FILE_ICON_QRZ,                          //!< Icon for QRZ.com
	FILE_APPS,                              //!< Application configuration file
	FILE_SETTINGS,                          //!< ZZALOG configuration file
	FILE_RIGS,                              //!< Rig configuration file
	FILE_FIELDS,                            //!< Field usage configuration file
	FILE_CONTEST,                           //!< Contests configuration file
	FILE_SOLAR,                             //!< Solar data (read every hour at most frequent
	FILE_STATUS,                            //!< Status log file
	FILE_STATION,                           //!< Station configuration file
};

//! File control datra
const std::map < file_contents_t, file_control_t > FILE_CONTROL = {
	// ID, { filename, reference, read-only
	{ FILE_ADIF, { "all.json", true, true, DEBUG_RESET_ADIF } },
	{ FILE_BANDPLAN, { "band_plan.json", true, false, DEBUG_RESET_BAND } },
	{ FILE_COUNTRY_CLUB, { "cty.xml", true, false, DEBUG_RESET_CTY } },
	{ FILE_COUNTRY_CFILES, { "cty.csv", true, false, DEBUG_RESET_CTY1 }},
	{ FILE_COUNTRY_DXATLAS, { "Prefix.lst", true, false, DEBUG_RESET_CTY2 }},
	{ FILE_COUNTRY, { "cty.json", true, false, DEBUG_RESET_CTY3, false } },
	{ FILE_INTLCHARS, { "intl_chars.txt", true, false, DEBUG_RESET_INTL }},
	{ FILE_ICON_GMAPS, { "google_maps.png", true, true, DEBUG_RESET_ICON }},
	{ FILE_ICON_PDF, { "pdf.png", true, true, DEBUG_RESET_ICON}},
	{ FILE_ICON_QRZ, { "qrz_1.jpg", true, true, DEBUG_RESET_ICON}},
	{ FILE_APPS, { "apps.json", false, false, DEBUG_RESET_APPS}},
	{ FILE_SETTINGS, { "ZZALOG.json", false, false, DEBUG_RESET_SETT }},
	{ FILE_RIGS, { "rigs.json", false, false, DEBUG_RESET_RIGS }},
	{ FILE_FIELDS, { "fields.json", false, false, DEBUG_RESET_FLDS}},
	{ FILE_CONTEST, { "contests.json", false, false, DEBUG_RESET_TEST }},
	{ FILE_SOLAR, { "solar.xml", false, false, 0}},
	{ FILE_STATUS, { "status.txt", false, false, 0}},
	{ FILE_STATION, { "station.json", false, false, DEBUG_RESET_STN }}
};

//! Data type for getting directory 
enum file_data_t : uint8_t {
	DATA_WORKING,               //!< Working directory
    DATA_SOURCE,                //!< Original source directory
	DATA_CODEGEN,               //!< Directory for generated code
	DATA_HTML,                  //!< Directory for HTML & PDF (userguide and code docs)
};

//! Holder for file naming
class file_holder
{
public:
	//! Constructor
	file_holder(bool development, std::string directory);
	//! Destructor
	~file_holder() {};

	//! Get file for input

	//! \param type File contents
	//! \param is Returned input stream
	//! \param filename Returns name of opened file
	//! \returns true if successful
	bool get_file(file_contents_t type, std::ifstream& is, std::string& filename);

	//! Get file for output

	//! \param type File contents
	//! \param os Returned input stream
	//! \param filename Returns name of opened file
	//! \returns true if successful
	bool get_file(file_contents_t type, std::ofstream& os, std::string& filename);

	//! Get filename for data \p type
	std::string get_filename(file_contents_t type) {
		const file_control_t ctrl = FILE_CONTROL.at(type);
		return default_data_directory_ + ctrl.filename;
	}

	//! Get directory for data \p type
	std::string get_directory(file_data_t type) {
		switch (type) {
		case DATA_WORKING: return default_data_directory_;
		case DATA_SOURCE: return default_source_directory_;
		case DATA_CODEGEN: return default_code_directory_;
		case DATA_HTML: return default_html_directory_;
		default: return "";
		}
	}

protected:

	//! Copy source to working
	bool copy_source_to_working(file_control_t ctrl);
	

	//! Default location for configuration files, and HTML files
	std::string default_data_directory_;

	//! Default location for reference source data
	std::string default_source_directory_;

	//! Default location for auto-generating compile fodder
	std::string default_code_directory_;

	//! Default directory for HTML files
	std::string default_html_directory_;


};

extern file_holder* file_holder_;
