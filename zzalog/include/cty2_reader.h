#pragma once

#include "cty_data.h"
#include "cty_element.h"

#include <fstream>
#include <string>



//! This class reads data from cty.dat from country-files.com.
class cty2_reader
{
public:
	//! Constructor.
	cty2_reader();
	//! Destructor.
	~cty2_reader();

	//! Load data

	//! \param data Internal database.
	//! \param in input stream.
	//! \param version Returns any version information in the file.
	//! \return true if successful, false if not.
	bool load_data(cty_data* data, std::istream& in, std::string& version);

protected:
	//! Load an entity item.
	
	//! \param entity Entity item to load.
	//! \param in Input stream.
	//! \param dxcc DXCC entity identifier: will be updated.
	//! \return true if succussful, false if not.
	bool load_entity(cty_entity* entity, std::istream& in, int& dxcc);

	//! Takes each pattern from the record and generates an element from it.
	
	//! \param value individual pattern.
	//! \param match updated with calsign if it is an exception.
	//! \param exception returns true if an exception pattern, otherwise false.
	//! \return either a cty_prefix* or a cty_exception* depending on the pattern.
	cty_element* load_pattern(std::string value, std::string& match, bool& exception);
	//! The internal inport database,
	cty_data* data_;

	

};

