#pragma once

#include <string>
#include <map>

#include <FL/Fl_Group.H>

using namespace std;
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

const string FLDIGI = "FLDigi";
const string WSJTX = "WSJT-X";

// App rig class
enum app_rig_class_t {
    ALL_RIGS,           // App applies to all rigs
    RIG_NO_CAT,         // App applies to the rig without needing CAT access
    RIG_CAT             // App requires CAT access to rig
};

// The app related data
struct app_data_t {
    string name;        // The name of the app
    bool server{ false };        // ZZALOG acts as a log server
    app_rig_class_t rig_class{ ALL_RIGS };     // Same command used for all rigs
    map<string, string> commands;   
                        // The commands for each rig
    bool admin{ false };         // The command needs to be run in administrator mode
    bool can_disable{ false };       // The app can be disconnected (Rig = NONE)
    bool (*has_server)() { nullptr };   // Function to call to see if serever is active
    bool (*has_data)() { nullptr };     // Function to call to see if server has request
    string address{ "" };        // Network address 
    int port_num{ 0 };
};

// Displays and controls the status of a single modem apps using zzalog as a log server
class app_grp :
    public Fl_Group 
{

    public:
        app_grp(int X, int Y, int W, int H, const char* L = nullptr);
        ~app_grp();

        // Load settimgs
        void load_values();
        // Create widgets
        void create_form();
        // Save settings
        void save_values();
        // Configure widgets
        void enable_widgets();

        void set_data(app_data_t* data);

        // Callback to initiate the socket to the modem app
        static void cb_bn_listen(Fl_Widget* w, void* v);
        // Callback to launch the modem app
        static void cb_bn_connect(Fl_Widget* w, void* v);
        // Callback to save app name
        static void cb_ip_app(Fl_Widget* w, void* v);
        // Callback for changing rig_class
        static void cb_bn_class(Fl_Widget* w, void* v);
        // Callback for changing server
        static void cb_bn_server(Fl_Widget* w, void* v);
        // Call back for needing admin
        static void cb_bn_admin(Fl_Widget* w, void* v);
        // Callback to delete the application
        static void cb_bn_delete(Fl_Widget* w, void* v);
        // Callback to set can_disable
        static void cb_bn_disable(Fl_Widget* w, void* v);
        // Callback to save disable app name
        static void cb_ip_disable(Fl_Widget* w, void* v);
        // Callback on enetring password
        static void cb_ip_passw(Fl_Widget* w, void* v);
        // callback to show script
        static void cb_show_script(Fl_Widget* w, void* v);
        // Callback for network address
        static void cb_ip_nwaddr(Fl_Widget* w, void* v);
        // Callack for network port
        static void cb_ip_nwport(Fl_Widget* w, void* v);
        
    protected:

        // Create rig name
        const char* rig_id();

 
        // Widgets
        filename_input* ip_app_name_;
        Fl_Light_Button* bn_listening_;
        Fl_Light_Button* bn_connect_;
        Fl_Light_Button* bn_server_;
        Fl_Group* radio_class_;
        Fl_Radio_Light_Button* bn_common_;
        Fl_Radio_Light_Button* bn_rig_nocat_;
        Fl_Radio_Light_Button* bn_rig_cat_;
        Fl_Button* bn_rig_;
        Fl_Light_Button* bn_admin_;
        password_input* ip_passw_;
        Fl_Button* bn_delete_;
        Fl_Light_Button* bn_disable_;
        filename_input* ip_disable_app_;
        Fl_Button* bn_show_script_;
        Fl_Button* bn_show_script2_;
        Fl_Input* ip_nw_address_;
        Fl_Int_Input* ip_nw_port_;

        app_data_t* app_data_;
};

// Displays and controls all the interfaces to the modem apps
class qso_apps:
    public Fl_Group

{
public:
    qso_apps(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_apps();

    virtual int handle(int event);

    // Load settings
    void load_values();
    // Create widgets
    void create_form();
    // Save settings
    void save_values();
    // Configure widgets
    void enable_widgets();
    // Add the server links
    void add_servers(app_data_t* data);
    // Delete the application
    void delete_app(app_grp* w);
    // Get the viewer
    file_viewer* viewer();
    // Get network address
    string network_address(string app_name);
    // Get network port number
    int network_port(string app_name);

    // Callback
    void static cb_bn_new(Fl_Widget* w, void* v);
    // Input tab
    void static cb_ip_new(Fl_Widget* w, void* v);
    // CAllback tabs
    void static cb_tabs(Fl_Widget* w, void* v);



protected:

    // Add all the tabs
    void create_tabs(string name = "");

    void adjust_size();

    // New app
    Fl_Button* bn_new_;
    Fl_Input* ip_new_;

   // Tabbed set of app_grp
    Fl_Tabs* tabs_;

    // File viewer
    file_viewer* viewer_;

    // The application data
    map<string, app_data_t*> apps_data_; 

    // string new app name
    string new_name_;

    int default_tab_;
  
};
