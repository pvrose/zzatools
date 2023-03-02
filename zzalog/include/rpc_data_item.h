#ifndef __RPC_DATA_ITEM__
#define __RPC_DATA_ITEM__

#include <string>
#include <map>
#include <vector>
#include <list>

using namespace std;



	// XML-RPC Request/Response - XML element types
	enum rpc_element_t {
		XRP_NONE,               // Not in an element - at top-level
		XRP_METHODCALL,         // <methodCall> - requesting paticular method
		XRP_METHODNAME,         // <methodName> - name of method
		XRP_METHODRESPONSE,     // <methodResponse> - response element
		XRP_FAULT,              // <fault> - error message
		XRP_PARAMS,             // <params> - start of parameter section    
		XRP_PARAM,              // <param> - individual parameter
		XRP_VALUE,              // <value> - individual data item
		XRP_ARRAY,              // <array> - array of items
		XRP_DATA,               // <data> - start of array items
		XRP_STRUCT,             // <struct> - hashed array of items
		XRP_MEMBER,             // <member> - individual entry in hash array
		XRP_NAME,               // <name> - key to item in hash array
		XRP_INT,                // <int> or <i4> - 32-bit integer item
		XRP_BOOLEAN,            // <Boolean> - 1 or 0
		XRP_DOUBLE,             // <double> - 64-bit floating point
		XRP_STRING,             // <string>
		XRP_DATETIME,           // <dateTime.iso8601> - Dates in ISO8601 format: YYYYMMDDTHH:MM:SS
		XRP_BASE64,             // <base64> - Binary information encoded as Base 64, as defined in RFC 2045
	};

	// XML-RPC data types
	enum rpc_data_t {
		XRT_EMPTY,              // Empty data set
		XRT_INT,                // I4
		XRT_BOOLEAN,            // Y/N
		XRT_DOUBLE,             // double
		XRT_STRING,             // String (ASCII)
		XRT_DEFAULT,            // String (ASCII) by default of no type
		XRT_BYTES,              // store as string after compacting the data
		XRT_DATETIME,           // Date/time - store as string
		XRT_ARRAY,              // Array of data types
		XRT_STRUCT              // Struct
	};


	// This class describes an Remote Procedure Call data item 
	class rpc_data_item
	{
	public:
		// Compound RPC data-types
		typedef vector<rpc_data_item*> rpc_array;
		typedef map<string, rpc_data_item*> rpc_struct;
		typedef list<rpc_data_item*> rpc_list;

	public:
		rpc_data_item();
		~rpc_data_item();

		// Returns the data type
		rpc_data_t type();
		// Get the data as specific type - returms true if is the correct type
		bool get(int& i);
		bool get(string& s);
		bool get(double& d);
		bool get(rpc_array*& ap);
		bool get(rpc_struct*& mp);
		// Returns the data as specific type
		int get_int();
		string get_string();
		double get_double();
		rpc_array* get_array();
		rpc_struct* get_struct();
		// Convert the item to a textual format
		string print_item();
		// Set the data as int or bool
		void set(int i, rpc_data_t Type);
		// Set the data as a string or byte encoded string or a date/time
		void set(string s, rpc_data_t Type);
		void set(double d);
		void set(rpc_array* ap);
		void set(rpc_struct* mp);

	protected:
		// The data
		rpc_data_t type_;
		// Its representation in specific type
		int i_;
		string s_;
		double d_;
		rpc_array* array_;
		rpc_struct* struct_;

	};
#endif 