#pragma once

#include "qsl_data.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#include <FL/Fl.H>

enum extract_mode_t : uchar;

// Class to manage QSL designs

//! Contains data needed to access logbooks by call on QSL server sites.
struct qsl_call_data {
	bool used;                           //!< Data is used.
	std::string key;                     //!< logbook access key.
	unsigned long long last_logid;       //!< Universal QSO record identifier.
	std::string last_download;           //!< Date last downloaded (YYYYMMDD)
};

//! Contains data needed to access QSL server sites.
struct server_data_t {
	bool upload_per_qso{false};          //!< Upload per QSO
	bool enabled{false};                 //!< Access enabled
	std::string user{""};                //!< User-name (or e-mail
	std::string password{""};            //!< Password
	std::string last_downloaded{""};     //!< Date of last download
	bool download_confirmed{false};      //!< Download confirmed as well (eqSL)
	std::string qso_message{""};         //!< Message to add to a QSL card (eQSL)
	std::string swl_message{""};         //!< Message to add to a QSL card - SWL (eQSL)
	std::string export_file{""};         //!< File for uploading (LotW)
	bool use_api{false};                 //!< Use API (QRZ.com)
	bool use_xml{false};                 //!< Use XML (QRZ.com)
	std::string mail_server{""};         //!< Mail server (eMail)
	std::string cc_address{""};          //!< cc Address (eMail)
	std::map<std::string, qsl_call_data*> call_data; //!< API logbook data (QRZ.com)
};

//! The container for all data pertinant to QSL card designs and QSL server sites.
class qsl_dataset
{
public:

	//! Constructor.
	qsl_dataset();
	//! Destructor.
	~qsl_dataset();

	//! Return the QSL design associated with the \p callsign and QSL \p type
	qsl_data* get_card(std::string callsign, qsl_data::qsl_type type, bool create);
	//! Get the path to settings/Datapath/QSLs
	std::string get_path();
	//! Returns the server data associated with the \p server (per extract_mode_t)
	server_data_t* get_server_data(std::string server);
	//! Returns QRZ logbook credentials
	qsl_call_data* get_qrz_api(std::string callsign);
	//! Returns set of callsign not wanting electronic QSLs
	std::set<std::string>* get_no_qsl_list();

	//! Mark data dirty
	void dirty(qsl_data* card);
	//! Store carddesigns
	void save_data();
	//! Save JSON file
	void save_json();
	//! Create server data
	bool new_server(std::string server);
	//! Current server name during save
	static std::string server_name();
	//! \p callsign does not want QSLs
	bool no_qsl(std::string callsign);


protected:
	//! Read card designs
	void load_data();
	//! Read from JSON
	bool load_json();
	//! Get JSON file from settings
	std::string json_file();

	//! QSL card data
	std::map<qsl_data::qsl_type, std::map<std::string, qsl_data*>* > data_;
	//! QSL server data
	std::map<std::string, server_data_t*> server_data_; 
	//! Do not QSL list
	std::set<std::string>  no_qsl_list_;
	//! Path to QSL data
	std::string qsl_path_;
	//! Load failed
	bool load_failed_;
	//! Current server name during save
	static std::string server_name_;
};


