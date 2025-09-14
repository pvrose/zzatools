#ifndef __XML__
#define __XML__

#include "xml_element.h"

#include <fstream>
#include <string>
#include <map>
#include <regex>
#include <ctime>




	//! \brief This class reads XML text and generates an XML element containing the contents 
	//! as a family tree of other elements. 
	 
	//! It can be used in derived classes which can
	//! then interpret the XML elements
	class xml_reader
	{
	public:

		//! Constructor
		xml_reader();
		//! Destructor
		virtual ~xml_reader();

		//! Read input stream \p is and parse it: return true if successful
		bool parse(std::istream& is);
		//! Get the top element
		xml_element* element();

		// Overloadable XML handlers 
		//! Start Element
		
		//! \param name Element name
		//! \param attributes Sttributes
		virtual bool start_element(std::string name, std::map<std::string, std::string>* attributes);
		//! End element
		
		//! \param name name of the element
		virtual bool end_element(std::string name);
		//! Special element
		virtual bool declaration(xml_element::element_t type, std::string name, std::string content);
		//! Processing instruction
		virtual bool process_instr(std::string name, std::string content);
		//! characters
		virtual bool characters(std::string content);
		//! Returns information concerning any processing errors
		std::string& information();

	protected:
		//! Process between < and >
		bool process_tag(std::istream& is);
		//! Process between > and <
		bool process_chars(std::istream& is);
		//! Report error
		bool report_error(std::string message, bool can_accept);
		//! Process <!
		bool process_decl(std::istream& is);
		//! Process <?
		bool process_process(std::istream& is);
		//! Process </
		bool process_end_tag(std::istream& is);
		//! Process other <
		bool process_start_tag(std::istream& is);
		//! Get the first word in the tag
		bool process_name(std::istream& is, std::string& name);
		//! Get the next attribute
		bool process_attr(std::istream& is, std::map<std::string, std::string>*& attributes);
		//! Ignore white space
		bool ignore_white_space(std::istream& is);
		//! Convert &..; to original character
		void process_escape(std::istream& is, std::string& result);
		//! The prolog element
		xml_element* prolog_;
		//! The top-element
		xml_element* element_;
		//! The current element being parsed
		xml_element* current_element_;
		//! Used to accumulate information concerning any processing errors
		std::string information_;
		//! Current line number in the input stream
		int line_num_;
		//! Report errors to screen 
		bool report_errors_to_screen_;
		//! The XML entities - initially the standard ones e.g. &lt;
		std::map<std::string, std::string> entities_;
	};
#endif