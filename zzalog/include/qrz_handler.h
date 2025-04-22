#ifndef __QRZ_HANDLER__
#define __QRZ_HANDLER__

#include <string>
#include <istream>
#include <map>

using namespace std;

class record;
class xml_element;
class Fl_Help_Dialog;

// This class provides the interface to the QRZ.com XML database look-up - it currently
// is restricted to looking up details based on callsign. 
// Structure to hold API data per QRZ.com logbook		
struct qrz_api_data {
	bool used;
	string key;
	unsigned long long last_logid;
	string last_download;
};


	class qrz_handler
	{
	public:
		qrz_handler();
		~qrz_handler();

		// Load the API information
		void load_data();
		// Update the API information
		void store_data();

		

		// Open the QRZ.com session
		bool open_session();
		// Fetch the contacts details and present the user with a merge opportunity
		bool fetch_details();
		// Callback to say done
		void merge_done();
		// REturn message
		string get_merge_message();
		// Get merge data
		record* get_record();
		// Is there a session?
		bool has_session();
		// Open web-page using browser
		void open_web_page(string callsign);
		// Download update - for STATION_CALLSIGN == station
		bool download_qrzlog_log(stringstream* adif);
		// Upload current QSL.
		bool upload_qso(record* qso);


	protected:
		// Decode session response
		bool decode_session_response(istream& response);
		// DEcode details response
		bool decode_details_response(istream& response);
		// Query user on merge
		bool query_merge();
		// Generate URI for session request
		string generate_session_uri();
		// Generate URI for details request
		string generate_details_uri(string callsign);
		// Fetch user details
		bool user_details();
		// Decode an XML element - into a data map
		bool decode_xml(xml_element* element);
		// Decode QRZDatabase element
		bool decode_top(xml_element* element);
		// Decode Session element
		bool decode_session(xml_element* element);
		// Decode Callsign element
		bool decode_callsign(xml_element* element);
		// Generate fetch request
		bool fetch_request(qrz_api_data* api, ostream& request, int count);
		// Decode fetch response
		bool fetch_response(qrz_api_data* api, istream& response, int& count, string& adif);
		// Get last LogID downloaded
		unsigned long long last_logid(qrz_api_data* api, string adif);

		// The key returned from the login session attempt
		string session_key_;
		// User's QRZ.com account name
		string username_;
		// User's QRZ.com password
		string password_;
		// Current response data
		map< string, string> data_;
		// and as a record
		record* qrz_info_;
		// QRZ database version
		string qrz_version_;
		// Merge complete
		bool merge_done_;
		// Use XML Database
		bool use_xml_database_;
		// Non-subscriber
		bool non_subscriber_;
		// Use API
		bool use_api_;
		// Web dialog
		Fl_Help_Dialog* web_dialog_;
		// API data
		map<string, qrz_api_data*> api_data_;

	};

#endif


