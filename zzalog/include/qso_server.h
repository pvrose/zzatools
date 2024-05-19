#pragma once
#include "modems.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

class server_grp :
    public Fl_Group 
{
    public:
        server_grp(int X, int Y, int W, int H, const char* L = nullptr);
        ~server_grp();

        void modem(modem_t t);
        modem_t modem();
        void load_values();
        void create_form();
        void save_values();
        void enable_widgets();

        static void cb_bn_listen(Fl_Widget* w, void* v);
        static void cb_bn_connect(Fl_Widget* w, void* v);
        
    protected:
        Fl_Light_Button* bn_listening_;
        Fl_Light_Button* bn_connect_;

        modem_t modem_;

};

class qso_server:
    public Fl_Group

{
public:
    qso_server(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_server();

    void load_values();
    void create_form();
    void save_values();
    void enable_widgets();



protected:

    Fl_Box* rig_;
    server_grp* fldigi_;
    server_grp* wsjtx_;
};
