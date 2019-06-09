#include "url_handler.h"
#include "status.h"

#include <FL/fl_ask.H>

using namespace zzalog;

extern status* status_;

// Constructor
url_handler::url_handler()
	: curl_(nullptr)
{
	// Global initialisation of CURL
	curl_global_init(CURL_GLOBAL_ALL);
}

// Destructor
url_handler::~url_handler()
{
	// we're done with libcurl, so clean it up 
	curl_global_cleanup();
}

// Handles the URL GET callback - copy the data directly to the output stream
size_t url_handler::cb_write(void* data, size_t size, size_t nmemb, ostream* os) {
	// calculate the number of bytes in data
	size_t real_size = size * nmemb;
	// Send data to the output stream
	os->write((char*)data, real_size);
	if (os->good()) {
		// Successful - tell CURL 
		return real_size;
	}
	else {
		// Unsuccessful  - tell CURL
		return 0;
	}
}

// Handles the URL POST callback - copy the data from the input stream to the data
size_t url_handler::cb_read(void* data, size_t size, size_t nmemb, istream* is) {
	// Calculate the number of bytes in data
	size_t read_size = size * nmemb;
	// Read data from the input stream and send to CURL
	is->read((char*)data, read_size);
	// If successful - tell CURL number of bytes actually sent
	if (is->good() || is->eof()) {
		return (size_t)is->gcount();
	}
	else {
		return 0;
	}
}

// Read the URL (HTTP GET) and write it back to the output stream
bool url_handler::read_url(string url, ostream* os) {

	CURLcode result;
	// Start a new transfer
	curl_ = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	/* send all data to this function  */
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, cb_write);
	/* we pass the output stream to the callback function */
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, os);
	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	curl_easy_setopt(curl_, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	/* get it! */
	result = curl_easy_perform(curl_);

	/* check for errors */
	if (result != CURLE_OK) {
		char* message = new char[strlen(curl_easy_strerror(result)) + 50];
		sprintf(message, "HTTP GET: failed: %s", curl_easy_strerror(result));
		status_->misc_status(ST_ERROR, message);
		delete[] message;
		curl_easy_cleanup(curl_);
		return false;
	}

	/* reset transfer details */
	curl_easy_cleanup(curl_);

	return true;
}

// Perform an HTTP PUT operation - this may respond with data
bool url_handler::post_url(string url, string resource, istream* req, ostream* resp) {

	CURLcode result;
	// Start a new transfer
	curl_ = curl_easy_init();

	// Get the request length
	streampos startpos = req->tellg();
	req->seekg(0, ios::end);
	streampos endpos = req->tellg();
	long req_length = (long)(endpos - startpos);
	req->seekg(0, ios::beg);

	// Specify the URL
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	/* Now specify we want to POST data */ 
    curl_easy_setopt(curl_, CURLOPT_POST, 1L);
	// Request target
	curl_easy_setopt(curl_, CURLOPT_REQUEST_TARGET, resource.c_str());
 	// Set the request handler
	curl_easy_setopt(curl_, CURLOPT_READFUNCTION, cb_read);
	// Provde the request data
	curl_easy_setopt(curl_, CURLOPT_READDATA, req);
	/* Set the expected POST size */
	curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, req_length);
	/* send all data to this function  */
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, cb_write);
	/* we pass the output stream to the callback function */
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, resp);
	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	curl_easy_setopt(curl_, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	/* get it! */
	result = curl_easy_perform(curl_);

	/* check for errors */
	if (result != CURLE_OK) {
		// TODO: This doesn't clean up all the memory
		// Reset the operation and clean up

		char* message = new char[strlen(curl_easy_strerror(result)) + 50];
		sprintf(message, "HTTP POST: failed: %s", curl_easy_strerror(result));
		status_->misc_status(ST_ERROR, message);
		delete[] message;
		curl_easy_reset(curl_);
		curl_easy_cleanup(curl_);
		return false;
	}

	/* reset transfer details */
	curl_easy_cleanup(curl_);

	return true;
}

