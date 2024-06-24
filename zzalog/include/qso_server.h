#pragma once
#include "modems.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

// Displays and controls the status of a single modem apps using zzalog as a log server
class server_grp :
    public Fl_Group 
{
    public:
        server_grp(int X, int Y, int W, int H, const char* L = nullptr);
        ~server_grp();

        // Set/Get the modem being controlled
        void modem(modem_t t);
        modem_t modem();
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
        
    protected:

        // Widgets
        Fl_Light_Button* bn_listening_;
        Fl_Light_Button* bn_connect_;

        // Modem app
        modem_t modem_;

};

// Displays and controls all the interfaces to the modem apps
class qso_server:
    public Fl_Group

{
public:
    qso_server(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_server();

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
    // Controls for FLDIGI
    server_grp* fldigi_;
    // Controls for WSJT-X
    server_grp* wsjtx_;
};
