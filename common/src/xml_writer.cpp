#include "xml_writer.h"
#include "utils.h"

#include <FL/fl_ask.H>



// Constructor
xml_writer::xml_writer()
	: style_(INDENT)
	, indent_depth_(4)
	, indent_char_(' ')
	, num_written_(0)
	, progress_(false)
{
}

// Destructor
xml_writer::~xml_writer()
{
}

// Set the indent style
void xml_writer::indent(xml_writer::format_style_t output_style, int depth) {
	style_ = output_style;
	switch (style_) {
	case INDENT:
		// Indent style - a number of space characters
		indent_depth_ = depth;
		indent_char_ = ' ';
		break;

	case TAB_INDENT:
		// Indent style - a single tab character
		indent_depth_ = 1;
		indent_char_ = '\t';
		break;
	case CONTINUOUS:
	case LINE_FEED:
		// No indent
		indent_depth_ = 0;
		break;
	}
}

// Output the data to the stream
bool xml_writer::data(ostream& os) {
	if (element_ == nullptr || prolog_ == nullptr) {
		return false;
	}
	else {
		// initialise progress
		int total = element_->descendants();
		progress(0, total);
		// Output the prolog
		bool ok = write_prolog(prolog_, os);
		// Output the element at indent level 0
		ok &= write_element(element_, os, 0);
		return ok;
	}
}

// Write the prolog - <?xml ...attributes?>
bool xml_writer::write_prolog(xml_element* prolog, ostream& os) {
	char temp[1024];
	snprintf(temp, 1024, "<?%s %s?>", prolog->name().c_str(), prolog->content().c_str());
	os << temp;
	return true;
}

// Write the element <NAME ATTR=VALUE...>content children</NAME> or empty <NAME ATTR=VALUE/>
bool xml_writer::write_element(xml_element* element, ostream& os, int level) {
	// Depending on element type
	switch (element->type()) {
	case xml_element::ELEMENT: {
		// Element 
		char temp[1024];
		// Add indent
		bool ok = write_indent(os, level);
		// Output start of start tag <NAME
		snprintf(temp, 1024, "<%s", element->name().c_str());
		os << temp;
		// If there are any attributes
		if (element->attributes() != nullptr) {
			// For each attribute
			for (auto it = element->attributes()->begin(); it != element->attributes()->end(); it++) {
				char quote;
				// If it has a " in the value use ' as quote else use "
				if (it->second.find('\"') != it->second.npos) {
					quote = '\'';
				}
				else {
					quote = '\"';
				}
				snprintf(temp, 1024, " %s=%c%s%c", it->first.c_str(), quote, it->second.c_str(), quote);
				os << temp;
			}
		}
		if (element->content().length() == 0 && element->count() == 0) {
			// Empty element
			os << "/>";
		}
		else {
			// End start-tag and add content and end-tag
			os << '>';
			// For each child element output the child element
			for (int i = 0; i < element->count() && ok; i++) {
				ok &= write_element(element->child(i), os, level + 1);
			}
			// If there were chile elements add an indent before the end tag
			if (element->count() > 0) {
				write_indent(os, level);
			}
			// Add any character content
			string escaped = escape_string(element->content());
			snprintf(temp, 1024, "%s</%s>", escaped.c_str(), element->name().c_str());
			os << temp;
		}
		// If writing XML to file display progress every 100 elements at any level
		num_written_++;
		if (num_written_ % 100 == 0) {
			progress(num_written_);
		}
		return ok;
	}
	case xml_element::COMMENT: 
		// Add content
		os << '\n' << "<!--" << element->content() << "-->" << '\n';
		return true;
	default:
		return false;
	}
}

// output any indentation to the stream
bool xml_writer::write_indent(ostream& os, int level) {
	if (style_ == INDENT || style_ == TAB_INDENT || style_ == LINE_FEED) {
		os << '\n';
		for (int i = 0; i < indent_depth_ * level; i++) {
			os << indent_char_;
		}
	}
	return true;
}

// Escape special characters
string xml_writer::escape_string(string source) {
	string result = "";
	// For the length of the source striing
	for (size_t i = 0; i < source.length(); i++) {
		char c = source[i];
		bool is_entity = false;
		string key;
		// Look up the character in the entities
		for (auto it = entities_.begin(); it != entities_.end() && !is_entity; it++) {
			// If it is one get the entity string
			if (it->second[0] == c) {
				is_entity = true;
				key = it->first;
			}
		}
		// Append either the entity string or the original character
		if (is_entity) {
			result += key;
		//}
		//else if (c < '\x20' || c > '\x7f') {
		//	result += "x" + to_hex(c, false);
		} else {
			result += c;
		}
	}
	return result;
}