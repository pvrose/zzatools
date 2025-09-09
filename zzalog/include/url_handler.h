#ifndef __URL_HANDLER__
#define __URL_HANDLER__

#include <string>
#include <ostream>
#include <istream>
#include <cstdio>
#include <vector>
#include <mutex>

#include <curl/curl.h>

using namespace std;



	//! This class uses the libcurl API to read and post to URLs
	class url_handler
	{
	public:
		//! Constructor
		url_handler();
		//! Destructor
		~url_handler();

		//! Structure to pass various object types
		struct field_pair {
			string name;      //!< Object name
			string value;     //!< Value for a string data item
			string filename;  //!< Value for a filename 
			string type;      //!< Value for a data-type.
		};

		//! Libcurl callback to write data received from curl to the output stream
		
		//! \param data Data received from curl 
		//! \param size size of data blocks
		//! \param nmemb Number of data blocks.
		//! \param userp Pointer to user object. In this case it's an output stream the data is sent to.
		static size_t cb_write(char* data, size_t size, size_t nmemb, void* userp);
		//! Libcurl callback to read data from the input stream and forward it to curl

		//! \param data Data to be sent to curl 
		//! \param size size of data blocks
		//! \param nmemb Number of data blocks.
		//! \param userp Pointer to user object. In this case it's an input stream the data is read from.
		static size_t cb_read(char* data, size_t size, size_t nmemb, void* userp);
		//! Perform an HTTP GET operation.
		
		//! \param url Address of web resource.
		//! \param data Data stream to send data to.
		bool read_url(string url, ostream* data);
		//! Perform an HTTP POST operation.
		 
		//! \param url Address of web resource
		//! \param resource Identifier of resource type
		//! \param req Data stream to send to URL
		//! \param resp Data stream to receive any response.
		bool post_url(string url, string resource, istream* req, ostream* resp);
		//! Performa an HTTP POST FORM operation.
		 
		//! \param url Address of web resource.
		//! \param fields POST FORM parameter name/value pairs.
		//! \param req Data stream to send to URL if \p fields is nullptr.
		//! \param resp Data stream to receive any response.
		bool post_form(string url, vector<field_pair> fields, istream* req, ostream* resp);
		//! Send an e-mail
		 
		//! \param url Address of e-Mail server
		//! \param user Username
		//! \param password User's password.
		//! \param to_list List of "To:" addressees
		//! \param cc_list List of "Cc:" addressees
		//! \param bcc_list List of "Bcc:" addressees
		//! \param subject e-Mail subject.
		//! \param payload e-Mai contents
		//! \param attachments List of attached files.
		//! \param formats Formats of attached files,
		bool send_email(string url, string user, string password,
			vector<string> to_list, vector<string> cc_list, vector<string> bcc_list,
			string subject, string payload, vector<string> attachments, vector<string> formats);

	protected:
		//! Output the associated data to the stream for debugging purposes.
		
		//! \param text Description of data.
		//! \param stream Output stream.
		//! \param ptr Pointer to data.
		//! \param size Size of data (in bytes)
		static void dump(const char* text, FILE* stream, unsigned char* ptr, size_t size);
		//! Callback from libcurl debug
		
		//! \param handle Pointer to curl instance. (not used by ZZALOG)
		//! \param type Type of information from curl.
		//! \param data Debug data
		//! \param size Size of debug data.
		//! \param userp Pointer to user object (not used by ZZALOG).
		static int cb_debug(CURL* handle, curl_infotype type, char* data, size_t size, void* userp);


		//! Current curl session
		CURL * curl_;
		
		//! Lock to ensure only 1 CURL operation at once
		static recursive_mutex lock_;
	};
#endif
