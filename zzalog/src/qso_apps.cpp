#include "qso_apps.h"

#include "file_viewer.h"
#include "filename_input.h"
#include "fllog_emul.h"
#include "main.h"
#include "password_input.h"
#include "qso_manager.h"
#include "qso_rig.h"
#include "settings.h"
#include "status.h"
#include "wsjtx_handler.h"

#include "callback.h"
#include "drawing.h"
#include "utils.h"

#include "nlohmann/json.hpp"

#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Radio_Light_Button.H>

using json = nlohmann::json;

// JSON mapping for app_rig_class_t
NLOHMANN_JSON_SERIALIZE_ENUM(app_rig_class_t, {
    { NO_CONNECTION, "No connection" },
    { AUDIO_ONLY, "Audio only"},
    { AUDIO_CAT, "Audio & CAT"}
})

// JSON serialisation from app_data_t
void to_json(json& j, const app_data_t& d) {
    j = json{
        { "Connection", json(d.rig_class) },
        { "Is server", d.server },
        { "Needs administrator", d.admin },
        { "Can disable", d.can_disable}
    };
    if (d.server) {
        j["Network address"] = d.address;
        j["Network port"] = d.port_num;
    }
    json js;
    for (auto sc : d.commands) {
        js[sc.first] = sc.second;
    }
    j["Scripts"] = js;
}

// JSON serialisation to app_data_t
void from_json(const json& j, app_data_t& d) {
    j.at("Connection").get_to(d.rig_class);
    j.at("Is server").get_to(d.server);
    j.at("Needs administrator").get_to(d.admin);
    j.at("Can disable").get_to(d.can_disable);
    if (d.server) {
        j.at("Network address").get_to(d.address);
        j.at("Network port").get_to(d.port_num);
    }
    if (j.find("Scripts") != j.end()) {
        auto scripts = j.at("Scripts").get<std::map<std::string, std::string> >();
        for (auto& sc : scripts) {
            d.commands[sc.first] = sc.second;
        }
    }
}

// Constructor for one std::set of modem controls
app_grp::app_grp(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    create_form();
}

// Destructor
app_grp::~app_grp() {
}

// Create fthe widgets
void app_grp::create_form() {
    int curr_x = x() + GAP;
    int curr_y = y() + HTEXT;

    box(FL_BORDER_BOX);

    radio_class_ = new Fl_Group(curr_x, curr_y, WBUTTON * 3, HBUTTON, "Rig connection");
    radio_class_->box(FL_FLAT_BOX);

    // Button to select common command for all rigs
    bn_common_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Common");
    bn_common_->callback(cb_bn_class, (void*)(intptr_t)NO_CONNECTION);
    bn_common_->tooltip("Select if no configuration per rig is required");

    curr_x += WBUTTON;

    // Button to select command for rig specific, CAT mnon-specific
    bn_rig_nocat_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Audio");
    bn_rig_nocat_->callback(cb_bn_class, (void*)(intptr_t)AUDIO_ONLY);
    bn_rig_nocat_->tooltip("Select if configuration is required only for audio");

    curr_x += WBUTTON;

    // Button to select command for rig and CAT specific
    bn_rig_cat_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Aud./CAT");
    bn_rig_cat_->callback(cb_bn_class, (void*)(intptr_t)AUDIO_CAT);
    bn_rig_cat_->tooltip("Select if configuration is required for audio and CAT");

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

    curr_x = x() + GAP + WBUTTON / 2;
    curr_y += HBUTTON;

     // Network address
    ip_nw_address_ = new Fl_Input(curr_x, curr_y, 3 * WBUTTON / 2, HBUTTON, "Addr");
    ip_nw_address_->callback(cb_ip_nwaddr, &(app_data_->address));
    ip_nw_address_->tooltip("Enter the network for ZZALOG as a server");

    curr_x += WBUTTON * 2;
    ip_nw_port_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Port");
    ip_nw_port_->callback(cb_ip_nwport, &(app_data_->port_num));
    ip_nw_port_->tooltip("Enter the port number for ZZALOG as a server");

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
    bn_common_->value(app_data_->rig_class == NO_CONNECTION);
    bn_rig_nocat_->value(app_data_->rig_class == AUDIO_ONLY);
    bn_rig_cat_->value(app_data_->rig_class == AUDIO_CAT);
    bn_server_->value(app_data_->server);
    ip_nw_address_->value(app_data_->address.c_str());
    char temp[32];
    snprintf(temp, sizeof(temp), "%d", app_data_->port_num);
    ip_nw_port_->value(temp);
    if (app_data_->server) {
        ip_nw_address_->activate();
        ip_nw_port_->activate();
    }
    else {
        ip_nw_address_->deactivate();
        ip_nw_port_->deactivate();
    }
    bn_rig_->copy_label(rig_name);
    if (app_data_->commands.find(rig_name) != app_data_->commands.end()) {
        ip_app_name_->value(app_data_->commands.at(std::string(rig_name)).c_str());
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
    std::string server = that->label();
    if (server == FLDIGI) {
        if (fllog_emul_->has_server()) {
            fllog_emul_->close_server();
        } else {
            fllog_emul_->run_server();
        }
    }
    else if (server == WSJTX) {
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
        std::string app = that->app_data_->commands.at(rig_name);
        char msg[128];
        snprintf(msg, sizeof(msg), "DASH: Starting app: %s", app.c_str());
        status_->misc_status(ST_NOTE, msg);
#ifdef _WIN32
        std::string cmd = "start /min " + app;
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
    std::string value;
    cb_value<Fl_Input, std::string>(w, &value);
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

// Network address
void app_grp::cb_ip_nwaddr(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    cb_value<Fl_Input, std::string>(w, &that->app_data_->address);
    that->enable_widgets();
}

// Network port
void app_grp::cb_ip_nwport(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    cb_value_int<Fl_Int_Input>(w, &that->app_data_->port_num);
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

// Callback to std::set can_disable
void app_grp::cb_bn_disable(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    cb_value<Fl_Check_Button, bool>(w, &that->app_data_->can_disable);
    that->enable_widgets();
}

// Callback to read disable command
void app_grp::cb_ip_disable(Fl_Widget* w, void* v) {
    app_grp* that = ancestor_view<app_grp>(w);
    std::string value;
    cb_value<Fl_Input, std::string>(w, &value);
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
    std::string fn = ip->value();
    qso_apps* qa = ancestor_view<qso_apps>(that);
    file_viewer* fwin = qa->viewer();
    if (fwin->file() == fn && fwin->visible() && !fwin->is_dirty()) {
        fwin->hide();
    } else if (fn.length() > 0) {
        fwin->load_file(fn);
    }
}

// Generate rig id 
const char* app_grp::rig_id() {
    char* result = new char[32];
    qso_manager* mgr = ancestor_view<qso_manager>(this);
    qso_rig* rig_ctrl = mgr->rig_control();
    switch(app_data_->rig_class) {
        case NO_CONNECTION:
            strcpy(result, "COMMON");
            break;
        case AUDIO_ONLY:
            if (rig_ctrl) {
                strcpy(result, mgr->rig_control()->label());
            } else {
                strcpy(result, "");
            }
            break;
        case AUDIO_CAT:
            if (rig_ctrl) {
                snprintf(result, 32, "%s %s",
                    mgr->rig_control()->label(),
                    mgr->rig_control()->cat());
            } else {
                strcpy(result, "");
            }
        break;
    }
    return result;
}

// Constructor for the tab form
qso_apps::qso_apps(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
    tooltip("Controls any partner apps");
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

// Handle
int qso_apps::handle(int event) {
    int result = Fl_Group::handle(event);
    // Now handle F1 regardless
    switch (event) {
    case FL_FOCUS:
        return true;
    case FL_UNFOCUS:
        // Acknowledge focus events to get the keyboard event
        return true;
    case FL_PUSH:
		if (!result) take_focus();
        return true;
    case FL_KEYBOARD:
        switch (Fl::event_key()) {
        case FL_F + 1:
            open_html("qso_apps.html");
            return true;
        }
        break;
    }
    return result;
}

// Load JSON


// Load settings
void qso_apps::load_values() {
    std::string filename = default_data_directory_ + "apps.json";
    std::ifstream i(filename);
    char msg[128];
    if (i.good()) {
        try {
            json j;
            i >> j;
            if (!j.at("Apps").is_null()) {
                auto apps = j.at("Apps").get<std::map<std::string, json>>();
                for (auto a : apps) {
                    app_data_t* data = new app_data_t(a.second);
                    data->name = a.first;
                    apps_data_[a.first] = data;
                    if (data->server) add_servers(data);
                }
            }
        }
        catch (const json::exception& e) {
            snprintf(msg, sizeof(msg), "APPS: Reading JSON failed %d (%s)\n",
                e.id, e.what());
            status_->misc_status(ST_ERROR, msg);
            i.close();
        }
        snprintf(msg, sizeof(msg), "APPS: %s loaded OK!", filename.c_str());
        status_->misc_status(ST_OK, msg);
    }
    // Load default tab value
    settings top_settings;
    settings view_settings(&top_settings, "Views");
    settings dash_settings(&view_settings, "Dashboard");
    dash_settings.get("Default App", default_tab_, 0);
}

// Create the tabs
void qso_apps::create_tabs(std::string name) {
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
        if (std::string(tabs_->child(ix)->label()) == name) {
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
    ip_new_->callback(cb_ip_new, &new_name_);
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
    json j;
    
    for (auto it : apps_data_) {
        j[it.first] = *it.second;
    }

    json jout;
    jout["Apps"] = j;

    std::string filename = default_data_directory_ + "apps.json";
    std::ofstream o(filename);
    o << std::setw(4) << jout << '\n';
    o.close();
    settings top_settings;
    settings view_settings(&top_settings, "Views");
    settings dash_settings(&view_settings, "Dashboard");
    // Find the current selected tab and save its index
    Fl_Widget* w = tabs_->value();
    for (int ix = 0; ix != children(); ix++) {
        if (child(ix) == w) {
            dash_settings.set("Default App", ix);
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
    if (new_name_.length()) {
        bn_new_->activate();
    } else {
        bn_new_->deactivate();
    }
}

// Set new tab
void qso_apps::cb_bn_new(Fl_Widget* w, void* v) {
    qso_apps* that = ancestor_view<qso_apps>(w);

     // Default data
    app_data_t* new_data = new app_data_t;
    new_data->name = that->new_name_;
    new_data->commands = {};
    new_data->rig_class = AUDIO_CAT;
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

// Input new app nae
void qso_apps::cb_ip_new(Fl_Widget* w, void* v) {
    qso_apps* that = ancestor_view<qso_apps>(w);
    cb_value<Fl_Input, std::string>(w, v);
    that->enable_widgets(); 
}

// Switch tab
void qso_apps::cb_tabs(Fl_Widget* w, void* v) {
    qso_apps* that = ancestor_view<qso_apps>(w);
    that->enable_widgets();
}

// Add the server links
void qso_apps::add_servers(app_data_t* data) {
    if (data->name == FLDIGI) {
        if (fllog_emul_) {
            data->has_server = fllog_emul_->has_server;
            data->has_data = fllog_emul_->has_data;
        }
    } 
    else if (data->name == WSJTX) {
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
    apps_data_.erase(std::string(w->label()));
    int ch = tabs_->find(w);
    tabs_->delete_child(ch);
    enable_widgets();
}

file_viewer* qso_apps::viewer() { 
    return viewer_;
}

std::string qso_apps::network_address(std::string app) {
    if (apps_data_.find(app) != apps_data_.end()) {
        return apps_data_[app]->address;
    }
    else {
        return "";
    }
}

int qso_apps::network_port(std::string app) {
    if (apps_data_.find(app) != apps_data_.end()) {
        return apps_data_[app]->port_num;
    }
    else {
        return 0;
    }
}