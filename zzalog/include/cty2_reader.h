#pragma once

#include "cty_data.h"

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

	bool load_entity(cty_data::ent_entry* , istream& in, int& dxcc);

	void load_pattern(string value, string& match, cty_data::patt_entry*);

	cty_data* data_;

	

};

