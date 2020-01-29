#ifndef __URL_HANDLER__
#define __URL_HANDLER__

#include <string>
#include <ostream>
#include <istream>
#include <cstdio>
#include <vector>

#include <curl/curl.h>

using namespace std;

namespace zzalib {

	// This class uses the Curl API to read and post to URLs
	class url_handler
	{
	public:
		url_handler();
		~url_handler();

		struct field_pair {
			string name;
			string value;
			string filename;
			string type;
		};

		// curl callback to write data received to the output stream
		static size_t cb_write(char* data, size_t size, size_t nmemb, void* userp);
		// curl callback to read data from the input stream and forward it to curl
		static size_t cb_read(char* data, size_t size, size_t nmemb, void* userp);
		// Perform an HTTP GET operation
		bool read_url(string url, ostream* data);
		// Perform an HTTP POST operation
		bool post_url(string url, string resource, istream* req, ostream* resp);
		// Performa an HTTP POST FORM operation - fields provides the form fields - if fields[name] is nullptr it uses req in the read callback
		bool post_form(string url, vector<field_pair> fields, istream* req, ostream* resp);

	protected:
		static void dump(const char* text, FILE* stream, unsigned char* ptr, size_t size);
		static int cb_debug(CURL* handle, curl_infotype type, char* data, size_t size, void* userp);


		// Current curl session
		CURL * curl_;
	};

}
#endif
