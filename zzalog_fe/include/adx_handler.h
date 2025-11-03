#pragma once

#include "pugixml.hpp"

#include <istream>

class book;
class record;

using xml_document = pugi::xml_document;
using xml_node = pugi::xml_node;
using xml_attribute = pugi::xml_attribute;

//! This class provides loading and storing ADX version data usin pugixml
class adx_handler
{
public:
	//! Constructor
	adx_handler();

	//! Destructor
	~adx_handler();

	//! Load data from input stream \a in to book \a book.
	bool load_book(book* book, std::istream& in);

	//! Generate XML for the records in book and send them to the output stream.

	//! \param book the data to be written.
	//! \param os output stream.
	//! \param clean mark records as clean after writing.
	//! \return true if successful, false if not.
	bool store_book(book* book, std::ostream& os, bool clean);

	//! Used in showing progress.

	//! \return fraction of bytes processed. 
	double progress();

	//! Returns loading
	bool loading();

	//! Returns storing
	bool storing();

protected:

	//! Load the \p qso from the XML \p node
	bool load_record(record* qso, xml_node& node);

	//! Store the \p qso in the XML \p node
	bool store_record(record* qso, xml_node& node);

	//! ADX handler is laoding and converting data
	bool loading_;

	//! ADX handler is converting and storing data
	bool storing_;

	//! Total number of records
	int total_records_;

	//! Number processed
	int num_records_;


};

