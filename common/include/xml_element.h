#ifndef __XML_ELEMENT__
#define __XML_ELEMENT__

#include <string>
#include <map>
#include <vector>

using namespace std;



	// This class provides the basic constituents of an XML element
	class xml_element
	{
	public:
		// XML element types
		enum element_t {
			ELEMENT,                // Standard element - has name, attributes and content
			COMMENT,                // <!--........-->  - has content
			DOC_DECL,               // <!DOCTYPE.....>  - has name and content
			ELEM_DECL,              // <!ELEMENT.....>  - do.
			ATTLIST_DECL,           // <!ATTLIST.....>  - do.
			ENTITY_DECL,            // <!ENTITY......>  - do.
			NOTATION_DECL,          // <!NOTATION....>  - do.
			CDATA_DECL,             // <![CDATA[.....>
			XML_DECL,               // <?xml........?>  - has name and attributes
			PROC_INSTR              // <?           ?>  - has name and content
		};

		// Basic constructor
		xml_element();
		// Construct an element from all its components
		xml_element(xml_element* parent, const string& name, const string& content, map<string, string>* attributes, element_t type = ELEMENT);
		~xml_element();

		// Add a child element 
		void element(xml_element* element);
		// Add an attributes
		bool attribute(string& name, string& sValue);
		// Add content to the element
		bool content(string& content);
		// Returns the number of child elements
		int count();
		// Returns child element number n
		xml_element* child(int n);
		// Returns the parent element
		xml_element* parent();
		// Returns the name of the element
		string name();
		// Returns the attributes of the element
		map<string, string>* attributes();
		// Returns the content of the element
		string content();
		// Returns the type of element
		element_t type();
		// Returns the total number of descendant elements and self
		int descendants();

	protected:
		// Element type
		element_t type_;
		// The child elements
		vector<xml_element*> children_;
		// Name
		string name_;
		// Content
		string content_;
		// Attributes
		map<string, string>* attributes_;
		// The parent element
		xml_element* parent_;
	};
#endif