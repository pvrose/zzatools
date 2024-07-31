#pragma once

#include <string>
#include <map>

#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tabs.H>

using namespace std;

// The app related data
struct app_data_t {
    string name;        // The name of the app
    bool server;        // ZZALOG acts as a log server
    bool is_common;     // Same command used for all rigs
    map<string, string> commands;   
                        // The commands for each rig
    bool (*has_server)();   // Function to call to see if serever is active
    bool (*has_data)();     // Function to call to see if server has request
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
        // Callback for changing params
        static void cb_bn_app(Fl_Widget* w, void* v);
        
    protected:

        // Widgets
        Fl_Input* ip_app_name_;
        Fl_Light_Button* bn_listening_;
        Fl_Light_Button* bn_connect_;
        Fl_Light_Button* bn_server_;
        Fl_Light_Button* bn_common_;
        Fl_Button* bn_rig_;

        app_data_t* app_data_;
};

// Displays and controls all the interfaces to the modem apps
class qso_apps:
    public Fl_Group

{
public:
    qso_apps(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_apps();

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

    // Callback
    void static cb_bn_new(Fl_Widget* w, void* v);
    // CAllback tabs
    void static cb_tabs(Fl_Widget* w, void* v);



protected:

    // Add all the tabs
    void create_tabs(string name = "");

    // New app
    Fl_Button* bn_new_;
    Fl_Input* ip_new_;

   // Tabbed set of app_grp
    Fl_Tabs* tabs_;

    // The application data
    map<string, app_data_t*> apps_data_; 

    // string new app name
    string new_name_;
  
};
