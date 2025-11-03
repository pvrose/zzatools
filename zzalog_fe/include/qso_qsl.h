#pragma once

#include "import_data.h"
#include "extract_data.h"

#include <FL/Fl_Group.H>

class Fl_Button;
class Fl_Check_Button;

//! \brief This class controls the download and upload of QSLs to the various servers
//! and for printing off labels
class qso_qsl :
    public Fl_Group
{
public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qso_qsl(int X, int Y, int W, int H, const char* L);
    //! Destructor.
    ~qso_qsl();

    //! Inheritedfrom Fl_Group::handle(): it allows keyboard F1 to open userguide.
    virtual int handle(int event);

    //! Copies references to server credentials from qsl_data.
    void load_values();
    //! Instantiates component widgets.
    void create_form();
    //! Updates data in qsl_data.
    void save_values();
    //! Configures the component widgets after a data change.
    void enable_widgets();

   // Shared download method
    //! Download std::list of QSLs from \p server.
    void qsl_download(import_data::update_mode_t server);
    //! Extract data for uploading to \p server.
    void qsl_extract(extract_data::extract_mode_t server);
    //! Upload extracted data to remembered server.
    void qsl_upload();
    //! Print labels for extracted data.
    void qsl_print();
    //! Mark all extracted QSO records as with QSL sent flag and date. 
    void qsl_mark_done();
    //! Clear the extracted data.
    void qsl_cancel();
    //! Generate PNG file for an e-mailed QSL card.
    void qsl_generate_png();
    //! Send e-mail QSLs for all extracted QSO records.
    void qsl_send_email();
    //! Upload the selected QSO record to \p server.
    void qsl_1_upload(extract_data::extract_mode_t server);
    //! Generate a single PNG file for the selected QSO record.
    void qsl_1_generate_png();
    //! Send an e-mail for the selected QSO record.
    void qsl_1_send_email();
    //! Send an e-mail for the specified QSO record \p qso.
    void qsl_1_send_email(record* qso);

    //! Update eQSL image download count.
    void update_eqsl(int count);

protected:
    // callbacks
    //! Callback from check buttons: \p v indicates eQSL/LotW/ClubLog
    static void cb_auto(Fl_Widget* w, void* v);
    //! Callback from download buttons: \p v indicates eQSL/LotW/QRZ.com
    static void cb_download(Fl_Widget* w, void* v);
    //! Callback from extract buttons: \p v indicates eQSL/LotW/ClubLog/Card
    static void cb_extract(Fl_Widget* w, void* v);
    //! Callback from upload buttons: \p v indicates eQSL/LotW/Clublog
    static void cb_upload(Fl_Widget* w, void* v);
    //! Callback from print button: for Card only.
    static void cb_print(Fl_Widget* w, void* v);
    //! Callback from "Done" button: \p v indicates QRZ.com/Card
    static void cb_mark_done(Fl_Widget* w, void* v);
    //! Callback from undo buttons: \p v indicates which extracted data.
    static void cb_cancel(Fl_Widget* w, void* v);
    //! Callback from save button: for e-Mail only (create and save PNG image).
    static void cb_png(Fl_Widget* w, void* v);
    //! Callback from e-mail button: for e-Mail only.
    static void cb_email(Fl_Widget* w, void* v);
    //! Callback from "Only handle selected QSO" check button.
    static void cb_single(Fl_Widget* w, void* v);

 
    // Attributes
    bool auto_eqsl_;       //!< Update to eQSL after each QSO.
    bool auto_lotw_;       //!< Update to LotW after each QSO.
    bool auto_club_;       //!< Update to Clublog.org after each QSO.
    bool auto_qrz_;        //!< Update to QRZ.com after each QSO.

    //! Count of outstanding eQSL downloads
    int os_eqsl_dnld_;
    //! Indicates fraction complete of the eQSL.cc throttle period.
    float tkr_value_;
    //! Extract in progress - update gets called while it is
    bool extract_in_progress_;
    //! Upload only selected QSL
    bool single_qso_;
    //! Enumeration value for QSL_SENT_VIA update when "Done" is clicked.
    std::string via_code_;

    // Widgets
    Fl_Check_Button* bn_single_qso_;  //!< Check button "...selected QSO".
    Fl_Check_Button* bn_auto_eqsl_;   //!< Check button "eQSL"
    Fl_Check_Button* bn_auto_lotw_;   //!< Check button "LotW"
    Fl_Check_Button* bn_auto_club_;   //!< Check button "ClubLog"
    Fl_Check_Button* bn_auto_qrz_;    //!< Check button: "QRZ.com"
    Fl_Button* bn_down_eqsl_;         //!< Button: download eQSL
    Fl_Button* bn_down_lotw_;         //!< Button: download LotW
    Fl_Button* bn_extr_eqsl_;         //!< Button: extract eQSL
    Fl_Button* bn_extr_lotw_;         //!< Button: extract LotW
    Fl_Button* bn_extr_club_;         //!< Button: extract ClubLog
    Fl_Button* bn_extr_card_;         //!< Button: extract Cards
    Fl_Button* bn_extr_email_;        //!< Button: extract e-Mails
    Fl_Button* bn_upld_eqsl_;         //!< Button: upload eQSL
    Fl_Button* bn_upld_lotw_;         //!< Button: upload LotW
    Fl_Button* bn_upld_club_;         //!< Button: upload ClubLog
    Fl_Button* bn_down_qrz_;          //!< Button: download QRZ.com
    Fl_Button* bn_extr_qrz_;          //!< Button: extract QRZ.com
    Fl_Button* bn_upld_qrz_;          //!< Button: upload QRZ.com
    Fl_Button* bn_save_qrz_;          //!< Button: save QRZ.com log
    Fl_Button* bn_qrz_done_;          //!< Button: "Done" QRZ.com
    Fl_Button* bn_print_;             //!< Button: print labels
    Fl_Button* bn_png_;               //!< Button: create and save PNG (e-Mails)
    Fl_Button* bn_card_done_;         //!< Button: "Done" Cards
    Fl_Button* bn_cancel_;            //!< Button: undo.
    Fl_Button* bn_send_email_;        //!< Button: send e-Mails
    Fl_Button* bn_email_done_;        //!< Button: "Done" e-Mails
    Fl_Button* bn_down_oqrs_;         //!< Button: "OQRS"

};

