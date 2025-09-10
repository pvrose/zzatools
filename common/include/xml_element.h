#ifndef __XML_ELEMENT__
#define __XML_ELEMENT__

#include <string>
#include <map>
#include <vector>

using namespace std;



	//! This class provides the basic constituents of an XML element
	class xml_element
	{
	public:
		//! XML element types
		enum element_t {
			ELEMENT,                //!<  Standard element - has name, attributes and content
			COMMENT,                //!< \code <!--........--> \endcode - has content
			DOC_DECL,               //!< \code <!DOCTYPE.....> \endcode - has name and content
			ELEM_DECL,              //!< \code <!ELEMENT.....> \endcode - do.
			ATTLIST_DECL,           //!< \code <!ATTLIST.....> \endcode - do.
			ENTITY_DECL,            //!< \code <!ENTITY......> \endcode - do.
			NOTATION_DECL,          //!< \code <!NOTATION....> \endcode - do.
			CDATA_DECL,             //!< \code <![CDATA[.....> \endcode
			XML_DECL,               //!< \code <?xml........?> \endcode - has name and attributes
			PROC_INSTR              //!< \code <?           ?> \endcode - has name and content
		};

		//! Basic constructor
		xml_element();
		//! Construct an element from all its components
		
		//! \param parent Element that this element is a child of.
		//! \param name name of the element
		//! \param content content of the element
		//! \param attributes attributes in the element start tag.
		//! \param type Element type
		xml_element(xml_element* parent, const string& name, const string& content, map<string, string>* attributes, element_t type = ELEMENT);
		//! Destructor
		~xml_element();

		//! Add a child element 
		void element(xml_element* element);
		//! Add an attributes
		bool attribute(string& name, string& sValue);
		//! Add content to the element
		bool content(string& content);
		//! Returns the number of child elements
		int count();
		//! Returns child element number n
		xml_element* child(int n);
		//! Returns the parent element
		xml_element* parent();
		//! Returns the name of the element
		string name();
		//! Returns the attributes of the element
		map<string, string>* attributes();
		//! Returns the content of the element
		string content();
		//! Returns the type of element
		element_t type();
		//! Returns the total number of descendant elements and self
		int descendants();

	protected:
		//! Element type
		element_t type_;
		//! The child elements
		vector<xml_element*> children_;
		//! Name
		string name_;
		//! Content
		string content_;
		//! Attributes
		map<string, string>* attributes_;
		//! The parent element
		xml_element* parent_;
	};
#endif