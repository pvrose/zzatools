#pragma once

#include "utils.h"

#include "nlohmann/json.hpp"

#include <chrono>
#include <string>
#include <list>
#include <map>

using time_point = std::chrono::time_point<std::chrono::system_clock>;
using seconds = std::chrono::seconds;
using string = std::string;
using string_list = std::list<std::string>;

//! The basic QSO entry
struct zl4_entry
{
	//! Fields defining QSO
	struct {
		time_point start;    //!<  Start time in UTC
		seconds duration;    //!< Duration in seconds
		string callsign;     //!< Callsign of station worked
		bool use_freq;       //!< Frequency field is valid
		uint64_t frequency;  //!< Frequency in hertz
		string band;         //!< Band
		string mode;         //!< Mode used (ADIF SUBMODE)
		string rpt_rcvd;     //!< Report received
		string rpt_sent;     //!< Report sent;
	} qso;
	bool has_contest;        //!< Has contest data
	
	//! Contest data
	struct {
		string_list exch_rcvd;      //!< Received contest exchange
		string_list exch_sent;      //!< Sent contest exchange
	} contest;

	//! QSL server type
	enum qsl_server_t {
		EQSL,                       //!< eQSL.cc QSL status
		LOTW,                       //!< LotW QSL status
		CLUBLOG,                    //!< Clublog QSL ststus
		QRZ,                        //!< QRZ.com
		PAPER                       //!< Physical paper QSLs
	};
	//! QSL status
	struct qsl_status_t {
		bool not_wanted;          //!< QSL not wanted
		bool sent;               //!< QSL sent
		bool rcvd;               //!< QSL received	
		time_point sent_date;   //!< Date QSL was sent
		time_point rcvd_date;   //!< Date QSL was received
	};

	std::map<qsl_server_t, qsl_status_t> qsl_status; //!< QSL status map

	//! Geographical data
	struct {
		string country;          //!< Country name
		string county;           //!< County name
		string continent;        //!< Continent name
		string cq_zone;          //!< CQ zone
		string dxcc;            //!< DXCC entity
		string grid_square;     //!< Maidenhead grid square
		string itu_zone;        //!< ITU zone
		string state;           //!< State or province
		lat_long_t coords;      //!< Latitude/Longitude coordinates
	} geo;

	//! Operational data
	struct {
		string qth;                //!< QTH or location name
		string operator_callsign;  //!< Callsign of operator
		string station_callsign;   //!< Callsign of station used
	} ops;
};

void to_json(nlohmann::json& j, const zl4_entry& e);
void from_json(const nlohmann::json& j, zl4_entry& e);

NLOHMANN_JSON_SERIALIZE_ENUM(zl4_entry::qsl_server_t, {
	{zl4_entry::EQSL, "EQSL"},
	{zl4_entry::LOTW, "LOTW"},
	{zl4_entry::CLUBLOG, "CLUBLOG"},
	{zl4_entry::QRZ, "QRZ"},
	{zl4_entry::PAPER, "PAPER"}
	})

void to_adif(const zl4_entry& e, string s&);
void from_adif(const string& s, zl4_entry& e);

