#include "drawing.h"
#include "fllog_emul.h"
#include "wsjtx_handler.h"
#include "status.h"
#include "qso_apps.h"
#include "qso_manager.h"
#include "qso_rig.h"
#include "utils.h"
#include "callback.h"

#include <FL/Fl_Preferences.H>

using namespace std;

extern fllog_emul* fllog_emul_;
extern wsjtx_handler* wsjtx_handler_;
extern status* status_;
extern Fl_Preferences* settings_;

// Constructor for one set of modem controls
app_grp::app_grp(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
}

// Destructor
app_grp::~app_grp() {
}

// Load settings
void app_grp::load_values() {
}

// Save settings
void app_grp::save_values() {
}

// Create fthe widgets
void app_grp::create_form() {
    int curr_x = x() + GAP;
    int curr_y = y() + GAP;

    box(FL_BORDER_BOX);

    // Button to select common command for all rigs
    bn_common_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "All rigs");
    bn_common_->callback(cb_bn_app, &(app_data_->is_common));
    bn_common_->tooltip("Select if the same command is used for all rigs");

    curr_x += WBUTTON;

    // Button to act as a log server
    bn_server_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Server");
    bn_server_->callback(cb_bn_app, &(app_data_->server));
    bn_server_->tooltip("Select if ZZALOG acts as a log server for this app");

    curr_x = x() + GAP;
    curr_y += HBUTTON;

    // Button to display rig name
    bn_rig_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON);
    bn_rig_->when(FL_WHEN_NEVER);
    bn_rig_->box(FL_FLAT_BOX);
    bn_rig_->tooltip("Displays the current rig - or ALL");

    curr_x += WBUTTON;

    // Input to specify the command for invoking the app
    ip_app_name_ = new Fl_Input(curr_x, curr_y, WBUTTON * 2, HBUTTON);
    ip_app_name_->callback(cb_ip_app);
    ip_app_name_->tooltip("Enter the command to invoke the app for the selected rig");
    ip_app_name_->when(FL_WHEN_CHANGED);

    curr_y += HBUTTON;

    // Light to say that we are listening for the modem app to connect
    // Button will switch this on or off
    bn_listening_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Listen");
    bn_listening_->selection_color(FL_GREEN);
    bn_listening_->callback(cb_bn_listen);
    bn_listening_->tooltip("Listen or block messages from this modem");

    curr_x += WBUTTON;
    // Light to say we have received a packet from the app
    // Button will launch the mode app
    bn_connect_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
    bn_connect_->callback(cb_bn_connect);
    bn_connect_->tooltip("Launch the modem app");

    curr_y += HBUTTON + GAP;

    resizable(nullptr);
    size(w(), curr_y - y());

    end();
}

// Configure the widgets
void app_grp::enable_widgets() {
    qso_manager* mgr = ancestor_view<qso_manager>(this);
    const char* rig_name = mgr->rig_control()->label();
    // Update the various widgets
    bn_common_->value(app_data_->is_common);
    bn_server_->value(app_data_->server);
    bn_rig_->copy_label(rig_name);
    if (app_data_->commands.find(rig_name) != app_data_->commands.end()) {
        ip_app_name_->value(app_data_->commands.at(string(rig_name)).c_str());
    } else {
        ip_app_name_->value("");
    }
    if (app_data_->server) {
        if (app_data_->has_server && (*(app_data_->has_server))()) {
            bn_listening_->value(true);
            bn_listening_->activate();
            bn_connect_->activate();
            if (app_data_->has_data && (*(app_data_->has_data))()) {
                bn_connect_->value(true);
            } else {
                bn_connect_->value(false);
            }
        } else {
            bn_listening_->value(false);
            bn_listening_->activate();
            bn_connect_->deactivate();
        }
    } else {
        bn_listening_->deactivate();
        bn_connect_->activate();
    }
}

// Set the dataa for this app
void app_grp::set_data(app_data_t* data) {
    app_data_ = data;
    // Set button call backs to use the new data
    bn_common_->user_data(&app_data_->is_common);
    bn_server_->user_data(&app_data_->server);

    enable_widgets();

}

// Listen button callback (per modem)
// v is not used
void app_grp::cb_bn_listen(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    string server = that->label();
    if (server == "FLDigi") {
        if (fllog_emul_->has_server()) {
            fllog_emul_->close_server();
        } else {
            fllog_emul_->run_server();
        }
    }
    else if (server == "WSJT-X") {
        if (wsjtx_handler_->has_server()) {
            wsjtx_handler_->close_server();
        } else {
            wsjtx_handler_->run_server();
        }
    }
    that->enable_widgets();
}

// Start button callback
// v is not used
void app_grp::cb_bn_connect(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    qso_manager* mgr = ancestor_view<qso_manager>(that);
    const char* rig_name = mgr->rig_control()->label();
    if (that->app_data_->commands.find(rig_name) == that->app_data_->commands.end()) {
        char msg[128];
        snprintf(msg, sizeof(msg), "APPS: App %s does not know how to connect %s", 
            that->app_data_->name.c_str(), rig_name);
        status_->misc_status(ST_ERROR, msg);
    } else {
        string app = "bash -i -c " + that->app_data_->commands.at(rig_name);
        char msg[128];
        snprintf(msg, sizeof(msg), "DASH: Starting app: %s", app.c_str());
        status_->misc_status(ST_NOTE, msg);
        system(app.c_str());
    }
    that->enable_widgets();
  
}

// App typed in
void app_grp::cb_ip_app(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    string value;
    cb_value<Fl_Input, string>(w, &value);
    qso_manager* mgr = ancestor_view<qso_manager>(that);
    const char* rig_name = mgr->rig_control()->label();
    that->app_data_->commands[rig_name] = value; 
}

// Listen and server button callback
void app_grp::cb_bn_app(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    cb_value<Fl_Light_Button, bool>(w, v);
    ((qso_apps*)that->parent())->add_servers(that->app_data_);
    that->enable_widgets();
}

// Constructor for the tab form
qso_apps::qso_apps(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
    load_values();
    create_tabs();
    enable_widgets();
}

// Destructor - save settings
qso_apps::~qso_apps() {
    save_values();
}

// Load settings
void qso_apps::load_values() {
    Fl_Preferences apps_settings(settings_, "Apps");
    for (int ix = 0; ix < apps_settings.groups(); ix++) {
        const char* app = apps_settings.group(ix);
        Fl_Preferences app_settings(apps_settings, app);
        app_data_t* data = new app_data_t;
        data->name = app;
        int tempi;
        app_settings.get("Common", tempi, (int)false);
        data->is_common = tempi;
        app_settings.get("Server", tempi, (int)false);
        data->server = tempi;
        data->has_server = nullptr;
        data->has_data = nullptr;
        if (data->server) add_servers(data);
        Fl_Preferences rigs_settings(app_settings, "Rigs");
        for (int iy = 0; iy < rigs_settings.entries(); iy++) {
            const char* rig = rigs_settings.entry(iy);
            char* temp;
            rigs_settings.get(rig, temp, app);
            data->commands[string(rig)] = temp;
        }
        if (data->is_common && data->commands.size() > 1) {
            char msg[128];
            snprintf(msg, sizeof(msg), "APPS: Data error for app %s", app);
            status_->misc_status(ST_WARNING, msg);
        }
        apps_data_[data->name] = data;
    }
}

// Create the tabs
void qso_apps::create_tabs(string name) {
    Fl_Group* save = Fl_Group::current();
    Fl_Group::current(tabs_);

    tabs_->clear();

    int rx = 0;
    int ry = 0;
    int rw = 0;
    int rh = 0;
    tabs_->client_area(rx, ry, rw, rh, 0);
    tabs_->callback(cb_tabs);
    int saved_rh = rh;
    int saved_rw = rw;

    for (auto it = apps_data_.begin(); it != apps_data_.end(); it++) {
        app_grp* g = new app_grp(rx, ry, rw, rh, (*it).second->name.c_str());
        g->set_data((*it).second);
        g->labelsize(FL_NORMAL_SIZE + 2);
        rh = max(rh, g->h());
        rw = max(rw, g->w());
    }

    for (auto ix = 0; ix < tabs_->children(); ix++) {
        tabs_->child(ix)->size(rw, rh);
        if (string(tabs_->child(ix)->label()) == name) {
            tabs_->value(tabs_->child(ix));
        }
    }

    tabs_->resizable(nullptr);
    tabs_->size(tabs_->w() + rw - saved_rw, tabs_->h() + rh - saved_rh);

    Fl_Group::current(save);
}

// Create the tab form
void qso_apps::create_form() {
    int curr_x = x() + GAP;
    int curr_y = y() + GAP;

    bn_new_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "New app");
    bn_new_->callback(cb_bn_new);
    bn_new_->tooltip("Type in the app name in adjacent field and click to create");

    curr_x += WBUTTON;

    ip_new_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON);
    ip_new_->callback(cb_value<Fl_Input, string>, &new_name_);
    ip_new_->tooltip("Type in the name of the new app");

    curr_x = x();
    curr_y += HBUTTON + GAP;

    int avail_w = w() - GAP - GAP;
    int avail_h = h() + y() - curr_y - GAP;

    curr_x = x() + GAP;

    tabs_ = new Fl_Tabs(curr_x, curr_y, avail_w, avail_h);
    tabs_->box(FL_BORDER_BOX);
    create_tabs();
    tabs_->end();
 
    resizable(nullptr);
    int max_x = tabs_->x() + tabs_->w() + GAP;
    int max_y = tabs_->y() + tabs_->h() + GAP;

    size(max_x - x(), max_y - y());
    end();

    redraw();
}

// save settings
void qso_apps::save_values() {
    Fl_Preferences apps_settings(settings_, "Apps");
    apps_settings.clear();
    for (auto it = apps_data_.begin(); it != apps_data_.end(); it++) {
        Fl_Preferences app_settings(apps_settings, (*it).first.c_str());
        app_settings.set("Common", (*it).second->is_common);
        app_settings.set("Server", (*it).second->server);
        Fl_Preferences rigs_settings(app_settings, "Rigs");
        for (auto iu = (*it).second->commands.begin(); 
            iu != (*it).second->commands.end(); iu++) {
            rigs_settings.set((*iu).first.c_str(), (*iu).second.c_str());
        }
    }
}

// Configure widgets
void qso_apps::enable_widgets() {
    add_servers(apps_data_.at(tabs_->value()->label()));
    for (int ix = 0; ix < tabs_->children(); ix++) {
        app_grp* ax = (app_grp*)tabs_->child(ix);
        ax->enable_widgets();
        if (ax == tabs_->value()) {
            ax->labelfont((ax->labelfont() | FL_BOLD) & (~FL_ITALIC));
            ax->labelcolor(FL_FOREGROUND_COLOR);
        }
        else {
            ax->labelfont((ax->labelfont() & (~FL_BOLD)) | FL_ITALIC);
            ax->labelcolor(FL_FOREGROUND_COLOR);
        }
    }
}

// Set new tab
void qso_apps::cb_bn_new(Fl_Widget* w, void* v) {
    qso_apps* that = ancestor_view<qso_apps>(w);

     // Default data
    app_data_t* new_data = new app_data_t;
    new_data->name = that->new_name_;
    new_data->commands = {};
    new_data->is_common = false;
    new_data->server = false;
    new_data->has_data = nullptr;
    new_data->has_server = nullptr;

    if (new_data->server) that->add_servers(new_data);

    that->apps_data_[that->new_name_] = new_data;

    that->create_tabs(new_data->name);
    that->enable_widgets();

    that->redraw();
}

// Switch tab
void qso_apps::cb_tabs(Fl_Widget* w, void* v) {
    qso_apps* that = ancestor_view<qso_apps>(w);
    that->enable_widgets();
}

// Add the server links
void qso_apps::add_servers(app_data_t* data) {
    if (data->name == "FLDigi") {
        if (fllog_emul_) {
            data->has_server = fllog_emul_->has_server;
            data->has_data = fllog_emul_->has_data;
        }
    } 
    else if (data->name == "WSJT-X") {
        if (wsjtx_handler_) {
            data->has_server = wsjtx_handler_->has_server;
            data->has_data = wsjtx_handler_->has_data;
        }
    }
    else {
        char msg[128];
        snprintf(msg, sizeof(msg), "APPS: Don't know how to serve %s", data->name.c_str());
        status_->misc_status(ST_WARNING, msg);
    }

}