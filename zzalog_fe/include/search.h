/* Types and constants for use in search and extraction related classes
*/

#ifndef __SEARCH__
#define __SEARCH__

#include <nlohmann/json.hpp>

#include <string>

	//! extract match condition - values match position of radio button in search_dialog.
	enum search_cond_t : int {
		XC_DXCC = 0,       //!< records match on DXCC by code or nickname
		XC_CQZ,            //!< records match on CQ Zone number
		XC_ITUZ,           //!< records match on ITU Zone number
		XC_CONT,           //!< records match on 2-letter continent code
		XC_SQ2,            //!< records match on first two letters of grid-square locator
		XC_SQ4,            //!< records match on first 4 letters of grid-square locator
		XC_CALL,           //!< records match on callsign (usually used with regular expressions)
		XC_UNFILTERED,     //!< all records
		XC_FIELD,          //!< records match on specified ADIF field name
		XC_MAXIMUM         //!< Value for use as a limit
	};

	//! extract data combination mode - order used for radio button
	enum search_combi_t : int {
		XM_NEW = 0,        //!< new extraction, discard previous data
		XM_AND = 1,        //!< use only records that are in this and previous extractions
		XM_OR = 2          //!< merge the two sets of extractions
	};

	//! extract comparison operator
	enum search_comp_t : int {
		XP_REGEX = 0,      //!< Compare by regular expression
		XP_NE,             //!< Compare if not equal
		XP_LT,             //!< Compare if less than
		XP_LE,             //!< Compare if less than or equal
		XP_EQ,             //!< Compare if equal
		XP_GE,             //!< Compare if greater than or equal
		XP_GT,             //!< Compare if greater than
	};

	//! extract criteria data structure
	struct search_criteria_t {
		search_cond_t condition;        //!< Match condition
		search_comp_t comparator;       //!< How to compare data
		bool by_dates;                  //!< match records between two dates inclusively
		std::string from_date;               //!< Start date 
		std::string to_date;                 //!< End date
		std::string band;                    //!< match only records on this band (can be ANY)
		std::string mode;                    //!< match only records with this mode (can be ANY)
		bool confirmed_eqsl;            //!< match only records confirmed by eQSL
		bool confirmed_lotw;            //!< match only records confirmed by LotW
		bool confirmed_card;            //!< match only records confirmed by card
		search_combi_t combi_mode;      //!< Combination mode
		std::string field_name;              //!< Field name for criterion = FIELD
		std::string pattern;                 //!< Matching pattern
		std::string my_call;                 //!< match only records with this value of station callsign

		//! Default Constuctor
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
		//! Value constructor.
		search_criteria_t(search_cond_t a, search_comp_t b, bool c, std::string d, std::string e, std::string f, std::string g, bool h, bool i, bool j, search_combi_t k, std::string m, std::string n, std::string o) :
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


	//! JSON serialisation for search_cond_t
	NLOHMANN_JSON_SERIALIZE_ENUM(search_cond_t, {
		{ XC_DXCC, "DXCC" },
		{ XC_CQZ, "CQ Zone" },
		{ XC_ITUZ, "ITU Zone" },
		{ XC_CONT, "Continent" },
		{ XC_SQ2, "2-character Gridsquare" },
		{ XC_SQ4, "4-character Gridsquare" },
		{ XC_CALL, "Callsign" },
		{ XC_UNFILTERED, "All records" },
		{ XC_FIELD, "Named Field" }
		}
	)

	//! JSON serialisation for search_combi_t
	NLOHMANN_JSON_SERIALIZE_ENUM(search_combi_t, {
		{ XM_NEW, "New search" },
		{ XM_AND, "AND" },
		{ XM_OR, "OR" }
		}
	)

	//! JSON serialisation for search_comp_t
	NLOHMANN_JSON_SERIALIZE_ENUM(search_comp_t, {
		{ XP_REGEX, "Regex" },
		{ XP_NE, "Not equal" },
		{ XP_LT, "Less than"  },
		{ XP_LE, "Less than or equal" },
		{ XP_EQ, "Equal" },
		{ XP_GE, "Greater than or equal" },
		{ XP_GT, "Greater than" }
		}
	)


#endif