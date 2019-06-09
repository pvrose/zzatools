#ifndef __POWER_MATRIX__
#define __POWER_MATRIX__

#include <vector>
#include <map>
#include <string>

using namespace std;

namespace zzalog {
	// This class provides a container for the power matrix for a specific rig
	// and the calculations necessary to interpolate power from drive
	class power_matrix
	{

	public:
		power_matrix();
		power_matrix(string rig);
		~power_matrix();
		// Returns the power for the specific band and drive-level
		int power(string band, int drive);
		// Returns the number of rows (bands)
		vector<string> bands();
		// Returns the map for a band
		map<int, int>* get_map(string band);
		// Delete map
		void delete_rig();
		// Add band
		void add_band(string band);
		// Delete band
		bool delete_band(string band);

	protected:
		// The map
		map<string, map<int, int>* > map_;
		// Current rig
		string rig_;
	};

}
#endif