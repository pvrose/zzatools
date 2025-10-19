#include "cty_element.h"

#include <nlohmann/json.hpp>

#include <cmath>

using json = nlohmann::json;

cty_element::cty_element() { type_ = CTY_UNSPECIFIED; }
cty_element::~cty_element() {}

// Merge a similar element into this one - 
// return clash bit if fields are different
// update this field if it is not specified in this
cty_element::error_t cty_element::merge(cty_element* elem) {
	error_t result = CE_OK;
	if (cq_zone_ <= 0) {
		cq_zone_ = elem->cq_zone_;
	}
	else {
		if (elem->cq_zone_ > 0) {
			result |= CE_CQ_CLASH;
		}
	}
	if (itu_zone_ <= 0) {
		itu_zone_ = elem->itu_zone_;
	}
	else {
		if (elem->itu_zone_ > 0) {
			result |= CE_ITU_CLASH;
		}
	}
	if (continent_ == "") {
		continent_ = elem->continent_;
	}
	else {
		if (elem->continent_ != "") {
			result |= CE_CONT_CLASH;
		}
	}
	if (coordinates_.is_nan()) {
		coordinates_ = elem->coordinates_;
	}
	else {
		if (!elem->coordinates_.is_nan()) {
			result |= CE_COORD_CLASH;
		}
	}
	if (name_ == "") {
		name_ = elem->name_;
	}
	else {
		if (elem->name_ != "") {
			result |= CE_NAME_CLASH;
		}
	}
	if (deleted_ != elem->deleted_) {
		result |= CE_DEL_CLASH;
	}
	return result;
}

// Returns true if elem's valiidty overlaps this validity
bool cty_element::time_overlap(cty_element* elem) {
	time_scope lhs = time_validity_;
	time_scope rhs = elem->time_validity_;
	if (lhs.start == "*") lhs.start = "00000000";
	if (lhs.finish == "*") lhs.finish = "99999999";
	if (rhs.start == "*") rhs.start = "00000000";
	if (rhs.finish == "*") rhs.finish = "99999999";
	if (rhs.start >= lhs.start && rhs.start <= lhs.finish) return true;
	if (rhs.finish >= lhs.start && rhs.finish <= lhs.finish) return true;
	if (lhs.start >= rhs.start && lhs.start <= rhs.finish) return true;
	if (lhs.finish >= rhs.start && lhs.finish <= rhs.finish) return true;
	return false;
}

// Return true if this validity wholly contains elem's validity
bool cty_element::time_contains(cty_element* elem) {
	time_scope lhs = time_validity_;
	time_scope rhs = elem->time_validity_;
	if (lhs.start == "*") lhs.start = "00000000";
	if (lhs.finish == "*") lhs.finish = "99999999";
	if (rhs.start == "*") rhs.start = "00000000";
	if (rhs.finish == "*") rhs.finish = "99999999";
	if (rhs.start >= lhs.start && rhs.finish <= lhs.finish) return true;
	return false;
}

// Return true if supplied time is within this validity
bool cty_element::time_contains(std::string when) {
	time_scope lhs = time_validity_;
	if (lhs.start == "*") lhs.start = "00000000";
	if (lhs.finish == "*") lhs.finish = "99999999";
	if (when >= lhs.start && when <= lhs.finish) return true;
	return false;
}

// Output streaming operator
std::ostream& operator<<(std::ostream& os, const cty_element& rhs) {
	os << "(" << rhs.time_validity_.start << "-" << rhs.time_validity_.finish << ") " <<
		"Name=" << rhs.name_ << ", CQ=" << rhs.cq_zone_ << ", ITU=" << rhs.itu_zone_ <<
		", Cont=" << rhs.continent_ << ", Coords=" << degrees_to_dms(rhs.coordinates_.latitude, true) <<
		", " << degrees_to_dms(rhs.coordinates_.longitude, false) <<
		(rhs.deleted_ ? " (DELETED)" : "");
	return os;
}


cty_entity::cty_entity() : cty_element() {
	type_ = CTY_ENTITY;
	nickname_ = "";
}
cty_entity::~cty_entity() {}

cty_element::error_t cty_entity::merge(cty_element* elem) {
	error_t result = cty_element::merge(elem);
	cty_entity* entry = (cty_entity*)elem;
	if (nickname_ == "") {
		nickname_ = entry->nickname_;
	}
	else {
		if (entry->nickname_ != "") {
			result |= CE_OTHER_CLASH;
		}
	}
	if (filters_.size() == 0) {
		filters_ = entry->filters_;
	}
	else {
		if (entry->filters_.size()) {
			result |= CE_OTHER_CLASH;
		}
	}
	return result;
}

std::ostream& operator<<(std::ostream& os, const cty_entity& rhs) {
	os << (cty_element)rhs << " Nickname=" << rhs.nickname_ << ", " << rhs.filters_.size() << " Filters";
	return os;
}

cty_prefix::cty_prefix() : cty_element() {
	type_ = CTY_PREFIX;
}
// Ddstructor - delete children
cty_prefix::~cty_prefix() { }

std::ostream& operator<<(std::ostream& os, const cty_prefix& rhs) {
	os << (cty_element)rhs;
	return os;
}

cty_exception::cty_exception() : cty_element() { 
	type_ = CTY_EXCEPTION;
	exc_type_ = EXC_INVALID;
}
cty_exception::~cty_exception() {}

std::ostream& operator<<(std::ostream& os, const cty_exception& rhs) {
	os << (cty_element)rhs << (rhs.exc_type_ == cty_exception::EXC_INVALID ? " (INVALID)" : "");
	return os;
}

cty_filter::cty_filter() : cty_element () { 
	type_ = CTY_FILTER;
	filter_type_ = FT_USAGE;
	pattern_ = "";
	nickname_ = "";
	reason_ = "";
}
cty_filter::~cty_filter() {}

std::ostream& operator<<(std::ostream& os, const cty_filter& rhs) {
	os << (cty_element)rhs << ", Filter=" << rhs.pattern_ << ", Reason=" << rhs.reason_ <<
		", Nickname=" << rhs.nickname_;
	return os;
}

cty_geography::cty_geography() : cty_filter() {
	type_ = CTY_GEOGRAPHY;
	filter_type_ = FT_GEOGRAPHY;

	province_ = "";
}
cty_geography::~cty_geography() { }

std::ostream& operator<<(std::ostream& os, const cty_geography& rhs) {
	os << (cty_filter)rhs << ", PAS=" << rhs.province_;
	return os;
}

// JSON Serialisation from cty_element
void to_json(json& j, const cty_element& e) {
	j["DXCC"] = e.dxcc_id_;
	if (e.name_.length()) j["Name"] = e.name_;
	if (e.time_validity_.start != "*") j["Start"] = e.time_validity_.start;
	if (e.time_validity_.finish != "*") j["Finish"] = e.time_validity_.finish;
	if (e.cq_zone_ != -1) j["CQ Zone"] = e.cq_zone_;
	if (e.itu_zone_ != -1) j["ITU Zone"] = e.itu_zone_;
	if (e.continent_.length()) j["Continent"] = e.continent_;
	if (!std::isnan(e.coordinates_.latitude)) j["Latitude"] = e.coordinates_.latitude;
	if (!std::isnan(e.coordinates_.longitude)) j["Latitude"] = e.coordinates_.longitude;
	if (e.deleted_) j["Deleted"] = e.deleted_;
	if (e.filters_.size()) {
		json jf;
		for (auto f : e.filters_) {
			switch (f->filter_type_) {
			case cty_filter::FT_GEOGRAPHY:
				jf.push_back(*(cty_geography*)f);
				break;
			case cty_filter::FT_USAGE:
				jf.push_back(*f);
				break;
			default:
				break;
			}
		}
		j["Filters"] = jf;
	}
}
// JSON Serialisation from cty_element
void from_json(const json& j, cty_element& e) {
	if (j.find("DXCC") != j.end()) j.at("DXCC").get_to(e.dxcc_id_);
	else e.dxcc_id_ = -1;
	if (j.find("Name") != j.end()) j.at("Name").get_to(e.name_);
	else e.name_ = "";
	if (j.find("Start") != j.end()) j.at("Start").get_to(e.time_validity_.start);
	else e.time_validity_.start = "*";
	if (j.find("Finish") != j.end()) j.at("Finish").get_to(e.time_validity_.finish);
	else e.time_validity_.finish = "*";
	if (j.find("CQ Zone") != j.end()) j.at("CQ Zone").get_to(e.cq_zone_);
	else e.cq_zone_ = -1;
	if (j.find("ITU_Zone") != j.end()) j.at("ITU_Zone").get_to(e.itu_zone_);
	else e.itu_zone_ = -1;
	if (j.find("Continent") != j.end()) j.at("Continent").get_to(e.continent_);
	else e.continent_ = "";
	if (j.find("Latitude") != j.end()) j.at("Latitude").get_to(e.coordinates_.latitude);
	else e.coordinates_.latitude = nan("");
	if (j.find("Longitude") != j.end()) j.at("Longitude").get_to(e.coordinates_.longitude);
	else e.coordinates_.longitude = nan("");
	if (j.find("Deleted") != j.end()) j.at("Deleted").get_to(e.deleted_);
	else e.deleted_ = false;
	if (j.find("Filters") != j.end()) {
		for (auto f : j.at("Filters")) {
			cty_filter::filter_t t = cty_filter::FT_NOT_USED;
			if (f.find("Filter Type") != f.end()) {
				f.at("Filter Type").get_to(t);
			}
			if (t == cty_filter::FT_GEOGRAPHY) {
				cty_geography* filter = new cty_geography;
				f.get_to(*filter);
				e.filters_.push_back(filter);
			}
			else {
				cty_filter* filter = new cty_filter;
				f.get_to(*filter);
				e.filters_.push_back(filter);
			}
		}
	}
}
// JSON Serialisation of cty_entity
void to_json(json& j, const cty_entity& e) {
	j = json((cty_element&)e);
	if (e.nickname_.length()) j["Nickname"] = e.nickname_;
}
// JSON Serialisation to cty_entity
void from_json(const json& j, cty_entity& e) {
	from_json(j, (cty_element&)e);
	if (j.find("Nickname") != j.end()) j.at("Nickname").get_to(e.nickname_);
	else e.nickname_ = "";
}
// JSON Serialisation of cty_prefix
void to_json(json& j, const cty_prefix& e) {
	j = json((cty_element&)e);
}
// JSON Serialisation to cty_prefix
void from_json(const json& j, cty_prefix& e) {
	from_json(j, (cty_element&)e);
}
// JSON Serialisation of cty_exception::exc_type_t
NLOHMANN_JSON_SERIALIZE_ENUM(cty_exception::exc_type_t, {
	{ cty_exception::EXC_INVALID, "Invalid Operation"},
	{ cty_exception::EXC_OVERRIDE, "Override Entity"}
	}
)

// JSON Serialisation of cty_exception
void to_json(json& j, const cty_exception& e) {
	j = json((cty_element&)e);
	j["Exception Type"] = e.exc_type_;
}
// JSON Serialiser to cty_exception
void from_json(const json& j, cty_exception& e) {
	from_json(j, (cty_element&)e);
	if (j.find("Exception Type") != j.end()) j.at("Exception Type").get_to(e.exc_type_);
	else e.exc_type_ = cty_exception::EXC_INVALID;
}
// JSON Serialisation of cty_filter::filter_t 
NLOHMANN_JSON_SERIALIZE_ENUM(cty_filter::filter_t, {
	{ cty_filter::FT_NOT_USED, "No filter"},
	{ cty_filter::FT_GEOGRAPHY, "Geography"},
	{ cty_filter::FT_USAGE, "Usage"}
	}
)

// JSON Serialisation of cty_filter
void to_json(json& j, const cty_filter& e) {
	j = json((cty_element&)e);
	j["Filter Type"] = e.filter_type_;
	if (e.pattern_.length()) j["Pattern"] = e.pattern_;
	if (e.nickname_.length()) j["Nickname"] = e.nickname_;
	if (e.reason_.length()) j["Reason"] = e.reason_;
}
// JSON Serialisation to cty_filter
void from_json(const json& j, cty_filter& e) {
	from_json(j, (cty_element&)e);
	if (j.find("Filter Type") != j.end()) j.at("Filter Type").get_to(e.filter_type_);
	else e.filter_type_ = cty_filter::FT_NOT_USED;
	if (j.find("Pattern") != j.end()) j.at("Pattern").get_to(e.pattern_);
	else e.pattern_ = "";
	if (j.find("Nickname") != j.end()) j.at("Nickname").get_to(e.nickname_);
	else e.nickname_ = "";
	if (j.find("Reason") != j.end()) j.at("FReason").get_to(e.reason_);
	else e.reason_ = "";
}
// JSON Serialisation from cty_geography
void to_json(json& j, const cty_geography& e) {
	j = json((cty_filter&)e);
	j["Primary Subdivision"] = e.province_;
}
// JSON Serialisation to cty_geography
void from_json(const json& j, cty_geography& e) {
	from_json(j, (cty_filter&)e);
	if (j.find("Primary Subdivision") != j.end()) j.at("Primary Subdivision").get_to(e.province_);
	else e.province_ = "";
}

