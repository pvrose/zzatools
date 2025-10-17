#pragma once

#include "contest_scorer.h"

#include <nlohmann/json.hpp>

//! This file contains the JSON enumerated type conversions
NLOHMANN_JSON_SERIALIZE_ENUM(contest_scorer::ct_status, {
	{ contest_scorer::NO_CONTEST, "No contest" },
	{ contest_scorer::FUTURE, "Upcoming contest"},
	{ contest_scorer::ACTIVE, "Working contest"},
	{ contest_scorer::PAUSED, "Paused contest"},
	{ contest_scorer::PAST, "After contest "}
	}
)


