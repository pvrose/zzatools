#pragma once

#include <string>
#include <map>

#include <FL/Fl_Group.H>


class password_input;
class Fl_Input;
class Fl_Int_Input;
class Fl_Light_Button;
class Fl_Check_Button;
class Fl_Radio_Light_Button;
class Fl_Button;
class Fl_Tabs;
class filename_input;
class file_viewer;

const std::string FLDIGI = "FLDigi";
const std::string WSJTX = "WSJT-X";

//! How the app needs to access the rig.
enum app_rig_class_t {
    NO_CONNECTION,      //!< App applies to all rigs
    AUDIO_ONLY,         //!< App applies to the rig without needing CAT access
    AUDIO_CAT           //!< App requires CAT access to rig
};

//! The app related data
struct app_data_t {
    std::string name;                            //!< The name of the app.
    bool server{ false };                        //!< ZZALOG acts as a log server.
    app_rig_class_t rig_class{ NO_CONNECTION };  //!< How the app requires rig access.
    std::map<std::string, std::string> commands; //!< The commands for each rig.
    bool admin{ false };                         //!< The command needs to be run in administrator mode.
    bool can_disable{ false };                   //!< The app can be disconnected (Rig = NONE).
    bool (*has_server)() { nullptr };            //!< Function to call to see if server is active.
    bool (*has_data)() { nullptr };              //!< Function to call to see if server has request.
    std::string address{ "" };                   //!< Network address.
    int port_num{ 0 };                           //!< Network port number.
};

//! \brief The class displays and controls the status of a single modem application using
//! ZZALOG as a log server
class app_grp :
    public Fl_Group 
{

    public:
        //! Constructor.

        //! \param X horizontal position within host window
        //! \param Y vertical position with hosr window
        //! \param W width 
        //! \param H height
        //! \param L label
        app_grp(int X, int Y, int W, int H, const char* L = nullptr);
        //! Destructor.
        ~app_grp();

        //! Instantiate component widgets.
        void create_form();
        //! Configure widgets when data changes.
        void enable_widgets();

        //! Copy pointer to the configuration data for the app.
        void set_data(app_data_t* data);

        //! Callback from "Listen" button that initiates the socket to the modem app.
        static void cb_bn_listen(Fl_Widget* w, void* v);
        //! Callback from "Connect" button to launch the modem app.
        static void cb_bn_connect(Fl_Widget* w, void* v);
        //! Callback from input for command name to launch app.
        static void cb_ip_app(Fl_Widget* w, void* v);
        //! Callback from radio buttons changing rig_class.
        static void cb_bn_class(Fl_Widget* w, void* v);
        //! Callback from "Server" button: toggles whether ZZALOG acts as server.
        static void cb_bn_server(Fl_Widget* w, void* v);
        //! Callback frim "Adm" button to toggle administartor mode.
        static void cb_bn_admin(Fl_Widget* w, void* v);
        //! Callback to delete the application data.
        static void cb_bn_delete(Fl_Widget* w, void* v);
        //! Callback from undo button to toggle can_disable.
        static void cb_bn_disable(Fl_Widget* w, void* v);
        //! Callback from input for command name disable app name.
        static void cb_ip_disable(Fl_Widget* w, void* v);
        //! Callback from input for admin mode password.
        static void cb_ip_passw(Fl_Widget* w, void* v);
        //! callback from "show" to launch file_editor to show and edit script.
        static void cb_show_script(Fl_Widget* w, void* v);
        //! Callback from input for network address.
        static void cb_ip_nwaddr(Fl_Widget* w, void* v);
        //! Callback from input for network port.
        static void cb_ip_nwport(Fl_Widget* w, void* v);
        
    protected:

        //! Returns rig name.
        const char* rig_id();

 
        // Widgets
        filename_input* ip_app_name_;        //!< Input for app command name.
        Fl_Light_Button* bn_listening_;      //!< Light button indicates listening.
        Fl_Light_Button* bn_connect_;        //!< Light button to launch command.
        Fl_Light_Button* bn_server_;         //!< Light buuton indicates server mode.
        Fl_Group* radio_class_;              //!< Group radio buttons bn_common_, bn_rig_nocat_ bn_rig_cat_.
        Fl_Radio_Light_Button* bn_common_;   //!< Radio button - app configuration common.
        Fl_Radio_Light_Button* bn_rig_nocat_;//!< Radio button - app configuration affected by audio.
        Fl_Radio_Light_Button* bn_rig_cat_;  //!< Radio button - app configuration affected by CAT and audio
        Fl_Button* bn_rig_;                  //!< Button displaying rig and connections.
        Fl_Light_Button* bn_admin_;          //!< Light button controls admin. mode.
        password_input* ip_passw_;           //!< Input for admin mode password.
        Fl_Button* bn_delete_;               //!< Button to undo app.
        Fl_Light_Button* bn_disable_;        //!< Light button controls has undo command.
        filename_input* ip_disable_app_;     //!< Input for undo command.
        Fl_Button* bn_show_script_;          //!< Button to open editor for launch command.
        Fl_Button* bn_show_script2_;         //!< Button to open editor for undo command.
        Fl_Input* ip_nw_address_;            //!< Input for network address in server mode.
        Fl_Int_Input* ip_nw_port_;           //!< Input for network port in server mode.

        //! Configuration data for this app.
        app_data_t* app_data_;
};

//! This class displays and controls all the interfaces to the modem apps.
 
//! It has a number of common controls and a setof tabs one for each app_grp.
class qso_apps:
    public Fl_Group

{
public:
    //! Constructor.
    
    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qso_apps(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~qso_apps();

    //! Overide of Fl_Group::handle that takes focus to accept keybaord F1 for launching user guide.
    virtual int handle(int event);

    //! Load configuration data from apps.json
    void load_values();
    //! Instantiate component widgets.
    void create_form();
    //! Save configuration data to settings.
    void save_values();
    //! Configure component widgets after data changes.
    void enable_widgets();
    //! Add the server links from configuration \p data.
    void add_servers(app_data_t* data);
    //! Delete the application: removes data from configuration and removes app_grp \p w.
    void delete_app(app_grp* w);
    //! Returns the file_editor displaying the command script.
    file_viewer* viewer();
    //! Returns network address for the app \p app_name.
    std::string network_address(std::string app_name);
    //! Returns network port number for the app \p app_name.
    int network_port(std::string app_name);

    //! Callback from "New" button - creates a new item in the configuration data.
    void static cb_bn_new(Fl_Widget* w, void* v);
    //! Callback from "New" input.
    void static cb_ip_new(Fl_Widget* w, void* v);
    //! Callback when changing tabs - reformats the labels.
    void static cb_tabs(Fl_Widget* w, void* v);



protected:

    //! Add all the tabs
    void create_tabs(std::string name = "");

    //! ADjust the size of the widget after creating the component widgets.
    void adjust_size();

    Fl_Button* bn_new_;       //!< Button to create new app configuration.
    Fl_Input* ip_new_;        //!< Inputfor name of new application.

   // Tabbed std::set of app_grp
    Fl_Tabs* tabs_;           //!< Tabs, one for each app.

    // File viewer
    file_viewer* viewer_;     //!< File viewr window.

    //! The application data - items mapped by name of app.
    std::map<std::string, app_data_t*> apps_data_; 

    //! Name of new app.
    std::string new_name_;
    //! Read from settings, the index of the default tab to display.
    int default_tab_;
  
};
