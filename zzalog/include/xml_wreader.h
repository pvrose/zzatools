#pragma once

#include "xml_reader.h"

#include <map>
#include <list>
#include <string>



//! This class wraps xml_reader providing a standard method of parsing elements.
class xml_wreader :
    public xml_reader 
{
public:
    //! Constructor
    xml_wreader();
    //! Destructor
    ~xml_wreader();

     // Overloaded xml_reader methods
    //! Start - uses method std::map to direct the call
    
    //! \param name XML element name
    //! \param attributes XML attributes (name=value pairs)
    //! \return true if successful
    virtual bool start_element(std::string name, std::map<std::string, std::string>* attributes);
    //! End - uses method std::map to direct the call

    //! \param name XML element name
    //! \return true if successful
    virtual bool end_element(std::string name);
    //! Special element - override if necessary

    //! \param element_type Type of XML element
    //! \param name Element name
    //! \param content Data contents
    virtual bool declaration(xml_element::element_t element_type, std::string name, std::string content);
    //! Processing instruction - override if necessary

    //! \param name XML element name
    //! \param content Data contents
    virtual bool process_instr(std::string name, std::string content);
    //! characters - uses method std::map to direct the call

    //! \param content Data for enclosing element
    virtual bool characters(std::string content);
   
    //! structure to define the methods.
    
    //! These are static so first parameter is always a pointer to the reader object
    struct methods {
        //! Start method - second parameter attributes.
        bool (*start_method)(xml_wreader*, std::map<std::string, std::string>*);
        //! End method 
        bool (*end_method)(xml_wreader*);
        //! Characters method - second parameter data content
        bool (*chars_method)(xml_wreader*, std::string);
    };
  
    //! Map XML element name to enumerated type - std::set by client
    std::map<std::string, char> element_map_;
    //! Map enumerated type to std::set of methods to handle it  
    std::map<char, methods> method_map_;
    //! a stack of elements being processed
    std::list<char> elements_;

    //! Set to the ID to be used in status messages
    std::string reader_id_;

};