#pragma once

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tabs.H>

using namespace std;

// List of modem apps used in a couple of objects
enum app_t {
    FLDIGI,
    WSJTX,
    NUMBER_APPS
};

const string APP_NAMES[NUMBER_APPS] = { "FlDigi", "WSJT-X"};


// Displays and controls the status of a single modem apps using zzalog as a log server
class app_grp :
    public Fl_Group 
{
    public:
        app_grp(int X, int Y, int W, int H, const char* L = nullptr);
        ~app_grp();

        // Set/Get the modem being controlled
        void modem(app_t t);
        app_t modem();
        // Load settimgs
        void load_values();
        // Create widgets
        void create_form();
        // Save settings
        void save_values();
        // Configure widgets
        void enable_widgets();

        // Callback to initiate the socket to the modem app
        static void cb_bn_listen(Fl_Widget* w, void* v);
        // Callback to launch the modem app
        static void cb_bn_connect(Fl_Widget* w, void* v);
        // Callback to save app name
        static void cb_ip_app(Fl_Widget* w, void* v);
        
    protected:

        // Widgets
        Fl_Input* ip_app_name_;
        Fl_Light_Button* bn_listening_;
        Fl_Light_Button* bn_connect_;

        // Modem app
        app_t modem_;
        // App name
        string app_name_;

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



protected:

    // Box displaying current rig connection
    Fl_Box* rig_;
    // Tabbed set of app_grp
    Fl_Tabs* tabs_;
    // Controls for FLDIGI
    app_grp* apps_[NUMBER_APPS];
  
};
