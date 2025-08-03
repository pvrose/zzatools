#pragma once

#include "cty2_data.h"

#include <fstream>
#include <string>

using namespace std;

class cty2_reader
{
public:
	cty2_reader();
	~cty2_reader();

	// Load data from specified file into and add each record to the map
	bool load_data(cty2_data::data_t* data, istream& in, string& version);

protected:

	bool load_entity(cty2_data::prefix_entry* , istream& in, int& dxcc);

	void load_pattern(string value, string& match, cty2_data::pattern_entry*);

	cty2_data::data_t* data_;

	

};

