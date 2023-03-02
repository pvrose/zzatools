#ifndef __PREFIX__
#define __PREFIX__

#include <string>
#include <unordered_set>
#include <vector>

using namespace std;



	// Prefix record type - defined by DxAtlas
	enum prefix_t {
		PX_UNDEFINED = 0,
		PX_DXCC_ENTITY = 1,
		PX_GEOGRAPHY = 2,
		PX_SPECIAL_USE = 3,
		PX_REMOVED_ENTITY = 4,
		PX_OLD_PREFIX = 5,
		PX_UNRECOGNISED = 6,
		PX_UNASSIGNED = 7,
		PX_OLD_GEOGRAPHY = 8,
		PX_CITY = 9,
		PX_TOP
	};

	// This class provides the data structure for a single prefix entry in the database
	struct prefix
	{
		// struct members
		prefix_t type_;			    	    // Record type
		double longitude_;					// Longitude of centre of area
		double latitude_;					// Latitude ditto
		string name_;						// Name of country/region/speciality
		string nickname_;					// Usual prefix as a nickname
		vector<unsigned int> cq_zones_;		// All the zones in the area
		vector<unsigned int> itu_zones_;	// ditto
		vector<string> continents_;			// All continents it's in
		string timezone_;					// Timezone(s) TODO: make an array
		unsigned int dxcc_code_;			// DXCC code
		string state_;				    	// ADIF province ID
		string valid_from_;					// date valid from
		string valid_to_;					// Date valid to
		vector<string> patterns_;			// matching patterns to parse calls
		prefix* parent_;				    // Pointer to parent
		vector<prefix*> children_;          // List of pointers to child prefix records
		unsigned int depth_;                // Depth of record

	public:
		// Constructor - default values
		prefix() {
			type_ = PX_UNASSIGNED;
			longitude_ = nan("");
			latitude_ = nan("");
			name_ = "";
			nickname_ = "";
			cq_zones_.clear();
			itu_zones_.clear();
			continents_.clear();
			timezone_ = "";
			dxcc_code_ = 0;
			state_ = "";
			valid_from_ = "";
			valid_to_ = "";
			patterns_.clear();
			parent_ = 0;
			children_.clear();
			depth_ = 0;
		}
		// DEstructor
		~prefix() {
			cq_zones_.clear();
			itu_zones_.clear();
			continents_.clear();
			patterns_.clear();
			// delete each child
			for (size_t i = 0; i < children_.size(); i++) {
				delete children_[i];
			}
			children_.clear();
		}
	};
#endif
