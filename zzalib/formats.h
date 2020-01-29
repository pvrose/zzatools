/* Format enumerated types and strings
*/

#ifndef __FORMATS__
#define __FORMATS__

#include <string>

#include <FL/Fl_Widget.H>

using namespace std;

namespace zzalib {

	// record display format and save precision for frequency
	enum frequency_t : int {
		FREQ_Hz = 0,
		FREQ_Hz10 = 1,
		FREQ_Hz100 = 2,
		FREQ_kHz = 3,
		FREQ_kHz10 = 4,
		FREQ_kHz100 = 5,
		FREQ_MHz = 6
	};

	// Units used in displaying freaquencies.
	const string FREQ_FORMATS[3] = {
		"Hz", "kHz", "MHz"
	};

	// formats used in displaying frequencies with the precision defined as a frequency_t enum.
	const string FREQ_PRECISION[7] = {
		"%.6f", "%.5f", "%.4f", "%.3f", "%.2f", "%.1f", "%.0f"
	};

	// record display format for dates
	enum display_date_t {
		DATE_YYYYMMDD,
		DATE_YYYY_MM_DD,
		DATE_DD_MM_YYYY,
		DATE_MM_DD_YYYY,
		DATE_DD_MON_YYYY,
	};

	// Formats used for displaying dates
	const string DATE_FORMATS[5] = {
		"%Y%m%d", "%Y/%m/%d", "%d/%m/%Y", "%m/%d/%Y", "%d-%b-%Y"
	};

	// Formats used in a menu for displaying format options.
	const string DATE_FORMATS_MENU[5] = {
		"%Y%m%d", "%Y\\/%m\\/%d", "%d\\/%m\\/%Y", "%m\\/%d\\/%Y", "%d-%b-%Y"
	};

	// record display format for times
	enum display_time_t {
		TIME_HHMMSS,
		TIME_HHMM,
		TIME_LOGGED
	};

	// Menu items formats for displaying time
	const string TIME_FORMATS[3] = {
		"%H%M%S", "%H%M", "As Logged"
	};

	// Formats used for times
	const string TIME_PRECISION[2] = {
		"%H%M%S", "%H%M"
	};

	// record display format for power
	enum display_power_t : int {
		DPWR_Watt,
		DPWR_dBW
	};

	// Units used in displaying power.
	const string POWER_FORMATS[2] = {
		"W", "dBw"
	};

	// status display format for received signal strength
	enum display_signal_t : int {
		SIG_SPoints,
		SIG_dBm
	};

	// Units used in displaying signal strength
	const string SIGNAL_FORMATS[2] = {
		"S-points", "dBm"
	};

	// record save precision for power
	enum log_power_t : int {
		PWR_100mW,
		PWR_500mW,
		PWR_1W,
		PWR_5W,
		PWR_10W,
		PWR_50W,
		PWR_100W,
	};

	// Menu items for selecting power precision
	const string POWER_PRECISION[7] = {
		"100 mW", "500 mW", "1 W", "5 W", "10 W", "50 W", "100 W"
	};

}
#endif