#pragma once

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
		string mode;         //!< Mode used (ADIG SUBMODE)
		string rpt_rcvd;     //!< Report received
		string rpt_sent;     //!< Report sent;
	} qso;
	bool has_contest;        //!< Has contest data
	
	//!< Contest data
	struct {
		string_list exch_rcvd;      //!< Received contest exchange
		string_list exch_sent;      //!< Sent contest exchange
	} contest;

	//!< QSL server type
	enum qsl_server_t {
		EQSL,                       //!< eQSL.cc QSL status
		LOTW,                       //!< LotW QSL status
		CLUBLOG,                    //!< Clublog QSL ststus
		QRZ,                        //!< QRZ.com
		PAPER                       //!< Physical paper QSLs
	};
	//!< QSL status
	struct qsl_status_t {

	};
	

	};

		};
	};
};

