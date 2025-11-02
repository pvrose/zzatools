#include "search.h"

#include <nlohmann/json.hpp>

//! to_json for search_criteria_t
void to_json(nlohmann::json& j, const search_criteria_t& p) {
	j = nlohmann::json{
		{"Criterion", p.condition},
		{"By Comparison", p.comparator},
		{"By Dates", p.by_dates},
		{"From date", p.from_date},
		{"To Date", p.to_date},
		{"Band", p.band},
		{"Mode", p.mode},
		{"Confirmed eQSL", p.confirmed_eqsl},
		{"Confirmed LotW", p.confirmed_lotw},
		{"Confirmed Card", p.confirmed_card},
		{"Combine Extract", p.combi_mode},
		{"Field", p.field_name},
		{"Condition", p.pattern},
		{"Station Callsign", p.my_call}
	};
}
//! from_json for search_criteria_t
void from_json(const nlohmann::json& j, search_criteria_t& p) {
	j.at("Criterion").get_to(p.condition);
	j.at("By Comparison").get_to(p.comparator);
	j.at("By Dates").get_to(p.by_dates);
	j.at("From date").get_to(p.from_date);
	j.at("To Date").get_to(p.to_date);
	j.at("Band").get_to(p.band);
	j.at("Mode").get_to(p.mode);
	j.at("Confirmed eQSL").get_to(p.confirmed_eqsl);
	j.at("Confirmed LotW").get_to(p.confirmed_lotw);
	j.at("Confirmed Card").get_to(p.confirmed_card);
	j.at("Combine Extract").get_to(p.combi_mode);
	j.at("Field").get_to(p.field_name);
	j.at("Condition").get_to(p.pattern);
	j.at("Station Callsign").get_to(p.my_call);
}
