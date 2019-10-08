/* Types and constants for use in search and extraction related classes
*/

#ifndef __SEARCH__
#define __SEARCH__

#include <string>

using namespace std;

namespace zzalog {

	// extract match condition - values match radio button position in dialog
	enum search_cond_t : int {
		XC_DXCC = 0,           // records match on DXCC by code or nickname
		XC_GEO = 1,            // records match on sub-division of DXCC by nickname
		XC_CQZ = 2,            // records match on CQ Zone number
		XC_ITUZ = 3,           // records match on ITU Zone number
		XC_CONT = 4,           // records match on 2-letter continent code
		XC_SQ2 = 5,            // records match on first two letters of grid-square locator
		XC_SQ4 = 6,            // records match on first 4 letters of grid-square locator
		XC_CALL = 7,           // records match on callsign (usually used with regular expressions)
		XC_UNFILTERED = 8,     // all records
		XC_FIELD = 9           // records match on specified ADIF field name
	};

	// extract data combination mode - order used for radio button
	enum search_combi_t : int {
		XM_NEW = 0,            // new extraction, discard previous data
		XM_AND = 1,            // use only records that are in this and previous extractions
		XM_OR = 2              // merge the two sets of extractions
	};

	// extract criteria data structure
	struct search_criteria_t {
		search_cond_t condition;       // Match condi 
		bool by_regex;                  // use regular expression matching
		bool by_dates;                  // match records between two dates inclusively
		string from_date;               // Start date 
		string to_date;                 // End date
		string band;                    // match only records on this band (can be ANY)
		string mode;                    // match only records with this mode (can be ANY)
		bool confirmed_eqsl;            // match only records confirmed by eQSL
		bool confirmed_lotw;            // match only records confirmed by LotW
		bool confirmed_card;            // match only records confirmed by card
		search_combi_t combi_mode;     // Combination mode
		bool negate_results;            // use records that do not match criteria
		string field_name;              // Field name for criterion = FIELD
		string pattern;                 // Matching pattern

		search_criteria_t() :
			condition(XC_DXCC),
			by_regex(false),
			by_dates(false),
			from_date(""),
			to_date(""),
			band("Any"),
			mode("Any"),
			confirmed_eqsl(false),
			confirmed_lotw(false),
			confirmed_card(false),
			combi_mode(XM_NEW),
			negate_results(false),
			field_name(""),
			pattern("") {};
		search_criteria_t(search_cond_t a, bool b, bool c, string d, string e, string f, string g, bool h, bool i, bool j, search_combi_t k, bool l, string m, string n) :
			condition(a),
			by_regex(b),
			by_dates(c),
			from_date(d),
			to_date(e),
			band(f),
			mode(g),
			confirmed_eqsl(h),
			confirmed_lotw(i),
			confirmed_card(j),
			combi_mode(k),
			negate_results(l),
			field_name(m),
			pattern(n) {};
	};

}

#endif