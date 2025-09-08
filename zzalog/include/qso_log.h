#pragma once

#include <FL/Fl_Tabs.H>

class qso_log_info;
class qso_qsl;
class qso_apps;
class qso_bands;

//! This class prides tabs for the qso_log_info, qso_qsl and qso_apps objects.
class qso_log :
    public Fl_Tabs
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_log(int X, int Y, int W, int H, const char* l);
	//! Destructot.
	~qso_log();

	//! Read default tab from settings.
	void load_values();
	//! Instantiate component widgets.
	void create_form(int X, int Y);
	//! Configure component widgets.
	void enable_widgets();
	//! Save current tab to settings.
	void save_values();

	//! Returns a refernece to the qso_qsl object.
	qso_qsl* qsl_control();
	//! Returns a reference to the qso_log_info object
	qso_log_info* log_info();
	//! Returns a reference to the qso_apps object.
	qso_apps* apps();
	

protected:

	//! Callback from selecting tab reformats labels.
	static void cb_tabs(Fl_Widget* w, void* v);

	// The four objects
	qso_log_info* log_info_;   //!< Log status.
	qso_qsl* qsl_ctrl_;        //!< Controls for uploading and downloading QSLs.
	qso_apps* apps_ctrl_;      //!< Controls for configuring and launching associated apps.
	qso_bands* bands_;         //!< Bandplan viewer.

	//! Default tab to open on start-up.
	int default_tab_;

};

