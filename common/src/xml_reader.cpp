#include "xml_reader.h"
#include "xml_element.h"

#include <regex>
#include <ctime>
#include <istream>
#include <string>

#include <FL/fl_ask.H>
#include <FL/Fl.H>

// Definition of valid white-space
const basic_regex<char> REGEX_WHITE_SPACE("\\s*");

using namespace std;

// Constructor
xml_reader::xml_reader()
	: prolog_(nullptr)
	, element_(nullptr)
	, current_element_(nullptr)
	, information_("")
	, line_num_(0)
	, report_errors_to_screen_(false)
{
	// Set the predefined entities
	entities_.clear();
	entities_["&lt;"] = "<";
	entities_["&amp;"] = "&";
	entities_["&gt;"] = ">";
	entities_["&apos;"] = "'";
	entities_["&quot;"] = "\"\"";
}

// Destructor
xml_reader::~xml_reader()
{
	// Release memory
	delete element_;
	element_ = nullptr;
	delete prolog_;
	prolog_ = nullptr;
	entities_.clear();
}

// Process input stream
bool xml_reader::parse(istream& is) {
	bool ok = true;
	// Process alternately character data and tags
	while (is.good() && ok) {
		ok = process_chars(is);
		if (ok) ok = process_tag(is);
	}
	if (ok) {
		// If stopped by closing, we would not be at EOF
		ok = is.eof();
	}
	return ok;

}

// Returns the top element
xml_element* xml_reader::element() {
	return element_;
}

// Process the data between the < and the next > - leaves stream after >
bool xml_reader::process_tag(istream& is) {
	// Don't do it if at the end of the data - but return OK
	if (is.eof()) {
		return true;
	}
	// If not at a tag start - return FAIL
	char c = is.get();
	if (c != '<') {
		report_error("Tag '<' not found at expected position", false);
		return false;
	}
	else {
		// Look to see what the type of tag and call the appropriate 
		c = is.get();
		switch (c) {
		case '!':
			if (is.good()) return process_decl(is);
			return false;
		case '?':
			if (is.good()) return process_process(is);
			return false;
		case '/':
			if (is.good()) return process_end_tag(is);
			return false;
		default:
			// Put the got character back on the stream and process it as an element start tag
			is.unget();
			return process_start_tag(is);
		}
		// Still good
		return is.good() || is.eof();
	}
}

// Report the error message - can_accept indicates it may be a recoverable error
bool xml_reader::report_error(string message, bool can_accept) {
	bool accepted = false;
	// Offer the user the option of continuing
	if (can_accept || report_errors_to_screen_) {
		fl_beep(FL_BEEP_QUESTION);
		int choice = fl_choice("XML Issue: %s at line %d. Continue yes/no?", "Continue?", "Quit?", nullptr, message.c_str(), line_num_);
		accepted = (choice == 0);
	}
	// Accumulate error messages
	information_ += message + '\n';
	return accepted;

}

// reads the stream until white space or = / > - leaves stream at that character
bool xml_reader::process_name(istream& is, string& name) {
	// find the end of the name indicated by white-space, / or >
	bool end = false;
	while (!end) {
		// Read the character
		char c = is.get();
		switch (c) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '=':
		case '/':
		case '>':
			// White-space, =, / or > indicates end of name - backtrack input stream
			is.unget();
			end = true;
			break;
		default:
			// append the character to the name
			name += c;
			break;
		}
	}
	return is.good() || is.eof();;
}

// Looks at the parse point and gets the next NAME = VALUE pair
// Moves the parse point to after the attribute and adjusts the search length
bool xml_reader::process_attr(istream& is, map<string, string>*& attributes) {
	// Create attributes
	if (attributes == nullptr) {
		attributes = new map<string, string>;
		attributes->clear();
	}
	bool ok = true;
	// Skip white space
	if (ok) ok = ignore_white_space(is);
	if (ok) {
		string name;
		// Get name and skip any white space
		ok = process_name(is, name);
		ok &= ignore_white_space(is);
		char c = is.get();
		if (c != '=') {
			// Expect = after name and white space
			report_error("This is not an attribute pair", false);
		} else {
			ok = ignore_white_space(is);
			if (ok) {
				// the first character will be ' or " - the attribute value will end with the same
				char quote = is.get();
				char c = is.get();
				string value = "";
				// Until closing quote or erroneously read end bracket
				while (c != quote && is.good() && c != '>') {
					value += c;
					c = is.get();
				}
				// If EOF or closing bracket read before closing quote
				if (is.eof() || c == '>') {
					report_error("Closing quote not found in parsing attribute.", false);
					ok = false;
				} else {
					// Add attribute, creating the attributes if necessary
					(*attributes)[name] = value;
				}
			}
		}
	}
	return is.good() || is.eof();
}

// Ignore white space
bool xml_reader::ignore_white_space(istream& is) {
	bool cr = false;
	bool ok = true;
	bool white_space = true;
	while (white_space && ok) {
		char c = is.get();
		switch (c) {
		case ' ':
		case '\t':
			// Space or tab - continue search
			cr = false;
			white_space = true;
			ok = is.good();
			break;
		case '\r':
			// CR - note the fact
			cr = true;
			white_space = true;
			line_num_++;
			break;
		case '\n':
			// LF - if previous was CR don't increment line number again
			if (cr) {
				cr = false;
				white_space = true;
				ok = is.good();
			}
			else {
				cr = false;
				white_space = true;
				line_num_++;
			}
			break;
		default:
			cr = false;
			white_space = false;
			is.unget();
			break;
		}
	}
	return true;
}

// Get declaration - process tag starting <! - leave after >
bool xml_reader::process_decl(istream& is) {
	ignore_white_space(is);
	string name;
	string content;
	xml_element::element_t type;
	bool ok = true;
	char data[8];
	is.read(data, 8);
	if (strncmp(data, "--", 2) == 0) {
		// Comment - & and are allowed so copy tag directly
		// Step back to start of comments
		is.seekg(-6, ios_base::cur);
		bool end = false;
		bool crlf = false;
		while (!end && is.good()) {
			// look at the next three characters for end of comment
			is.read(data, 3);
			if (strncmp(data, "-->", 3) == 0) {
				end = true;
			}
			else {
				content += data[0];
				switch (data[0]) {
				case '\r':
				case '\n':
					// CR or LF - only incrment line_number for CR
					if (crlf) {
						crlf = false;
					}
					else {
						crlf = true;
						line_num_++;
					}
					break;
				default:
					crlf = false;
				}
				// step back two to effectively step the start of checked characters by 1
				is.seekg(-2, ios_base::cur);
			}
		}
		type = xml_element::COMMENT;
	}
	else if (strncmp(data, "[CDATA[", 7) == 0) {
		// Looking at CDATA
		is.seekg(-1, ios_base::cur);
		ignore_white_space(is);
		bool end = false;
		while (!end && is.good()) {
			// look at the next three characters for end of comment
			is.read(data, 3);
			if (strncmp(data, "]]>", 3) == 0) {
				end = true;
			}
			else {
				content += data[0];
				// step back to check the next three characters
				is.seekg(-2, ios_base::cur);
			}
		}
		type = xml_element::CDATA_DECL;
	}
	else if (strncmp(data, "DOCTYPE", 7) == 0) {
		is.seekg(-1, ios_base::cur);
		// DOCTYPE - & and are not allowed so unescape
		ignore_white_space(is);
		type = xml_element::DOC_DECL;
		while (is.peek() != '>' && is.good()) process_escape(is, content);
	}
	else if (strncmp(data, "ELEMENT", 7) == 0) {
		is.seekg(-1, ios_base::cur);
		// ELEMENT - & and are not allowed so unescape
		ignore_white_space(is);
		type = xml_element::ELEM_DECL;
		while (is.peek() != '>' && is.good()) process_escape(is, content);
		is.seekg(1, ios_base::cur);
	}
	else if (strncmp(data, "ATTLIST", 7) == 0) {
		is.seekg(-1, ios_base::cur);
		// ATTLIST - & and are not allowed so unescape
		ignore_white_space(is);
		type = xml_element::ATTLIST_DECL;
		while (is.peek() != '>' && is.good()) process_escape(is, content);
		is.seekg(1, ios_base::cur);
	}
	else if (strncmp(data, "ENTITY", 6) == 0) {
		is.seekg(-2, ios_base::cur);
		// ENTITY - & and are not allowed so unescape
		ignore_white_space(is);
		type = xml_element::ENTITY_DECL;
		while (is.peek() != '>' && is.good()) process_escape(is, content);
		is.seekg(1, ios_base::cur);
	}
	else if (strncmp(data, "NOTATION", 8) == 0) {
		// NOTATION - & and are not allowed so unescape
		ignore_white_space(is);
		type = xml_element::NOTATION_DECL;
		while (is.peek() != '>' && is.good()) process_escape(is, content);
		is.seekg(1, ios_base::cur);
	}
	else {
		ok = false;
	}
	if (ok) {
		declaration(type, name, content);
	}
	if (!ok) {
		char message[256];
		snprintf(message, 256, "Invalid declaration statement Name %s Content %s> encountered", name.c_str(), content.c_str());
		report_error(message, false);
	}
	return ok;
}

// Process <? - leave after ?>
bool xml_reader::process_process(istream& is) {
	string name;
	string content;
	bool ok = process_name(is, name);
	if (ok) {
		char data[2];
		bool end = false;
		// Examine stream two characters every character
		while (!end && is.good()) {
			// Keep checking for "?>"
			is.read(data, 2);
			if (strncmp(data, "?>", 2) == 0) {
				end = true;
			}
			else {
				// Step back
				is.seekg(-1, ios_base::cur);
				content += data[0];
			}
		}
		if (ok) process_instr(name, content);
	}
	if (!ok) {
		char message[256];
		snprintf(message, 256, "Error with process instruction name = %s", name.c_str());
		report_error(message, false);
	}
	return ok;
}

// Process </ - leave after >
bool xml_reader::process_end_tag(istream& is) {
	string name;
	bool ok = true;
	// get name of element
	ok &= process_name(is, name);
	if (ok) {
		// Check if a > after any white space
		ignore_white_space(is);
		if (is.get() != '>') {
			ok = false;
		}
		if (ok) {
			ok = is.good();
			end_element(name);
		}
	}
	if (!ok) {
		char message[256];
		snprintf(message, 256, "Error with end-tag %s", name.c_str());
		report_error(message, false);
	}
	return ok;
}

// Process other <
// Leave stream after >
bool xml_reader::process_start_tag(istream& is) {
	string name;
	map<string, string>* attributes = new map<string, string>;
	bool ok = true;
	// Gat name
	ok &= process_name(is, name);
	ok &= ignore_white_space(is);
	// treat anything upto > or /> as attributes
	while (ok && is.good() && is.peek() != '>' && is.peek() != '/') {
		ok &= process_attr(is, attributes);
	}
	if (is.good()) {
		// create the element
		ok &= start_element(name, attributes);
		if (is.peek() == '/') {
			// We have an empty element tag (i.e. no content so Start and end it)
			end_element(name);
			// skip />
			is.get();
			is.get();
		}
		else {
			// skip >
			is.get();
		}
	}
	else
	{
		char message[256];
		snprintf(message, 256, "Error processing start-tag %s", name.c_str());
		report_error(message, false);
		ok = false;
	}
	return ok;
}

// Process non-mark-up characters - i.e. between > and next < - white space is preserved
bool xml_reader::process_chars(istream& is) {
	string chars = "";
	// read string until we read an open tag character (or EOF)
	while (is.peek() != '<' && is.good()) {
		string escapee;
		process_escape(is, escapee);
		chars += escapee;
	}
	// Return the result 
	return characters(chars);
}

// Look at next character in stream, replace CR/LF combinations by single CR, lookup &...; escaped
// Reference XML specification - CR-LF pair or alone CR must be replaced by LF before processing
void xml_reader::process_escape(istream& is, string& output) {
	bool cr = false;
	bool lf = false;
	char c = is.get();
	switch (c) {
	case '\n':
		if (cr) {
			// CR-LF pair ignore LF
			cr = false;
		}
		else {
			lf = true;
			output += c;
		}
		break;
	case '\r':
		if (lf) {
			// LF-CR pair ignore LF
			lf = false;
		}
		else {
			cr = true;
			// Copy to string as LF
			output += '\n';
		}
		break;
	case '&': {
		// Look for matching ';' and copy &...; to escapee
		string escapee = "";
		escapee += c;
		do {
			c = is.get();
			escapee += c;
		} while (c != ';');

		// If we have the entity reference use what it refers to, otherwise leave as is
		if (entities_.find(escapee) != entities_.end()) {
			output += entities_[escapee];
		}
		else {
			output += escapee;
		}
		break;
	}
	default:
		// All other characters
		lf = false;
		cr = false;
		output += c;
		break;
	}
}

// Default XML handlers

// Start element
bool xml_reader::start_element(string name, map<string, string>* attributes) {
	if (current_element_ == nullptr) {
		// This is the top element - create it and allow other elements to hang from it
		element_ = new xml_element(nullptr, name, "", attributes);
		current_element_ = element_;
	}
	else {
		// Create enw element and add it to the current element and make this the current one
		xml_element* element = new xml_element(current_element_, name, "", attributes);
		current_element_->element(element);
		current_element_ = element;
	}
	return true;
}

// End element
bool xml_reader::end_element(string name) {
	if (current_element_ != nullptr) {
		// Check ths is the end tag for the equivalent start tag - names match
		if (current_element_->name() != name) {
			char message[256];
			snprintf(message, 256, "End tag %s has name mis-match with start tag %s", name.c_str(), current_element_->name().c_str());
			report_error(message, false);
			return false;
		}
		else {
			// We now go up to the enclosing element
			current_element_ = current_element_->parent();
			return true;
		}
	}
	else {
		// A singleton end tag at the top level
		char message[256];
		snprintf(message, 256, "End tag %s has no start tag to match against", name.c_str());
		report_error(message, false);
		return false;
	}
}

// Special element - only comments are currently handled
bool xml_reader::declaration(xml_element::element_t type, string name, string content) {
	if (type == xml_element::COMMENT) {
		// Add the comment to the current element
		if (current_element_ == nullptr) {
			string message = "Comment found outwuth an element - comment ignored\n" + content;
			report_error(message, true);
			return true;
		}
		else {
			xml_element* comment = new xml_element(current_element_, name, content, nullptr, xml_element::COMMENT);
			current_element_->element(comment);
			return true;
		}
	}
	else {
		char message[256];
		snprintf(message, 256, "declaration %s %s not yet handled by default processor", name.c_str(), content.c_str());
		report_error(message, false);
		return false;
	}
}

// Processing instruction - only prologue is handled
bool xml_reader::process_instr(string name, string content) {
	if (name != "xml") {
		char message[256];
		snprintf(message, 256, "Processing instruction %s %s not yet handled by default processor", name.c_str(), content.c_str());
		report_error(message, false);
		return false;
	}
	else {
		// Save prologue
		prolog_ = new xml_element(nullptr, name, content, nullptr);
		return true;
	}
}

// characters - the elements text content
bool xml_reader::characters(string content) {
	if (current_element_ != nullptr) {
		// Add the content to the element
		current_element_->content(content);
	}
	else {
		// Report if text found outwith the top-level element
		if (!regex_match(content.c_str(), REGEX_WHITE_SPACE)) {
			string message = "Non-white space data found outwith an element\n" + content;
			report_error(message, true);
		}
	}
	return true;
}

// Returns the accumulated error messages
string& xml_reader::information() {
	return information_;
}

// Convert XML date time format (ISO format) to time_t
time_t xml_reader::convert_xml_datetime(string value) {
	// Get resolution of time_t
	time_t now;
	time(&now);
	time_t then = now + 1;
	double resolution = difftime(then, now);

	// YYY-MM-DDTHH:MM:SS+HH:MM
	tm tv;
	tv.tm_year = stoi(value.substr(0, 4)) - 1900;
	tv.tm_mon = stoi(value.substr(5, 2)) - 1;
	tv.tm_mday = stoi(value.substr(8, 2));
	tv.tm_hour = stoi(value.substr(11, 2));
	tv.tm_min = stoi(value.substr(14, 2));
	tv.tm_sec = stoi(value.substr(17, 2));
	tv.tm_isdst = false;
#ifdef _WIN32
	time_t result = _mkgmtime(&tv);
#else
	time_t result = timegm(&tv);
#endif
	if (value.length() > 19) {
		bool subtract = value[19] == '+';
		long tz_hour = stoi(value.substr(20, 2));
		long tz_min = stoi(value.substr(23, 2));
		double seconds = (3600.0 * tz_hour) + (60.0 * tz_min);
		long adjust = (long)(seconds / resolution);
#ifdef _WIN32
		// dates pre 1970 return -1
		if (tv.tm_year < 70) {
			tv.tm_year += 100;
			double day_s = 24 * 60 * 60;
			double cent_s = (100 * 365 + 24) * day_s;
			result = _mkgmtime(&tv);
			long long cent_t = cent_s / resolution;
			result = result - cent_t;
		}
#endif
		if (subtract) {
			result -= adjust;
		}
		else {
			result += adjust;
		}
	}
	return result;
}
