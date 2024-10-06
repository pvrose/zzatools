#pragma once

#include "import_data.h"
#include "extract_data.h"

#include <FL/Fl_Group.H>

class Fl_Button;
class Fl_Check_Button;
class Fl_Fill_Dial;

// This controls the download and upload of QSLs to the various servers
// and for printing off labels
class qso_qsl :
    public Fl_Group
{
public:
    qso_qsl(int X, int Y, int W, int H, const char* L);
    ~qso_qsl();

    // Load settings
    void load_values();
    // Create the widgets
    void create_form();
    // Save the settings
    void save_values();
    // Configure the widgets
    void enable_widgets();

   // Shared download method
    void qsl_download(import_data::update_mode_t server);
    void qsl_extract(extract_data::extract_mode_t server);
    void qsl_upload();
    void qsl_print();
    void qsl_print_done();
    void qsl_cancel();
    void qsl_generate_png();
    void qsl_send_email();
    // Single QSO versions 
    void qsl_1_upload(extract_data::extract_mode_t server);
    void qsl_1_generate_png();
    void qsl_1_send_email();
    void qsl_1_send_email(record* qso);

    // Update eQSL image download count
    void update_eqsl(int count);

    // 1 second clock. Used to update eQSL throttle timer display
    void ticker();
    // Static interface to above
    static void cb_ticker(void* v);

protected:
    // callbacks
    // Auto download v = eQSL/LotW/ClubLog
    static void cb_auto(Fl_Widget* w, void* v);
    // Download. v = eQSL/LotW
    static void cb_download(Fl_Widget* w, void* v);
    // Extract. v = eQSL/LotW/ClubLog/Card
    static void cb_extract(Fl_Widget* w, void* v);
    // Upload. v = eQSL/LotW/Clublog
    static void cb_upload(Fl_Widget* w, void* v);
    // Print . Card only
    static void cb_print(Fl_Widget* w, void* v);
    // Mark printing done
    static void cb_mark_done(Fl_Widget* w, void* v);
    // Cancel extract
    static void cb_cancel(Fl_Widget* w, void* v);
    // generate PNG files
    static void cb_png(Fl_Widget* w, void* v);
    // generate e-mails
    static void cb_email(Fl_Widget* w, void* v);
    // Single QSO button
    static void cb_single(Fl_Widget* w, void* v);

 
    // Attributes
    bool auto_eqsl_;
    bool auto_lotw_;
    bool auto_club_;

    // Outstanding eQSL downloads
    int os_eqsl_dnld_;
    // Ticker count
    float tkr_value_;
    // Extract in progress - update gets called while it is
    bool extract_in_progress_;
    // Upload only specified QSL
    bool single_qso_;

    // Widgets
    Fl_Check_Button* bn_single_qso_;
    Fl_Check_Button* bn_auto_eqsl_;
    Fl_Check_Button* bn_auto_lotw_;
    Fl_Check_Button* bn_auto_club_;
    Fl_Button* bn_down_eqsl_;
    Fl_Button* bn_down_lotw_;
    Fl_Button* bn_extr_eqsl_;
    Fl_Button* bn_extr_lotw_;
    Fl_Button* bn_extr_club_;
    Fl_Button* bn_extr_card_;
    Fl_Button* bn_extr_email_;
    Fl_Button* bn_upld_eqsl_;
    Fl_Button* bn_upld_lotw_;
    Fl_Button* bn_upld_club_;
    Fl_Button* bn_print_;
    Fl_Button* bn_png_;
    Fl_Button* bn_mark_done_;
    Fl_Button* bn_cncl_eqsl_;
    Fl_Button* bn_cncl_lotw_;
    Fl_Button* bn_cncl_club_;
    Fl_Button* bn_cncl_card_;
    Fl_Button* bn_cncl_email_;
    Fl_Button* bn_send_email_;
    Fl_Fill_Dial* op_eqsl_count_;

};

