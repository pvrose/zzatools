#ifndef __POWER_MATRIX__
#define __POWER_MATRIX__

#include <vector>
#include <map>
#include <string>

using namespace std;

namespace zzalog {
	// This class provides a container for the power matrix for a specific rig
	// and the calculations necessary to interpolate power from drive

	typedef map<int, double> power_lut;

	class power_matrix
	{

	public:
		// Defualt constructor
		power_matrix();
		~power_matrix();
		// initialises the matrix from the rig data.
		void initialise(string rig);
		// Returns the power for the specific band and drive-level
		double power(string band, int drive);
		// Look up a single value
		double look_up(string band, int drive);
		// Returns the number of rows (bands)
		vector<string> bands();
		// Returns the map for a band
		power_lut* get_map(string band);
		// Delete map
		void delete_rig();
		// Add band
		void add_band(string band);
		// Delete band
		bool delete_band(string band);

	protected:
		// The map
		map<string, power_lut* > map_;
		// Current rig
		string rig_;
	};

}
#endif