#pragma once

#include "qsl_data.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>

using namespace std;

enum extract_mode_t : uchar;

// Class to manage QSL designs

struct qsl_call_data {
	bool used;                      //!< Data is used.
	string key;                     //!< logbook access key.
	unsigned long long last_logid;  //!< Universal QSO record identifier.
	string last_download;           //!< Date last downloaded (YYYYMMDD)
};

struct server_data_t {
	bool upload_per_qso{false};     //!< Upload per QSO
	bool enabled{false};            //!< Access enabled
	string user{""};                //!< User-name (or e-mail
	string password{""};            //!< Password
	string last_downloaded{""};     //!< Date of last download
	bool download_confirmed{false}; //!< Download confirmed as well (eqSL)
	string qso_message{""};         //!< Message to add to a QSL card (eQSL)
	string swl_message{""};         //!< Message to add to a QSL card - SWL (eQSL)
	string export_file{""};         //!< File for uploading (LotW)
	bool use_api{false};            //!< Use API (QRZ.com)
	bool use_xml{false};            //!< Use XML (QRZ.com)
	string mail_server{""};         //!< Mail server (eMail)
	string cc_address{""};          //!< cc Address (eMail)
	map<string, qsl_call_data*> call_data; //!< API logbook data (QRZ.com)
};

class qsl_dataset
{
public:

	//! Constructor.
	qsl_dataset();
	//! Destructor.
	~qsl_dataset();

	//! Return the QSL design associated with the \p callsign and QSL \p type
	qsl_data* get_card(string callsign, qsl_data::qsl_type type, bool create);
	//! Get the path to settings/Datapath/QSLs
	string get_path();
	//! Returns the server data associated with the \p server (per extract_mode_t)
	server_data_t* get_server_data(string server);
	//! Returns QRZ logbook credentials
	qsl_call_data* get_qrz_api(string callsign);

	//! Mark data dirty
	void dirty(qsl_data* card);
	//! Store carddesigns
	void save_data();
	//! Save XML data
	void save_xml(Fl_Preferences& settings);
	//! Load data from file
	void load_items(qsl_data* data);
	//! Create server data
	bool new_server(string server);


protected:
	//! Read card designs
	void load_data();
	//! Read from XML fiel
	bool load_xml(Fl_Preferences& settings);
	//! Get XML file from settings
	string xml_file(Fl_Preferences& settings);

	//! QSL card data
	map<qsl_data::qsl_type, map<string, qsl_data*>* > data_;
	//! QSL server data
	map<string, server_data_t*> server_data_; 
	//! Path to QSL data
	string qsl_path_;
	//! Load failed
	bool load_failed_;

};


