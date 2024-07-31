#ifndef __XML_WRITER__
#define __XML_WRITER__

#include "xml_reader.h"

#include <string>
#include <ostream>

using namespace std;

class xml_element;

	// This class provides the means to write out the XML prolog and element to an output
	// string. It derives from xml_reader as this provides the base API that the
	// application specific code uses to generate the XML element structure
	class xml_writer :
		public xml_reader
	{
	public:
		xml_writer();
		virtual ~xml_writer();

		// Style of formatting output
		enum format_style_t {
			INDENT,           // A number of space characters indent per depth 
			TAB_INDENT,       // A single tab characterper depth
			CONTINUOUS,       // Continual data, no LF
			LINE_FEED         // A single LF, no indentation
		};

		// Set the indent format and number of spaces to use
		void indent(format_style_t output_style, int depth);

		// Output the data to the output stream
		bool data(ostream& os);

		// Present progress
		virtual void progress(int count, int total = -1) {};

	protected:
		// Output an element
		bool write_element(xml_element* element, ostream& os, int level);
		// Output the prolog
		bool write_prolog(xml_element* pProlog, ostream& os);
		// Write indentation
		bool write_indent(ostream& os, int level);
		// Escape special characters
		string escape_string(string source);

		// The style to generate output
		format_style_t style_;
		// The number of characters per indent (TAB_INDENT always 1)
		int indent_depth_;
		// Indent character
		char indent_char_;
		// Number of elements written
		int num_written_;
		// writing progress
		bool progress_;

	};
#endif
