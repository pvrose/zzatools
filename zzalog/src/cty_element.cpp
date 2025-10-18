#include "cty_element.h"

#include <nlohmann/json.hpp>

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
	j = json{
		{ "DXCC", e.dxcc_id_ },
		{ "Name", e.name_ },
		{ "Start", e.time_validity_.start },
		{ "Finish", e.time_validity_.finish },
		{ "CQ Zone", e.cq_zone_ },
		{ "ITU Zone", e.itu_zone_ },
		{ "Continent", e.continent_ },
		{ "Latitude", e.coordinates_.latitude },
		{ "Longitude", e.coordinates_.longitude },
		{ "Deleted", e.deleted_}
	};
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

// JSON Serialisation of cty_entity
void to_json(json& j, const cty_entity& e) {
	j = json((cty_element&)e);
	j["Nickname"] = e.nickname_;
}

// JSON Serialisation of cty_prefix
void to_json(json& j, const cty_prefix& e) {
	j = json((cty_element&)e);
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
	j["Pattern"] = e.pattern_;
	j["Nickname"] = e.nickname_;
	j["Reason"] = e.reason_;
}

// JSON Serialisation from cty_geography
void to_json(json& j, const cty_geography& e) {
	j = json((cty_filter&)e);
	j["Primary Subdivision"] = e.province_;
}

