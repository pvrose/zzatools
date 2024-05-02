#include "url_handler.h"

#include <FL/fl_ask.H>

extern bool DEBUG_CURL;

mutex url_handler::lock_;

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
size_t url_handler::cb_write(char* data, size_t size, size_t nmemb, void* os) {
	// calculate the number of bytes in data
	size_t real_size = size * nmemb;
	printf("DEBUG: Getting %d bytes\n", real_size);
	// Send data to the output stream
	((ostream*)os)->write((char*)data, real_size);
	if (((ostream*)os)->good()) {
		// Successful - tell CURL 
		return real_size;
	}
	else {
		// Unsuccessful  - tell CURL
		return 0;
	}
}

// Handles the URL POST callback - copy the data from the input stream to the data
size_t url_handler::cb_read(char* data, size_t size, size_t nmemb, void* is) {
	// Calculate the number of bytes in data
	size_t read_size = size * nmemb;
	printf("DEBUG: Getting %d bytes\n", read_size);
	// Read data from the input stream and send to CURL
	((istream*)is)->read((char*)data, read_size);
	// If successful - tell CURL number of bytes actually sent
	if (((istream*)is)->good() || ((istream*)is)->eof()) {
		return (size_t)((istream*)is)->gcount();
	}
	else {
		return 0;
	}
}

// Read the URL (HTTP GET) and write it back to the output stream
bool url_handler::read_url(string url, ostream* os) {
	
	lock_.lock();

	CURLcode result;
	// Start a new transfer
	curl_ = curl_easy_init();

	/* specify URL to get */
	printf("DEBUG: Fetching %s\n", url.c_str());
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	/* send all data to this function  */
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, cb_write);
	/* we pass the output stream to the callback function */
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, os);

	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	curl_easy_setopt(curl_, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	if (DEBUG_CURL) {
		// Add extra verbosity
		curl_easy_setopt(curl_, CURLOPT_DEBUGFUNCTION, cb_debug);
		curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);
	}
	/* get it! */
	result = curl_easy_perform(curl_);

	/* check for errors */
	if (result != CURLE_OK) {
		curl_easy_cleanup(curl_);
		lock_.unlock();
		return false;
	}

	/* reset transfer details */
	curl_easy_cleanup(curl_);
	lock_.unlock();

	return true;
}

// Perform an HTTP PUT operation - this may respond with data
bool url_handler::post_url(string url, string resource, istream* req, ostream* resp) {

    lock_.lock();
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
	/* now specify we want to POST data */
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

	if (DEBUG_CURL) {
		// Add extra verbosity
		curl_easy_setopt(curl_, CURLOPT_DEBUGFUNCTION, cb_debug);
		curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);
	}
	/* get it! */
	result = curl_easy_perform(curl_);

	/* check for errors */
	if (result != CURLE_OK) {
		// TODO: This doesn't clean up all the memory
		// Reset the operation and clean up

		curl_easy_reset(curl_);
		curl_easy_cleanup(curl_);
		lock_.unlock();
		return false;
	}

	/* reset transfer details */
	curl_easy_cleanup(curl_);
    lock_.unlock();
	return true;
}

// Performa an HTTP POST FORM operation 
bool url_handler::post_form(string url, vector<field_pair> fields, istream* req, ostream* resp) {
	lock_.lock();
	CURLcode result;
	// Start a new transfer
	curl_ = curl_easy_init();
	curl_mime* form = nullptr;
	curl_mimepart* field = nullptr;


	// Specify the URL
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	/* send all data to this function  */
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, cb_write);
	/* we pass the output stream to the callback function */
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, resp);
	// now apend the form fields
	form = curl_mime_init(curl_);
	for (auto it = fields.begin(); it != fields.end(); it++) {
		field = curl_mime_addpart(form);
		curl_mime_name(field, (*it).name.c_str());
		if ((*it).value.length()) {
			// We have specified a non-empty string so use that
			curl_mime_data(field, (*it).value.c_str(), (*it).value.length());
		}
		else if (req != nullptr) {
			// Get the request length
			streampos startpos = req->tellg();
			req->seekg(0, ios::end);
			streampos endpos = req->tellg();
			long req_length = (long)(endpos - startpos);
			req->seekg(0, ios::beg);
			// Use the specified input stream
			curl_mime_data_cb(field, req_length, cb_read, nullptr, nullptr, req);
		}
		else {
			// TODO: Report error
		}
		// Add a filename - if supplied
		if ((*it).filename.length()) {
			curl_mime_filename(field, (*it).filename.c_str());
		}
		// Add type
		if ((*it).type.length()) {
			curl_mime_type(field, (*it).type.c_str());
		}
	}
	// Add the form to the post
	curl_easy_setopt(curl_, CURLOPT_MIMEPOST, form);
	if (DEBUG_CURL) {
		// Add extra verbosity
		curl_easy_setopt(curl_, CURLOPT_DEBUGFUNCTION, cb_debug);
		curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);
	}
	/* get it! */
	result = curl_easy_perform(curl_);

	/* check for errors */
	if (result != CURLE_OK) {
		// NB: This doesn't clean up all the memory
		// Reset the operation and clean up
		curl_easy_reset(curl_);
		curl_easy_cleanup(curl_);
		lock_.unlock();
		return false;
	}
	else {
		long code;
		// Check the HTTP response
		if (curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &code) == CURLE_OK) {
			if (code != 200) {
				lock_.unlock();
				return false;
			}
		}
	}

	/* reset transfer details */
	curl_easy_cleanup(curl_);
	curl_mime_free(form);
    lock_.unlock();
	return true;
}

// Dump ptr to the file stream
void url_handler::dump(const char* text,
	FILE* stream, unsigned char* ptr, size_t size)
{
	bool newline = false;

	// Output the name of the data and size (in decimal and hex)
	fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
		text, (long)size, (long)size);

	// For every data byte
	for (size_t i = 0; i < size; i++) {

		if (ptr[i] >= 7 && ptr[i] <= 13) {
			// Convert control characters to escaped 
			switch (ptr[i]) {
			case '\a':
				fputs("\\a", stream);
				break;
			case '\b':
				fputs("\\b", stream);
				break;
			case '\t':
				fputs("\\t", stream);
				break;
			case '\n':
				fputs("\\n", stream);
				newline = true;
				break;
			case '\v':
				fputs("\\v", stream);
				newline = true;
				break;
			case '\f':
				fputs("\\f", stream);
				newline = true;
				break;
			case '\r':
				fputs("\\r", stream);
				newline = true;
				break;
			}
		}
		else if (ptr[i] < 32 || ptr[i] > 0x7F) {
			// Convert other non-printable or non-ASCII characters to hex escaped form
			if (newline) {
				fputc('\n', stream);
				newline = false;
			}
			fprintf(stream, "\\x%02x", ptr[i]);

		}
		else if (ptr[i] == '\\') {
			// Convert '\' to '\\'
			if (newline) {
				fputc('\n', stream);
				newline = false;
			}
			fputs("\\\\", stream);
		} else
		{
			// Print the ASCII character
			if (newline) {
				fputc('\n', stream);
				newline = false;
			}
			fputc(ptr[i], stream);
		}
	}
	fputc('\n', stream);
}

// Callback to display any debug information
int url_handler::cb_debug(CURL* handle, curl_infotype type,
	char* data, size_t size,
	void* userp)
{
	const char* text;
	(void)handle; /* prevent compiler warning that we don't use input parameters*/
	(void)userp;

	switch (type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== Info: %s", data);
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	dump(text, stderr, (unsigned char*)data, size);
	return 0;
}
