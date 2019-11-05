#ifndef __URL_HANDLER__
#define __URL_HANDLER__

#include <string>
#include <ostream>
#include <istream>
#include <cstdio>
#include <map>

#include <curl/curl.h>

using namespace std;

namespace zzalog {

	// This class uses the Curl API to read and post to URLs
	class url_handler
	{
	public:
		url_handler();
		~url_handler();

		// curl callback to write data received to the output stream
		static size_t cb_write(char* data, size_t size, size_t nmemb, void* userp);
		// curl callback to read data from the input stream and forward it to curl
		static size_t cb_read(char* data, size_t size, size_t nmemb, void* userp);
		// Perform an HTTP GET operation
		bool read_url(string url, ostream* data);
		// Perform an HTTP POST operation
		bool post_url(string url, string resource, istream* req, ostream* resp);
		// Performa an HTTP POST FORM operation - fields provides the form fields - if fields[name] is nullptr it uses req in the read callback
		bool post_form(string url, map<string, string> fields, istream* req, ostream* resp);

	protected:
		// Current curl session
		CURL * curl_;
	};

}
#endif
