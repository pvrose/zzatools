#pragma once

#include "import_data.h"
#include "extract_data.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Progress.H>

class qso_qsl :
    public Fl_Group
{
public:
    qso_qsl(int X, int Y, int W, int H, const char* L);
    ~qso_qsl();

    void load_values();
    void create_form();
    void save_values();
    void enable_widgets();

   // Shared download method
    void qsl_download(import_data::update_mode_t server);
    void qsl_extract(extract_data::extract_mode_t server);
    void qsl_upload();
    void qsl_print();
    void qsl_print_done();
    void qsl_cancel();

    // Update eQSL image download count
    void update_eqsl(int count);

    void ticker();

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

 
    // Attributes
    bool auto_eqsl_;
    bool auto_lotw_;
    bool auto_club_;

    // Outstanding eQSL downloads
    int os_eqsl_dnld_;
    // Ticker count
    float tkr_value_;

    // Widgets
    Fl_Check_Button* bn_auto_eqsl_;
    Fl_Check_Button* bn_auto_lotw_;
    Fl_Check_Button* bn_auto_club_;
    Fl_Button* bn_down_eqsl_;
    Fl_Button* bn_down_lotw_;
    Fl_Button* bn_extr_eqsl_;
    Fl_Button* bn_extr_lotw_;
    Fl_Button* bn_extr_club_;
    Fl_Button* bn_extr_card_;
    Fl_Button* bn_upld_eqsl_;
    Fl_Button* bn_upld_lotw_;
    Fl_Button* bn_upld_club_;
    Fl_Button* bn_print_;
    Fl_Button* bn_mark_done_;
    Fl_Button* bn_cncl_eqsl_;
    Fl_Button* bn_cncl_lotw_;
    Fl_Button* bn_cncl_club_;
    Fl_Button* bn_cncl_card_;
    Fl_Progress* op_eqsl_count_;

};

