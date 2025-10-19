#pragma once

#include "cty_element.h"

#include "utils.h"

#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <list>
#include <map>
#include<ostream>
#include <string>



class record;

//! This class provides a wrapper for all the callsign exception data.
class cty_data
{

public:

	//! Source of the data - std::set by type()
	enum cty_type_t : uint8_t {
		ADIF = 0,          //!< Data from ADIF Specification.
		CLUBLOG,           //!< Data from Clublog.org.
		COUNTRY_FILES,     //!< Data from country-files.com.
		DXATLAS            //!< Data from dxatlas.com.
	};


	//! Database structure.
	struct all_data {
		//! All the entities - indexed by dxcc_id.
		std::map < int, cty_entity* > entities;
		//! All the entity level prefixes - indexed by starting std::string.
		std::map < std::string, std::list<cty_prefix*> > prefixes;
		//! All the exceptions - indexed by callsign.
		std::map < std::string, std::list<cty_exception*> > exceptions;
	};

protected:

	//! Data currently being loaded.
	cty_type_t type_ = ADIF;

public:

	//! Parse source
	enum parse_source_t {
		INVALID,            //!< Callsign marked invalid.
		NO_DECODE,          //!< Not decoded.
		EXCEPTION,          //!< Callsign in entity that is not its normal decode.
		ZONE_EXCEPTION,     //!< Callsign in a zone other than the default for the entity.
		PREVIOUS,           //!< Callsign previously decoded.
		DEFAULT             //!< Parsing based on default for the callsign.
	};

	//! Constructor.
	cty_data(bool reload = false);
	//! Destructor.
	virtual ~cty_data();

	// Return various fields of entity
	std::string nickname(record* qso);  //!< Returns the nickname for the entity in the \p QSO. 
	std::string name(record* qso);      //!< Returns the name of the entity in the \p QSO.
	std::string continent(record* qso); //!< Returns the continent of the entity in the \p QSO.
	int cq_zone(record* qso);      //!< Returns the CQ Zone of the callsign in the \p QSO.
	int itu_zone(record* qso);     //!< Returns the ITU Zone of the callsign in the \p QSO.
	// Get location
	lat_long_t location(record* qso); //!< Returns the longitude and latitude of the station in the \p QSO.
	//! Update record based on parsing
	
	//! \param qso QSO record to update.
	//! \param my_call update the "MY_...." fields rather than the other station's information. 
	//! \return true if successful, false if not.
	bool update_qso(record* qso, bool my_call = false);
	//! Get location details
	
	//! \param qso QSO record to parse.
	//! \return text information about parsing for displaying in a tooltip.
	std::string get_tip(record* qso);
	//! Parsing source
	
	//! \param qso QSO to parse.
	//! \return indicates how the callsign was parsed. 
	parse_source_t get_source(record* qso);
	//! Returns the DXCC identifier of the entity worked in \p qso. 
	int entity(record* qso);
	//! Returns geography information relating to the callsign worked in \p qso.
	std::string geography(record* qso);
	//! Returns usage information relating to the callsign worked in \p qso.
	std::string usage(record* qso);

	//! Returns the DXCC identifier for the entity with \p nickname.
	int entity(std::string nickname);
	//! Returns the entity nickname for the entity with DXCC identifier \p adif_id.
	std::string nickname(int adif_id);

	//! Add the entity \p entry to the database.
	void add_entity(cty_entity* entry);
	//! Add the prefix \p entry mapped by \p pattern to the database.
	void add_prefix(std::string pattern, cty_prefix* entry);
	//! Add the exception \p entry mapped by \p pattern to the database.
	void add_exception(std::string pattern, cty_exception* entry);
	//! Add the filter \p entry to the specified \p element in the database. 
	void add_filter(cty_element* element, cty_filter* entry);

	//! Returns the current output stream
	std::ostream& out() { return os_; };
	//! Returns the recorded timestamp for the data source by \p type.
	std::chrono::system_clock::time_point timestamp(cty_type_t type);
	//! Download the latest data from data source by \p type.
	
	//! Returns true if successful, false if not.
	bool fetch_data(cty_type_t type);
	//! Returns the version of the data source by \p type.
	std::string version(cty_type_t type);

protected:

	//! Load the data from the \p filename specified. 
	bool load_data(std::string filename);
	//! Delete data
	void delete_data(all_data* data);
	//! Returns the filename for the current data type.
	std::string get_filename();
	//! Merge imported data from latest source.
	void merge_data();
	//! Prepopulate from ADIF Specification.
	void load_adif_data();
	//! Find the entity, pattern and sub-patterns for the supplied QSO: updates internal attributes.
	void parse(record* qso);
	//! Use the attached \p suffix to "mutate" the \p call to parse eg W1ABC/2 type calls.
	void mutate_call(std::string& call, char suffix);
	//! Store json
	void store_json();
	//! Load JSON
	bool load_json();
	//! Load source data
	void load_sources();
	//! Find element that matches the call.
	
	//! \param call Callsign to match.
	//! \param when Date of QSO.
	//! \param matched_call Returns the part of the callsign that matches the element.
	//! \return The matching element: either an exception record or an entity.
	cty_element* match_pattern(std::string call, std::string when, std::string& matched_call);
	//! Find specific prefix element that matches call.
	
	//! \param call Callsign to match.
	//! \param when Date of QSO.
	//! \return The matching prefix record.
	cty_element* match_prefix(std::string call, std::string when);
	//! Find specific secondary filter that matches the call and type.
	
	//! \param element The starting point of the match search - usually an entity element or
	//! a previous filter for multi-layered filters.
	//! \param type Either FT_GEOGRAPHY or FT_USAGE.
	//! \param call The callsign to match.
	//! \param when The date of the QSO.
	cty_filter* match_filter(cty_element* element, cty_filter::filter_t type, std::string call, std::string when);

	//! Split \p call into call \p body and \p alt (alternate).
	void split_call(std::string call, std::string& alt, std::string& body);
	//! Dump database into a file - for checking data loaded cotrrectly.
	void dump_database();

	//! Returns Exception record for current parse result, nullptr if not an exception
	cty_exception* exception();
	//! Returns Prefix record for current parse result, nullptr of no prefix.
	cty_prefix* prefix();
	
	//! Get the system timestamp for the named \p filename.
	
	//! \param filename Filename.
	//! \return the system timestamp of the file.
	std::chrono::system_clock::time_point get_timestamp(std::string filename);

	//! Check the time stamp
	
	//! \param type The source of the data.
	//! \param days Age in days the filename is considered valid. A warning is raised if the file is older.
	void check_timestamp(cty_type_t type, int days);
	
	//! The result of a parse request.
	struct {
		//! The entity definition
		cty_entity* entity = nullptr;
		//! Either an exception or prefix
		cty_element* decode_element = nullptr;
		//! Selecetd geographic filter
		cty_geography* geography = nullptr;
		//! Usage filter
		cty_filter* usage = nullptr;
	} parse_result_;

	//! Previous callsign that was parsed, to avoid unnecessary re-parsing.
	std::string current_call_ = "";
	//! Previous QSO that was parsed.
	record* current_qso_ = nullptr;

	//! The country database.
	all_data* data_ = nullptr;

	//! The data being imported
	all_data* import_ = nullptr;

	//! Output stream for the merge report.
	std::ofstream os_;

	//! Warnings have been reported during data merge.
	bool report_warnings_ = false;
	//! Errors have been reported during data merge.
	bool report_errors_ = false;

	//! Mapping of data timestamps by data source.
	std::map<cty_type_t, std::chrono::system_clock::time_point> timestamps_;
	//! Time at start of loading.
	std::chrono::system_clock::time_point now_;
	//! Mapping of data versions by data source.
	std::map<cty_type_t, std::string> versions_;

};
//! Json Serilaisation from cty_data::all_data
void to_json(json& j, const cty_data::all_data& d);
//! JSON Serialisation to cty_data::all_data
void from_json(const json& j, cty_data::all_data& d);
