#ifndef __XML_WRITER__
#define __XML_WRITER__

#include "xml_reader.h"

#include <string>
#include<ostream>



class xml_element;

	//! This class provides the means to write out the XML prolog and element to an output std::string. 
	 
	//! It derives from xml_reader as this provides the base API that the
	//! application specific code uses to generate the XML element structure
	class xml_writer :
		public xml_reader
	{
	public:
		//! Constructor.
		xml_writer();
		//! Destructor
		virtual ~xml_writer();

		//! Style of formatting output
		enum format_style_t {
			INDENT,           //!< A number of space characters indent per depth 
			TAB_INDENT,       //!< A single tab characterper depth
			CONTINUOUS,       //!< Continual data, no LF
			LINE_FEED         //!< A single LF, no indentation
		};

		//! Set the indent format (\p output_style) and number of spaces (\p depth) to use
		void indent(format_style_t output_style, int depth);

		//! Output the data to the output stream \p os
		bool data(std::ostream& os);

		//! Present progress
		virtual void progress(int count, int total = -1) {};

		//! Returns time \p t as a std::string in the XML standard date and time format. 
		static std::string convert_xml_datetime(time_t t);

	protected:
		//! Output an \p element to stream \p os at indentation \p level
		bool write_element(xml_element* element, std::ostream& os, int level);
		//! Output the \p prolog to stream \p os
		bool write_prolog(xml_element* prolog, std::ostream& os);
		//! Write indentation at \p level.
		bool write_indent(std::ostream& os, int level);
		//! Returns \p source with special characters escaped.
		std::string escape_string(std::string source);

		//! The style to generate output
		format_style_t style_;
		//! The number of characters per indent (TAB_INDENT always 1)
		int indent_depth_;
		//! Indent character
		char indent_char_;
		//! Number of elements written
		int num_written_;
		//! writing progress
		bool progress_;

	};
#endif
