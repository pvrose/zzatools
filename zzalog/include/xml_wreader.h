#pragma once

#include "xml_reader.h"

#include <map>
#include <list>
#include <string>

using namespace std;

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
    //! Start - uses method map to direct the call
    
    //! \param name XML element name
    //! \param attributes XML attributes (name=value pairs)
    //! \return true if successful
    virtual bool start_element(string name, map<string, string>* attributes);
    //! End - uses method map to direct the call

    //! \param name XML element name
    //! \return true if successful
    virtual bool end_element(string name);
    //! Special element - override if necessary

    //! \param element_type Type of XML element
    //! \param name Element name
    //! \param content Data contents
    virtual bool declaration(xml_element::element_t element_type, string name, string content);
    //! Processing instruction - override if necessary

    //! \param name XML element name
    //! \param content Data contents
    virtual bool process_instr(string name, string content);
    //! characters - uses method map to direct the call

    //! \param content Data for enclosing element
    virtual bool characters(string content);
   
    //! structure to define the methods.
    
    //! These are static so first parameter is always a pointer to the reader object
    struct methods {
        //! Start method - second parameter attributes.
        bool (*start_method)(xml_wreader*, map<string, string>*);
        //! End method 
        bool (*end_method)(xml_wreader*);
        //! Characters method - second parameter data content
        bool (*chars_method)(xml_wreader*, string);
    };
  
    //! Map XML element name to enumerated type - set by client
    map<string, char> element_map_;
    //! Map enumerated type to set of methods to handle it  
    map<char, methods> method_map_;
    //! a stack of elements being processed
    list<char> elements_;

    //! Set to the ID to be used in status messages
    string reader_id_;

};