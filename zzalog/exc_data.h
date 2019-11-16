#ifndef __EXC_DATA__
#define __EXC_DATA__

#include "record.h"

#include <string>
#include <ctime>
#include <map>
#include <list>

using namespace std;

namespace zzalog {

	// An exc_data entry - it will be mapped by callsign to a list of them
	struct exc_entry {
		string call;
		unsigned short adif_id;
		unsigned short cq_zone;
		string continent;
		double longitude;
		double latitude;
		time_t start;
		time_t end;

		exc_entry() {
			call = "";
			adif_id = 0;
			cq_zone = 0;
			continent = "";
			longitude = nan("");
			latitude = nan("");
			start = -1;
			end = -1;
		}
		ostream& store(ostream& out);
		istream& load(istream& in);
	};

	// An invalid entry
	struct invalid {
		string call;
		time_t start;
		time_t end;

		invalid() {
			call = "";
			start = -1;
			end = -1;
		}
		ostream& store(ostream& out);
		istream& load(istream& in);
	};

	class exc_data
	{
	public:
		exc_data();
		~exc_data();

		// Return the exc_data entry for the record (call and date) - nullptr if one doesn't exist
		exc_entry* is_exception(record* record);
		// Check timeliness of data
		bool data_valid(string filename);
		// Load data 
		bool load_data(string filename);
		// Return invalid operation
		bool is_invalid(record* record);
		// Allow the reader to access the data directlt
		friend class exc_reader;
		ostream& store(ostream& out);
		istream& load(istream& in);

	protected:
		// Get the filename {REFERENCE DIR}/cty.xml
		string get_filename();
		// Delete contents
		void delete_contents();

			// The exc_data data
		map < string, list<exc_entry*> > entries_;
		// Invalid operations 
		map<string, list<invalid*> > invalids_;
		// File created
		string file_created_;


		
	};

}

#endif

