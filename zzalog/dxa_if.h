// This code uses windows specific inter-application communication
#ifdef _WIN32

#ifndef __DXA_FORM__
#define __DXA_FORM__

#include "record.h"
#include "fields.h"
#include "view.h"

#include <set>
#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Window.H>

#include <atlbase.h>
#include <atlcom.h>


//!! DX Atlas type library
#import "C:\Program Files (x86)\Afreet\DX Atlas\DxAtlas.exe"

using namespace std;
using namespace zzalib;

namespace zzalog {

	// This class provides an interface to control DxAtlas and to display on it the locations of records
	class dxa_if :
		public Fl_Window,
		// inheritance for DxAtlas communication interface
		public ::IDispEventSimpleImpl<2, dxa_if, &__uuidof(::DxAtlas::IDxAtlasEvents)>

	{
	protected:
		// DX Atlas - record selection radio button
		enum qsos_bn_t {
			AQ_NONE,       // Display nothing
			AQ_CURRENT,    // Display selected record
			AQ_ALL,        // Display all records
			AQ_SEARCH,     // Display extracted records
			AQ_DAYS,       // Display most recent days
			AQ_QSOS        // Display most recent QSOs
		};
		// DX Atlas - Colour selection radio button
		enum colour_bn_t {
			AC_NONE,       // Diplay all selected records in black
			AC_BANDS,      // Seperate colour per band
			AC_LOGMODE,    // Separate colour per logged (ADIF-defined) mode/submode
			AC_AWARDMODE   // Separate colour per DXCC defined mode
		};
		// Centre map position
		enum centre_t {
			HOME,          // Home location
			SELECTED,      // Selected QSO
			GROUP,         // Centre of chosen QSOs
			ZERO,          // zero longitude/latitude
		};

	public:
		dxa_if();
		virtual ~dxa_if();

		// Public methods
		// Standard methods inherited from page_dialog - need to be written for each
		// Load values from settings_
		void load_values();
		// Used to create the form
		void create_form();
		// Used to write settings back
		void save_values();
		// Enable widgets
		void enable_widgets();
		// something has changed in the book - usually record 1 is to be selected, record_2 usage per view
		void update(hint_t hint);
		// Callbacks

		// QSOs choice
		static void cb_ch_qsos(Fl_Widget* w, void* v);
		// Colour mode choice
		static void cb_ch_colour(Fl_Widget* w, void* v);
		// Centre choice
		static void cb_ch_centre(Fl_Widget* w, void* v);
		// Re-Centre button
		static void cb_bn_centre(Fl_Widget* w, void* v);
		// Add/subtract SWL reports
		static void cb_ch_swlen(Fl_Widget* w, void* v);
		// Open 
		static void cb_bn_start_stop(Fl_Widget* w, void* v);
		// Location choice
		static void cb_ch_locn(Fl_Widget* w, void* v);
		// Number days/QSOs input
		static void cb_ip_number(Fl_Widget* w, void* v);
		// Colour button click
		static void cb_bn_colour(Fl_Widget* w, void* v);
		// All colour button click
		static void cb_bn_all(Fl_Widget* w, void* v);


	public:

		// DxAtlas callbacks
		HRESULT __stdcall cb_map_changed(enum ::DxAtlas::EnumMapChange change_kind);
		HRESULT __stdcall cb_map_clicked(float latitude, float longitude);
		HRESULT __stdcall cb_mouse_moved(float latitude, float longitude);
		HRESULT __stdcall cb_exit_requested();

		// Inter-app callback map
		BEGIN_SINK_MAP(dxa_if)
			SINK_ENTRY_INFO(2, __uuidof(::DxAtlas::IDxAtlasEvents), 0x7, &dxa_if::cb_mouse_moved, new _ATL_FUNC_INFO({ CC_STDCALL, VT_EMPTY, 2,{ VT_R4, VT_R4 } }))
			SINK_ENTRY_INFO(2, __uuidof(::DxAtlas::IDxAtlasEvents), 0x6, &dxa_if::cb_map_clicked, new _ATL_FUNC_INFO({ CC_STDCALL, VT_EMPTY, 2,{ VT_R4, VT_R4 } }))
			SINK_ENTRY_INFO(2, __uuidof(::DxAtlas::IDxAtlasEvents), 0x3, &dxa_if::cb_map_changed, new _ATL_FUNC_INFO({ CC_STDCALL, VT_EMPTY, 1,{ VT_I4 } }))
			SINK_ENTRY_INFO(2, __uuidof(::DxAtlas::IDxAtlasEvents), 0x1, &dxa_if::cb_exit_requested, new _ATL_FUNC_INFO({ CC_STDCALL, VT_EMPTY, 0, {} }))
		END_SINK_MAP()

		// Get button Colour
		Fl_Color button_colour(int button_num);

	protected:
		// Get details of the home location for the selected record
		void home_location();
		// Connect to DxAtlas
		bool connect_dxatlas();
		// Disconnect from DxAtlas
		void disconnect_dxatlas(bool dxatlas_exit);
		// Initialise map
		void initialise_map();
		// Get records to display
		void get_records();
		// Allocate colours - and reset filter
		void allocate_colours();
		// Create colour_buttons
		void create_colour_buttons();
		// totally handle colours - calls the above
		void get_colours(bool reset);
		// Draw pins
		void draw_pins();
		// draw home flag
		void draw_home_flag();
		// Is the point displayed
		bool is_displayed(record_num_t record_num);
		// Convert colours
		DxAtlas::EnumColor convert_colour(Fl_Color colour);
		// Update location widgets
		void update_loc_widgets();
		// Centre map
		void centre_map();
		// On specific centre
		void centre_map(lat_long_t centre);
		// Zoom to centre
		void zoom_centre(lat_long_t centre, bool full);


	protected:
		// Radio button valur for QSOs to display
		qsos_bn_t qso_display_;
		// Selector for how to colour pins by - enum
		colour_bn_t atlas_colour_;
		//!! DX Atlas automation object
		DxAtlas::IAtlasPtr atlas_;
		// List of records to display
		set<record_num_t> records_to_display_;
		// List of records being displayed - i.e. have colour selected
		set<record_num_t> records_displayed_;
		// Name of the home location of the selected record
		string location_name_;
		// Latitude of home station...
		double home_lat_;
		// ...and in degrees,minutes and seconds
		string home_lat_dms_;
		// Longitude of home station...
		double home_long_;
		// ...and in degrees,minutes and seconds
		string home_long_dms_;
		// Include SWL reports on map
		bool include_swl_;
		// List of texts to use in the colour buttons
		vector<string> colours_used_;
		// Latitude for the centre of the map
		double centre_lat_;
		// Longitude for the centre of the map
		double centre_long_;
		// width of the DxAtlas window
		int atlas_width_;
		// height of the DxAtlas window
		int atlas_height_;
		// left edge position of the DxAtlas window
		int atlas_left_;
		// top edge position of the DxAtlas window
		int atlas_top_;
		// The layer to be used for call displays
		DxAtlas::ICustomLayerPtr call_layer_;
		// Flag to remember that any change was initiated from the app
		bool is_my_change_;
		// Locator for source location
		string locator_;
		// Most recent count
		unsigned int most_recent_count_;
		// Position of colour window
		int window_left_;
		int window_top_;
		// list of locations
		vector<string> locations_;
		// Zoom value
		float zoom_value_;
		// Centre mode
		centre_t centre_mode_;
		// Selected location
		lat_long_t selected_locn_;
		// Extrems of display range
		double westernmost_;
		double easternmost_;
		double northernmost_;
		double southernmost_;
		// Map properties
		int prefixes_;
		int cq_zones_;
		int itu_zones_;
		int grid_squares_;
		int lat_lon_grid_;
		int bearing_distance_;
		DxAtlas::EnumProjection projection_;
		// Colour button enables
		vector<bool> colour_enables_;

		Fl_Group* colour_grp_;
		// Colour buttons - instanced in colour_win_
		vector<Fl_Widget*> colour_bns_;
		Fl_Widget* most_recent_ip_;
		Fl_Widget* start_stop_bn_;
		Fl_Widget* locator_op_;
		Fl_Widget* lat_dms_op_;
		Fl_Widget* lon_dms_op_;
		Fl_Widget* centre_ch_;
	};

}
#endif // __DXA_FORM__

#endif // _WIN32