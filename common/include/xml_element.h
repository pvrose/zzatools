#ifndef __XML_ELEMENT__
#define __XML_ELEMENT__

#include <string>
#include <map>
#include <vector>





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
		xml_element(xml_element* parent, const std::string& name, const std::string& content, std::map<std::string, std::string>* attributes, element_t type = ELEMENT);
		//! Destructor
		~xml_element();

		//! Add a child element 
		void element(xml_element* element);
		//! Add an attributes
		bool attribute(std::string& name, std::string& sValue);
		//! Add content to the element
		bool content(std::string& content);
		//! Returns the number of child elements
		int count();
		//! Returns child element number n
		xml_element* child(int n);
		//! Returns the parent element
		xml_element* parent();
		//! Returns the name of the element
		std::string name();
		//! Returns the attributes of the element
		std::map<std::string, std::string>* attributes();
		//! Returns the content of the element
		std::string content();
		//! Returns the type of element
		element_t type();
		//! Returns the total number of descendant elements and self
		int descendants();

	protected:
		//! Element type
		element_t type_;
		//! The child elements
		std::vector<xml_element*> children_;
		//! Name
		std::string name_;
		//! Content
		std::string content_;
		//! Attributes
		std::map<std::string, std::string>* attributes_;
		//! The parent element
		xml_element* parent_;
	};
#endif