#ifndef __PFX_DATA__
#define __PFX_DATA__

#include "prefix.h"
#include "record.h"
#include "exc_data.h"

#include <map>
#include <unordered_set>
#include <string>
#include <regex>

using namespace std;





	// Callsign parsing type
	enum parse_t {
		PT_PLAIN,        // Callsign is unembellished - e.g. GM3ZZA
		PT_ROVING,       // Callsign has /A, /P or /M indicating the part before the last / can be parsed
		PT_MARITIME,     // Callsign has /MM indicating it should not be parsed - DXCC=0
		PT_COMPOSITE,    // Callsign has (most likely) a prefix indicating operating abroad - e.g. R3/GM3ZZA
		PT_COMPROVING,   // Composite and roving
		PT_COMPMARINE,   // Composite and /MM
		PT_SPECIAL,      // Callsign is prefix + > 1 number (plain only)
		PT_AREA,         // Callsign has /n indicating a different call-area in same entity - e.g. W1ABC/2
		PT_AREAROVING,   // Area and roving
		PT_UNPARSED      // None of the above
	};

	// Parse result
	enum parse_result_t {
		PR_UNCHANGED,          // Parse did not result in record changing
		PR_CHANGED,            // Parse was successful
		PR_ABANDONED           // Parse was stopped by user intervention
	};

	// regular expressions used in parsing
	//PT_PLAIN,        // Callsign is unembellished - e.g. GM3ZZA
	const basic_regex<char> REGEX_PLAIN("[A-Z0-9]+");
	//PT_ROVING,       // Callsign has /A, /P or /M indicating the part before the last / can be parsed
	const basic_regex<char> REGEX_ROVING("[A-Z0-9]+/[APM]");
	//PT_MARITIME,     // Callsign has /MM indicating it should not be parsed - DXCC=0
	const basic_regex<char> REGEX_MARITIME("[A-Z0-9]+/MM");
	//PT_COMPOSITE,    // Callsign has (most likely) a prefix indicating operating abroad - e.g. R3/GM3ZZA
	const basic_regex<char> REGEX_COMPOSITE("[A-Z0-9]+/[A-Z0-9]+");
	//PT_COMPROVING,   // Composite and roving
	const basic_regex<char> REGEX_COMPROVING("[A-Z0-9]+/[A-Z0-9]+/[APM]");
	//PT_COMPMARINE,   // Composite and /MM
	const basic_regex<char> REGEX_COMPMARINE("[A-Z0-9]+/[A-Z0-9]+/MM");
	//PT_SPECIAL,      // Callsign is prefix + > 1 number (plain only) or 4 or more suffix letters - Note these two require regex_search
	const basic_regex<char> REGEX_SPECIAL("^([BFGIKMNRUW][0-9][0-9]|[A-Z2-9][A-Z0-9][0-9][0-9])");
	const basic_regex<char> REGEX_SPECIAL4("[A-Z]{4,}$");
	//PT_AREA,         // Callsign has /n indicating a different call-area in same entity - e.g. W1ABC/2
	const basic_regex<char> REGEX_AREA("[A-Z0-9]+/[0-9]");
	//PT_AREAROVING,   // Area and roving
	const basic_regex<char> REGEX_AREAROVING("[A-Z0-9]+/[0-9]/[APM]");
	//PT_UNPARSED      // None of the above


	// This class is a container for the reference prefix database. It maps DXCC code number to
	// a prefix definition structure with all the information for that DXCC.
	// The original database has been produced by Alex Shovkoplyas, VE3NEA for use in DxAtlas which
	// he has made available. The inherited map container holds the database, indexed by DXCC code.
	class pfx_data : public map<unsigned int, prefix*>
	{
	public:
		pfx_data();
		~pfx_data();

		// public method
	public:
		// add the prefix to the map of nickname->prefix
		void add_pfx_to_nickname(prefix* prefix);
		// Get a prefix by nickname
		prefix* get_prefix(string nickname);
		// Get a prefix by DXCC code
		prefix* get_prefix(unsigned int dxcc_code);
		// Get a prefix by DXCC code and state/province code
		prefix* get_prefix(unsigned int dxcc_code, string state);
		// Get a prefix by analysing record
		prefix* get_prefix(record* record, bool special);
		// Get all the prefixes that a callsign is parsed to. Returns TRUE if any exist
		bool all_prefixes(record* record, vector<prefix*>* prefixes, bool special);
		// parse a record
		parse_result_t parse(record* record);
		// Update the ANT_AZ and DISTANCE fields based on record data or entity data
		bool update_bearing(record* record);
		// Return callsign parse type
		parse_t get_parse_type(string& callsign);
		// Generate tip text for parsing a call
		string get_tip(record* record);
		// Get exceptions
		exc_data* get_exceptions();

		//protected methods
	protected:
		// get the prefix data filename
		string get_file(bool force);
		// Get all the children that match callsign and date (geographic or special use)
		void get_children(vector<prefix*>& children, prefix* prefix, unsigned int dxcc_code, string& callsign, string& date, string& state, bool special);
		// Return true if the callsign matches the pattern in the prefix table
		bool match_callsign(prefix* prefix, string& callsign);
		// Delete an individual prefix
		void delete_prefix(prefix* prefix);
		// Update the DXCC - set the Country and Territory indices
		bool update_dxcc(record* record, prefix*& prefix, bool& query, string& reason, bool query_error = true);
		// Update the CQ Zone based on country and territory
		bool update_cq_zone(record* record, prefix* prefix, bool& query, string& reason);
		// Update the ITU Zone based on country and territory
		bool update_itu_zone(record* record, prefix* prefix, bool& query, string& reason);
		// Update the Continent based on country and territory
		bool update_continent(record* record, prefix* prefix, bool& query, string& reason);
		// Update DXCC, CONT, CQZ and ITUZ based on callsign - unless already set in ADIF
		bool update_geography(record* record, bool &query, string& reason, bool query_error = true);

		// protected attributes
	protected:
		// Prefix data table - indexed by prefix nickname
		map<string, prefix*> prefixes_by_nickname_;
		// Exception data
		exc_data* exceptions_;
	};
#endif

