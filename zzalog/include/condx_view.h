#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <FL/Fl_Group.H>

// Widgets
class Fl_Box;
class Fl_Image;
class Fl_Output;
class Fl_Tabs;

//! Solar data
struct solar_data {
    std::string    update;                 //!< Time last update.
    int            solar_flux_index;       //!< SFI: indication of F-layer ionisation.
    int            A_index;                //!< Daily-averaged geomagnetic intensity.
    int            K_index;                //!< Instantaneous geomagnetic intensity.
//    int            K_nanotesla;            //!< Geomagnetic intensity in nT (nanotesla).
    std::string    x_ray;                  //!< Strength of X-radiation.
    int            num_sunpsots;           //!< Normalised number of sunspots.
    double         He_strengh;             //!< Strength of helium emission at 30.4 nm.
    int            proton_flux;            //!< Density of protons in solar wind.
    int            electron_flux;          //!< Density of electrons in solar wind.
    std::string    aurora;                 //!< Indication of F-level ionisation leading to aurorae.
    double         aurora_latitude;        //!< Minimum latitude of aurora borealis.
    double         solar_wind_vel;         //!< Velocity of solar wind in km/s.
    double         mag_delta;              //!< Magnetic field disruption (Bz).
    std::map<std::string, std::map<std::string, std::string> > hf_forecasts;
                                           //!< HF band forecasts (map band->day/night->condx).
    std::map<std::string, std::string> vhf_forecasts; 
                                           //!< VHF band forecasts (map mode->band/loc->condx).
    std::string    mag_field;              //!< Level of acivity in geomagnetic field.
    std::string    solar_noise;            //!< Background solar noise (S-points)

};
//! Class to display solar-terrestrial data curtesy of N0NBH
class condx_view :
    public Fl_Group
{
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    condx_view(int X, int Y, int W, int H, const char* L = nullptr);

    //! Destructor
    ~condx_view();

    //! Override of handle to get F1
    virtual int handle(int e);

    //! Load data
    bool load_data();

    //! Instantiate component widgets
    void create_form();

    //! Enable widgets - copy data to them
    void enable_widgets();

    //! Callback
    static void cb_tabs(Fl_Widget* w, void* v);

    //! Timer
    static void cb_ticker(void* w);

protected:
    //! Fetch new data (greater than 1 hour old)
    bool fetch_data(std::stringstream& ss);

    //! Fetch solar image
    bool load_image();

    // Create individual tabs
    void create_solar(int x, int y, int w, int h);        //!< Create "Solar" tab
    void create_geomag(int x, int y, int w, int h);       //!< Create "Geomag:" tab
    void create_hf(int x, int y, int w, int h);           //!< Create "HF" tab
    void create_vhf(int x, int y, int w, int h);          //!< Create "VHF" tab
    void create_image(int x, int y, int w, int h);        //!< Create "Image" tab

    //! Data 
    solar_data* data_;

    //! Solar image - usually a GIF
    Fl_Image* solar_image_;

    // Components
    Fl_Tabs* w_tabs_;         //!< 5 tabs: Solar data, Geomag data, HF f/c, VHF f/c, Solar image.
    // Solar data
    Fl_Group* g_solar_;       //!< Solar data tab
    Fl_Output* w_sfi_;        //!< Solar flux index.
    Fl_Output* w_aix_;        //!< A-index.
    Fl_Output* w_kix_;        //!< K-index.
    Fl_Output* w_xray_;       //!< X-ray intensity.
    Fl_Output* w_sunspots_;   //!< Sunspot count.
    Fl_Output* w_helium_;     //!< Helium (30.4 nm) intensity.
    Fl_Output* w_proton_;     //!< Proton flux.
    Fl_Output* w_electron_;   //!< Electron flux.
    Fl_Output* w_wind_;       //!< Solar wind.
    // Geomagnetic data
    Fl_Group* g_geo_;         //!< Geomagnetic data tab
    Fl_Output* w_aurora_;     //!< Auroral intensity.
    Fl_Output* w_aur_lat_;    //!< Auroral latitude (degress North).
    Fl_Output* w_bz_;         //!< Bz value.
    Fl_Output* w_geomag_;     //!< Geomagentic field acivity.
    Fl_Output* w_noise_;      //!< Background solar noise.
    // HF Band conditions
    Fl_Group* g_hf_;          //!< HF band conditions tab
    std::map<std::string, Fl_Box*> w_band_names_;
                              //!< Row headers for bands  
    Fl_Group* g_day_;         //!< Group of daytime widgets.
    std::map<std::string, Fl_Output*> w_day_condx_;
                              //!< Daytime HF band conditions.
    Fl_Group* g_night_;       //!< Group of nighttime widgets.
    std::map<std::string, Fl_Output*> w_night_condx_;
                              //!< Nighttime HF band conditions.
    // VHF Band phenomena
    Fl_Group* g_vhf_;         //!< VHF phenomena tab
    std::map<std::string, Fl_Output*> w_vhf_phenomena_;
                              //!< VHF band phenomena.
    // Solar image
    Fl_Group* g_image_;       //!< Image tab
    Fl_Box* w_image_;         //!< Solar image.
    // Last updated
    Fl_Box* w_updated_;       //!< Date last updated.

};

