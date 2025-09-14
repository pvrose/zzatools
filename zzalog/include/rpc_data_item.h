#ifndef __RPC_DATA_ITEM__
#define __RPC_DATA_ITEM__

#include <string>
#include <map>
#include <vector>
#include <list>





	//! XML-RPC Request/Response - XML element types
	enum rpc_element_t {
		XRP_NONE,               //!< Not in an element - at top-level
		XRP_METHODCALL,         //!<  methodCall  - requesting paticular method
		XRP_METHODNAME,         //!<  methodName  - name of method
		XRP_METHODRESPONSE,     //!<  methodResponse  - response element
		XRP_FAULT,              //!<  fault  - error message
		XRP_PARAMS,             //!<  params  - start of parameter section    
		XRP_PARAM,              //!<  param  - individual parameter
		XRP_VALUE,              //!<  value  - individual data item
		XRP_ARRAY,              //!<  array  - array of items
		XRP_DATA,               //!<  data  - start of array items
		XRP_STRUCT,             //!<  struct  - hashed array of items
		XRP_MEMBER,             //!<  member  - individual entry in hash array
		XRP_NAME,               //!<  name  - key to item in hash array
		XRP_INT,                //!<  int  or  i4  - 32-bit integer item
		XRP_BOOLEAN,            //!<  Boolean  - 1 or 0
		XRP_DOUBLE,             //!<  double  - 64-bit floating point
		XRP_STRING,             //!<  std::string>
		XRP_DATETIME,           //!<  dateTime.iso8601  - Dates in ISO8601 format: YYYYMMDDTHH:MM:SS
		XRP_BASE64,             //!<  base64  - Binary information encoded as Base 64, as defined in RFC 2045
	};

	//! XML-RPC data types
	enum rpc_data_t {
		XRT_EMPTY,              //!< Empty data std::set
		XRT_INT,                //!< I4
		XRT_BOOLEAN,            //!< Y/N
		XRT_DOUBLE,             //!< double
		XRT_STRING,             //!< String (ASCII)
		XRT_DEFAULT,            //!< String (ASCII) by default of no type
		XRT_BYTES,              //!< store as std::string after compacting the data
		XRT_DATETIME,           //!< Date/time - store as std::string
		XRT_ARRAY,              //!< Array of data types
		XRT_STRUCT              //!< Struct
	};

	//! This class describes an Remote Procedure Call data item 
	class rpc_data_item
	{
	public:
		// Compound RPC data-types
		//! RPC Array of data items
		typedef std::vector<rpc_data_item*> rpc_array;
		//! RPC Structure
		typedef std::map<std::string, rpc_data_item*> rpc_struct;
		//! RPC List
		typedef std::list<rpc_data_item*> rpc_list;

	public:
		//! Constructor.
		rpc_data_item();
		//! Destructor.
		~rpc_data_item();

		//! Returns the data type of this data item
		rpc_data_t type();
		// Get the data as specific type - returms true if is the correct type
		bool get(int32_t& i);        //!< Receives item as a 32-bit integer, returns false if not integer
		bool get(std::string& s);         //!< Receives item as a std::string, returns false if not a std::string
		bool get(double& d);         //!< Receives item as a double, returns false if not floating point
		bool get(rpc_array*& ap);    //!< Receives iten as an array, returns false if not an array.
		bool get(rpc_struct*& mp);   //!< Receives item as a structure, returns false if not a structure.
		// Returns the data as specific type
		int32_t get_int();           //!< Returns item as a 32-bit integer
		std::string get_string();         //!< Returns item as a std::string
		double get_double();         //!< Returns item as a double-precision value
		rpc_array* get_array();      //!< Returns item as an array
		rpc_struct* get_struct();    //!< Returns item as a structure

		//! Convert the item to a textual format
		std::string print_item();
		//! Set the data as int or bool
		void set(int32_t i, rpc_data_t Type);
		//! Set the data as a std::string or byte encoded std::string or a date/time
		void set(std::string s, rpc_data_t Type);
		void set(double d);          //!< Set the item as a double-precision value.
		void set(rpc_array* ap);     //!< Set the item as an array
		void set(rpc_struct* mp);    //!< Set the item as a structure.

	protected:
		//! The type of data
		rpc_data_t type_;
		//! Its representation as a 32-bit integer
		int32_t i_;
		//! Its representation as a std::string
		std::string s_;
		//! Its representation as a double-precison value
		double d_;
		//! Its representation as an array
		rpc_array* array_;
		//! Its representation as a structure
		rpc_struct* struct_;

	};
#endif 