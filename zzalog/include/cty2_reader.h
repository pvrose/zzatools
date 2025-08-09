#pragma once

#include "cty_data.h"
#include "cty_element.h"

#include <fstream>
#include <string>

using namespace std;

class cty2_reader
{
public:
	cty2_reader();
	~cty2_reader();

	// Load data from specified file into and add each record to the map
	bool load_data(cty_data* data, istream& in, string& version);

protected:

	bool load_entity(cty_entity* , istream& in, int& dxcc);

	cty_element* load_pattern(string value, string& match, bool& exception);

	cty_data* data_;

	

};

