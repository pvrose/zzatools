#include "drawing.h"
#include "fllog_emul.h"
#include "wsjtx_handler.h"
#include "status.h"
#include "qso_apps.h"
#include "qso_manager.h"
#include "qso_rig.h"
#include "utils.h"
#include "callback.h"
#include "password_input.h"
#include "file_viewer.h"
#include "filename_input.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Radio_Light_Button.H>

using namespace std;

extern fllog_emul* fllog_emul_;
extern wsjtx_handler* wsjtx_handler_;
extern status* status_;
extern string VENDOR;
extern string PROGRAM_ID;

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

    radio_class_ = new Fl_Group(curr_x, curr_y, WBUTTON * 3, HBUTTON);
    radio_class_->box(FL_FLAT_BOX);

    // Button to select common command for all rigs
    bn_common_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "All rigs");
    bn_common_->callback(cb_bn_class, (void*)(intptr_t)ALL_RIGS);
    bn_common_->tooltip("Select if the same command is used for all rigs");

    curr_x += WBUTTON;

    // Button to select command for rig specific, CAT mnon-specific
    bn_rig_nocat_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Rig");
    bn_rig_nocat_->callback(cb_bn_class, (void*)(intptr_t)RIG_NO_CAT);
    bn_rig_nocat_->tooltip("Select if a separate command is required for the rig, but not CAT method");

    curr_x += WBUTTON;

    // Button to select command for rig and CAT specific
    bn_rig_cat_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Rig+CAT");
    bn_rig_cat_->callback(cb_bn_class, (void*)(intptr_t)RIG_CAT);
    bn_rig_cat_->tooltip("Select if a separate command is required for the rig and the CAT method");

    radio_class_->end();
    
    curr_x = x() + GAP;
    curr_y += HBUTTON;

    // Button to act as a log server
    bn_server_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Server");
    bn_server_->callback(cb_bn_server, &(app_data_->server));
    bn_server_->tooltip("Select if ZZALOG acts as a log server for this application");

    curr_x += WBUTTON;

    // Button t o delete application
    bn_delete_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Delete");
    bn_delete_->callback(cb_bn_delete);
    bn_delete_->tooltip("Remove the application");

    curr_x = x() + GAP;
    curr_y += HBUTTON;

    // Button to display rig name
    bn_rig_ = new Fl_Button(curr_x, curr_y, 3 * WBUTTON, HBUTTON);
    bn_rig_->when(FL_WHEN_NEVER);
    bn_rig_->box(FL_FLAT_BOX);
    bn_rig_->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    bn_rig_->tooltip("Displays the current rig - or COMMON");

    curr_y += HBUTTON;
    curr_x += WBUTTON / 2;

    // Button to show the script
    bn_show_script_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Show");
    bn_show_script_->tooltip("Show the script in a separate window");

    curr_x += WBUTTON / 2;

    // Input to specify the command for invoking the app
    ip_app_name_ = new filename_input(curr_x, curr_y, WBUTTON * 2, HBUTTON);
    ip_app_name_->callback(cb_ip_app);
    ip_app_name_->tooltip("Enter the command to invoke the app for the selected rig");
    ip_app_name_->when(FL_WHEN_CHANGED);

    bn_show_script_->callback(cb_show_script, &ip_app_name_);

    curr_y += HBUTTON;
    curr_x = x() + GAP;

    // Allow the application to be disconnected
    bn_disable_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "@undo");
    bn_disable_->align(FL_ALIGN_INSIDE);
    bn_disable_->callback(cb_bn_disable);
    bn_disable_->tooltip("Select if able to disconnect from application");

    curr_x += WBUTTON / 2;

    // Button to show the script
    bn_show_script2_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Show");
    bn_show_script2_->callback(cb_show_script, &ip_disable_app_);
    bn_show_script2_->tooltip("Show the undo script in a separate window");

    curr_x += WBUTTON / 2;

    // The script needed to disconnect
    ip_disable_app_ = new filename_input(curr_x, curr_y, WBUTTON * 2, HBUTTON);
    ip_disable_app_->callback(cb_ip_disable);
    ip_disable_app_->tooltip("Enter the command to disconnect the application");
    ip_disable_app_->when(FL_WHEN_CHANGED);

    bn_show_script2_->callback(cb_show_script, &ip_disable_app_);

    curr_y += HBUTTON;
    curr_x = x() + GAP;

    // Button to run application as administrator
    bn_admin_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Adm");
    bn_admin_->align(FL_ALIGN_INSIDE);
    bn_admin_->callback(cb_bn_admin);
    bn_admin_->tooltip("Run the application as adminstrator");

    curr_x += WBUTTON;

    // Password input
    ip_passw_ = new password_input(curr_x, curr_y, WBUTTON * 2, HBUTTON);
    ip_passw_->tooltip("Enter the administrator's password");
    ip_passw_->value("");
    ip_passw_->callback(cb_ip_passw);
    ip_passw_->when(FL_WHEN_CHANGED);

    curr_x += ip_passw_->w();

    curr_x = x() + GAP + WBUTTON;
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
    const char* rig_name = rig_id();
    // Update the various widgets
    bn_common_->value(app_data_->rig_class == ALL_RIGS);
    bn_rig_nocat_->value(app_data_->rig_class == RIG_NO_CAT);
    bn_rig_cat_->value(app_data_->rig_class == RIG_CAT);
    bn_server_->value(app_data_->server);
    bn_rig_->copy_label(rig_name);
    if (app_data_->commands.find(rig_name) != app_data_->commands.end()) {
        ip_app_name_->value(app_data_->commands.at(string(rig_name)).c_str());
        bn_show_script_->activate();
    } else {
        ip_app_name_->value("");
        bn_show_script_->deactivate();
    }
    if (app_data_->server) {
        if (app_data_->has_server && (*(app_data_->has_server))()) {
            bn_listening_->value(true);
            bn_listening_->activate();
            if (!app_data_->admin || strlen(ip_passw_->value()) > 0) {
                bn_connect_->activate();
                if (app_data_->has_data && (*(app_data_->has_data))()) {
                    bn_connect_->value(true);
                } else {
                    bn_connect_->value(false);
                }
            } else {
                bn_connect_->deactivate();
            }
        } else {
            bn_listening_->value(false);
            bn_listening_->activate();
            bn_connect_->deactivate();
        }
    } else {
        bn_listening_->deactivate();
        if (!app_data_->admin || strlen(ip_passw_->value()) > 0) {
            bn_connect_->activate();
            if (!app_data_->can_disable) {
                bn_connect_->value(false);
            }
        } else {
            bn_connect_->deactivate();
        }
    }
    if (app_data_->admin) {
        bn_admin_->value(true);
        ip_passw_->activate();
        ip_passw_->redraw();
    } else {
        bn_admin_->value(false);
        ip_passw_->value("");
        ip_passw_->deactivate();
    }
    if (app_data_->can_disable) {
        bn_disable_->value(true);
        ip_disable_app_->value(app_data_->commands["NONE"].c_str());
        ip_disable_app_->activate();
        bn_show_script2_->activate();
    } else {
        bn_disable_->value(false);
        ip_disable_app_->value("");
        ip_disable_app_->deactivate();
        bn_show_script2_->deactivate();
    }
}

// Set the dataa for this app
void app_grp::set_data(app_data_t* data) {
    app_data_ = data;
    // Set button call backs to use the new data
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
    const char* rig_name;
    if (that->app_data_->can_disable && !((Fl_Light_Button*)w)->value()) rig_name = "NONE";
    else rig_name = that->rig_id();
    if (that->app_data_->commands.find(rig_name) == that->app_data_->commands.end()) {
        char msg[128];
        snprintf(msg, sizeof(msg), "APPS: App %s does not know how to connect %s", 
            that->app_data_->name.c_str(), rig_name);
        status_->misc_status(ST_ERROR, msg);
    } else {
        string app = that->app_data_->commands.at(rig_name);
        char msg[128];
        snprintf(msg, sizeof(msg), "DASH: Starting app: %s", app.c_str());
        status_->misc_status(ST_NOTE, msg);
#ifdef _WIN32
        string cmd = "start /min " + app;
        system(cmd.c_str());
#else
        if (that->app_data_->admin) {
            char sudo[128];
            snprintf(sudo, sizeof(sudo), "MY_PW=%s; export MY_PW; %s", 
                that->ip_passw_->value(), app.c_str());
            system(sudo);
        } else {
            system(app.c_str());
        }
#endif
    }
    that->enable_widgets();
  
}

// App typed in
void app_grp::cb_ip_app(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    string value;
    cb_value<Fl_Input, string>(w, &value);
    const char* rig_name = that->rig_id();
    that->app_data_->commands[rig_name] = value; 
    that->enable_widgets();
}

// Listen and server button callback
void app_grp::cb_bn_class(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    that->app_data_->rig_class = (app_rig_class_t)(intptr_t)v;
    ((qso_apps*)that->parent())->add_servers(that->app_data_);
    that->enable_widgets();
}

// Listen and server button callback
void app_grp::cb_bn_server(Fl_Widget* w, void* v) {
    cb_value<Fl_Light_Button, bool>(w, v);
    app_grp* that = ancestor_view<app_grp>(w);
    that->enable_widgets();
}

// Call back for needing admin
void app_grp::cb_bn_admin(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    cb_value<Fl_Check_Button, bool>(w, &that->app_data_->admin);
    that->enable_widgets();
}

// Callback to delete the application
void app_grp::cb_bn_delete(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    qso_apps* apps = ancestor_view<qso_apps>(that);
    apps->delete_app(that);
}

// Callback to set can_disable
void app_grp::cb_bn_disable(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    cb_value<Fl_Check_Button, bool>(w, &that->app_data_->can_disable);
    that->enable_widgets();
}

// Callback to read disable command
void app_grp::cb_ip_disable(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    string value;
    cb_value<Fl_Input, string>(w, &value);
    that->app_data_->commands["NONE"] = value; 
}

// Callback on typing pasword
void app_grp::cb_ip_passw(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    that->enable_widgets();
}

// Show scripts
void app_grp::cb_show_script(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    filename_input* ip = *(filename_input**)v;
    string fn = ip->value();
    if (fn.length() > 0) {
        qso_apps* qa = ancestor_view<qso_apps>(that);
        file_viewer* fwin = qa->viewer();
        fwin->load_file(fn);
    }
}

// Generate rig id 
const char* app_grp::rig_id() {
    char* result = new char[32];
    qso_manager* mgr = ancestor_view<qso_manager>(this);
    switch(app_data_->rig_class) {
        case ALL_RIGS:
            strcpy(result, "COMMON");
            break;
        case RIG_NO_CAT:
            strcpy(result, mgr->rig_control()->label());
            break;
        case RIG_CAT:
            snprintf(result, 32, "%s %s",
                mgr->rig_control()->label(),
                mgr->rig_control()->cat());
            break;
    }
    return result;
}

// Constructor for the tab form
qso_apps::qso_apps(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
    load_values();
    create_tabs();
    enable_widgets();
    begin();
    viewer_ = new file_viewer(400, 300);
    end();
}

// Destructor - save settings
qso_apps::~qso_apps() {
    save_values();
}

// Load settings
void qso_apps::load_values() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
    Fl_Preferences apps_settings(settings, "Apps");
    for (int ix = 0; ix < apps_settings.groups(); ix++) {
        const char* app = apps_settings.group(ix);
        Fl_Preferences app_settings(apps_settings, app);
        app_data_t* data = new app_data_t;
        data->name = app;
        int tempi;
        app_settings.get("Common", tempi, (int)RIG_CAT);
        data->rig_class = (app_rig_class_t)tempi;
        app_settings.get("Server", tempi, (int)false);
        data->server = tempi;
        data->has_server = nullptr;
        data->has_data = nullptr;
        app_settings.get("Administrator", tempi, (int)false);
        data->admin = tempi;
        app_settings.get("Can Disconnect", tempi, (int)false);
        data->can_disable = tempi;
        if (data->server) add_servers(data);
        Fl_Preferences rigs_settings(app_settings, "Rigs");
        for (int iy = 0; iy < rigs_settings.entries(); iy++) {
            const char* rig = rigs_settings.entry(iy);
            char* temp;
            rigs_settings.get(rig, temp, app);
            data->commands[string(rig)] = temp;
        }
        if (data->rig_class == ALL_RIGS && !data->can_disable && data->commands.size() > 1) {
            char msg[128];
            snprintf(msg, sizeof(msg), "APPS: Data error for app %s", app);
            status_->misc_status(ST_WARNING, msg);
        }
        apps_data_[data->name] = data;
    }
    // Load default tab value
    Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
    tab_settings.get("Apps", default_tab_, 0);
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

    // Create a dummy tab to size the upper group correctly
    app_grp* dummy = new app_grp(rx, ry, rw, rh);
    rh = max(rh, dummy->h());
    rw = max(rw, dummy->w());
    tabs_->remove(dummy);
    Fl::delete_widget(dummy);

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
    if (tabs_->children() > 0) tabs_->value(child(default_tab_));

    resizable(nullptr);
    size(w(), tabs_->y() + tabs_->h() - y() + GAP);

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
 	tabs_->handle_overflow(Fl_Tabs::OVERFLOW_PULLDOWN);
    create_tabs();
    tabs_->end();
 
    resizable(nullptr);
    adjust_size();
    end();

    redraw();
}

// Adjust size to fit the tabs
void qso_apps::adjust_size() {
    int max_x = tabs_->x() + tabs_->w() + GAP;
    int max_y = tabs_->y() + tabs_->h() + GAP;

    size(max_x - x(), max_y - y());
}

// save settings
void qso_apps::save_values() {
    Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
    Fl_Preferences apps_settings(settings, "Apps");
    apps_settings.clear();
    for (auto it = apps_data_.begin(); it != apps_data_.end(); it++) {
        Fl_Preferences app_settings(apps_settings, (*it).first.c_str());
        app_settings.set("Common", (int)(*it).second->rig_class);
        app_settings.set("Server", (*it).second->server);
        app_settings.set("Administrator", (*it).second->admin);
        app_settings.set("Can Disconnect", (*it).second->can_disable);
        Fl_Preferences rigs_settings(app_settings, "Rigs");
        for (auto iu = (*it).second->commands.begin(); 
            iu != (*it).second->commands.end(); iu++) {
            rigs_settings.set((*iu).first.c_str(), (*iu).second.c_str());
        }
    }
    Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
    // Find the current selected tab and save its index
    Fl_Widget* w = tabs_->value();
    for (int ix = 0; ix != children(); ix++) {
        if (child(ix) == w) {
            tab_settings.set("Apps", ix);
        }
    }
}

// Configure widgets
void qso_apps::enable_widgets() {
    if (tabs_->value()) {
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
}

// Set new tab
void qso_apps::cb_bn_new(Fl_Widget* w, void* v) {
    qso_apps* that = ancestor_view<qso_apps>(w);

     // Default data
    app_data_t* new_data = new app_data_t;
    new_data->name = that->new_name_;
    new_data->commands = {};
    new_data->rig_class = RIG_CAT;
    new_data->server = false;
    new_data->has_data = nullptr;
    new_data->has_server = nullptr;
    new_data->can_disable = false;
    new_data->admin = false;

    if (new_data->server) that->add_servers(new_data);

    that->apps_data_[that->new_name_] = new_data;

    that->create_tabs(new_data->name);
    that->enable_widgets();
    that->adjust_size();

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
    else if (data->has_server) {
        char msg[128];
        snprintf(msg, sizeof(msg), "APPS: Don't know how to serve %s", data->name.c_str());
        status_->misc_status(ST_WARNING, msg);
    }

}


// Delete the application
void qso_apps::delete_app(app_grp* w) {
    apps_data_.erase(string(w->label()));
    int ch = tabs_->find(w);
    tabs_->delete_child(ch);
    enable_widgets();
}

file_viewer* qso_apps::viewer() { 
    return viewer_;
}