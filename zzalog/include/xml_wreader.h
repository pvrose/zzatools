#pragma once

#include "xml_reader.h"

#include <map>
#include <list>
#include <string>

using namespace std;

class xml_wreader :
    public xml_reader 
{
public:
    xml_wreader();
    ~xml_wreader();

     // Overloaded xml_reader methods
    // Start - uses method map to direct the call
    virtual bool start_element(string name, map<string, string>* attributes);
    // End - uses method map to direct the call
    virtual bool end_element(string name);
    // Special element - override if necessary
    virtual bool declaration(xml_element::element_t element_type, string name, string content);
    // Processing instruction - override if necessary
    virtual bool process_instr(string name, string content);
    // characters - uses method map to direct the call
    virtual bool characters(string content);
   
    // structure to define the methods
    struct methods {
        bool (*start_method)(xml_wreader*, map<string, string>*);
        bool (*end_method)(xml_wreader*);
        bool (*chars_method)(xml_wreader*, string);
    };
   // Name to element map - set by client
    map<string, char> element_map_;
    // Map for methods to call for the particular element 
    map<char, methods> method_map_;
    // a stack of elements being processed
    list<char> elements_;

    // Set to the ID to be used in status messages
    string reader_id_;

};