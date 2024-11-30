/* Types and constants for use in search and extraction related classes
*/

#ifndef __SEARCH__
#define __SEARCH__

#include <string>

using namespace std;



	// extract match condition - values match radio button position in dialog
	enum search_cond_t : int {
		XC_DXCC = 0,       // records match on DXCC by code or nickname
		XC_CQZ,            // records match on CQ Zone number
		XC_ITUZ,           // records match on ITU Zone number
		XC_CONT,           // records match on 2-letter continent code
		XC_SQ2,            // records match on first two letters of grid-square locator
		XC_SQ4,            // records match on first 4 letters of grid-square locator
		XC_CALL,           // records match on callsign (usually used with regular expressions)
		XC_UNFILTERED,     // all records
		XC_FIELD,          // records match on specified ADIF field name
		XC_MAXIMUM         // Value for use as a limit
	};

	// extract data combination mode - order used for radio button
	enum search_combi_t : int {
		XM_NEW = 0,            // new extraction, discard previous data
		XM_AND = 1,            // use only records that are in this and previous extractions
		XM_OR = 2              // merge the two sets of extractions
	};

	// extract comparison operator
	enum search_comp_t : int {
		XP_REGEX,              // Compare by regular expression
		XP_NE,                 // Compare if not equal
		XP_LT,                 // Compare if less than
		XP_LE,                 // Compare if less than or equal
		XP_EQ,                 // Compare if equal
		XP_GE,                 // Compare if greater than or equal
		XP_GT,                 // Compare if greater than
	};

	// extract criteria data structure
	struct search_criteria_t {
		search_cond_t condition;        // Match condi 
		search_comp_t comparator;       // How to compare data
		bool by_dates;                  // match records between two dates inclusively
		string from_date;               // Start date 
		string to_date;                 // End date
		string band;                    // match only records on this band (can be ANY)
		string mode;                    // match only records with this mode (can be ANY)
		bool confirmed_eqsl;            // match only records confirmed by eQSL
		bool confirmed_lotw;            // match only records confirmed by LotW
		bool confirmed_card;            // match only records confirmed by card
		search_combi_t combi_mode;     // Combination mode
		string field_name;              // Field name for criterion = FIELD
		string pattern;                 // Matching pattern
		string my_call;                 // match only records with this value of my_call

		search_criteria_t() :
			condition(XC_DXCC),
			comparator(XP_EQ),
			by_dates(false),
			from_date(""),
			to_date(""),
			band("Any"),
			mode("Any"),
			confirmed_eqsl(false),
			confirmed_lotw(false),
			confirmed_card(false),
			combi_mode(XM_NEW),
			field_name(""),
			pattern(""),
			my_call("Any") {};
		search_criteria_t(search_cond_t a, search_comp_t b, bool c, string d, string e, string f, string g, bool h, bool i, bool j, search_combi_t k, string m, string n, string o) :
			condition(a),
			comparator(b),
			by_dates(c),
			from_date(d),
			to_date(e),
			band(f),
			mode(g),
			confirmed_eqsl(h),
			confirmed_lotw(i),
			confirmed_card(j),
			combi_mode(k),
			field_name(m),
			pattern(n),
			my_call(o) {};
	};

#endif