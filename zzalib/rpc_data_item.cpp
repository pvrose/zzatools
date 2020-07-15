#include "rpc_data_item.h"
#include "../zzalib/utils.h"

#include <stdexcept>

using namespace zzalib;

// Constructor - sets default values
rpc_data_item::rpc_data_item()
: type_(XRT_EMPTY)
, i_(0)
, s_("")
, d_(nan(""))
, array_(nullptr)
, struct_(nullptr)

{
}

// Set the data to integer value (for either integer or Boolean RPC item
void rpc_data_item::set(int i, rpc_data_t type) {
	if (type == XRT_INT || type == XRT_BOOLEAN) {
		type_ = type;
		i_ = i;
	}
}

// Set the data to a string type
void rpc_data_item::set(string s, rpc_data_t type) {
	if (type == XRT_BYTES || type == XRT_DATETIME || type == XRT_STRING) {
		type_ = type;
		s_ = s;
	}
	else if (type == XRT_DEFAULT) {
		// Some servers send bad RPC with integers and doubles supplied as default 
		// type which is string
		type_ = type;
		s_ = s;
		// Try it as integer
		try {
			i_ = stoi(s_.c_str());
		}
		catch (invalid_argument&) {
			i_ = 0;
		}
		// Try it as double
		try {
			d_ = stod(s_.c_str());
		}
		catch (invalid_argument&) {
			d_ = nan("");
		}
	}
}

// Set a double
void rpc_data_item::set(double d) {
	type_ = XRT_DOUBLE;
	d_ = d;
}

// Set an array
void rpc_data_item::set(rpc_data_item::rpc_array* ap) {
	type_ = XRT_ARRAY;
	array_ = ap;
}

// Set a struct
void rpc_data_item::set(rpc_data_item::rpc_struct* mp) {
	type_ = XRT_STRUCT;
	struct_ = mp;
}

// Destructor
rpc_data_item::~rpc_data_item()
{
	// Destroy compound types
	switch (type_) {
	case XRT_ARRAY:
		// For each element of the array
		for (auto it = array_->begin(); it != array_->end(); it++) {
			// destroy it
			delete *it;
		}
		array_->clear();
		delete array_;
	case XRT_STRUCT:
		// For each element in a structure
		for (auto it = struct_->begin(); it != struct_->end(); it++) {
			// destroy it
			delete it->second;
		}
		struct_->clear();
		delete struct_;
	}
}

// Return the data type
rpc_data_t rpc_data_item::type() {
	return type_;
}

// Get the integer
bool rpc_data_item::get(int& i) {
	if (type_ == XRT_INT || type_ == XRT_BOOLEAN || type_ == XRT_DEFAULT) {
		i = i_;
		return true;
	}
	else {
		// Not an integer type
		return false;
	}
}

// Get the integer
int rpc_data_item::get_int() {
	return i_;
}

// Get the string
bool rpc_data_item::get(string& s) {
	if (type_ == XRT_STRING || type_ == XRT_BYTES || type_ == XRT_DATETIME) {
		s = s_;
		return true;
	}
	else {
		// Not a string type
		return false;
	}
}

// return string
string rpc_data_item::get_string() {
	return s_;
}

// Get the double
bool rpc_data_item::get(double& d) {
	if (type_ == XRT_DOUBLE || type_ == XRT_DEFAULT) {
		d = d_;
		return true;
	}
	else {
		// Not a double
		return false;
	}
}

// Return the double
double rpc_data_item::get_double() {
	return d_;
}

// Get the array
bool rpc_data_item::get(rpc_array*& ap) {
	if (type_ == XRT_ARRAY) {
		ap = array_;
		return true;
	}
	else {
		// Not an array
		return false;
	}
}

// Returns the array
rpc_data_item::rpc_array* rpc_data_item::get_array() {
	return array_;
}

// Get the structure
bool rpc_data_item::get(rpc_struct*& mp) {
	if (type_ == XRT_STRUCT) {
		mp = struct_;
		return true;
	}
	else {
		// Not a structure
		return false;
	}
}

// Returns the struct
rpc_data_item::rpc_struct* rpc_data_item::get_struct() {
	return struct_;
}

// Convert the item to text for display
string rpc_data_item::print_item() {
	char temp[1024];
	string result = "";
	switch (type_) {
	case XRT_BOOLEAN:
		// Display 0 or 1
		sprintf(temp, "Boolean: %1d\n", i_);
		result = temp;
		break;
	case XRT_INT:
		// Integer
		sprintf(temp, "Int: %d\n", i_);
		result = temp;
		break;
	case XRT_DOUBLE:
		// Double
		sprintf(temp, "Double: %f\n", d_);
		result = temp;
		break;
	case XRT_STRING:
		// String
		sprintf(temp, "String: %s\n", s_.c_str());
		result = temp;
		break;
	case XRT_DATETIME:
		// Date/Time as string
		sprintf(temp, "Date/Time: %s\n", s_.c_str());
		result = temp;
		break;
	case XRT_BYTES:
		// Base64 encoding as string
		sprintf(temp, "Base64: %s\n", encode_base_64(s_).c_str());
		result = temp;
		break;
	case XRT_ARRAY:
		// Array
		result = "rpc_array:\n";
		// For each item in the array - append it text
		for (auto it = array_->begin(); it != array_->end(); it++) {
			result += (*it)->print_item();
		}
		break;
	case XRT_STRUCT:
		// Structure
		result = "rpc_struct:\n";
		// For each element in the structure - appends its nae and text
		for (auto it = struct_->begin(); it != struct_->end(); it++) {
			string key = it->first;
			rpc_data_item* item = it->second;
			sprintf(temp, "Name: %s\nValue: ", key.c_str());
			result += temp;
			result += item->print_item();
		}
		break;
	}
	return result;
}
