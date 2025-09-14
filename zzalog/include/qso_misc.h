#pragma once

#include <FL/Fl_Tabs.H>

class record;
class qso_details;
class qso_dxcc;
class qso_qsl_vwr;
class contest_scorer;
typedef size_t qso_num_t;

//! This class displays a std::set of tabs for qso_qth, qso_details, qso_dxcc, qso_qsl_vwr and contest_scorer.
class qso_misc :
    public Fl_Tabs
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_misc(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
    ~qso_misc();

	//! Gets default tab from settings.
	void load_values();
	//! Instantiates component widgets.
	void create_form();
	//! Configures component widgets after data changes.
	void enable_widgets();
	//! Save current tab to settings.
	void save_values();

	//! Set the \p qso and \p qso_number to component widgets.
	void qso(record* qso, qso_num_t qso_number);
	//! Returns the contest_scorer.
	contest_scorer* contest();

protected:
	//! Callback when selected tab is changed to reformat labels.
	static void cb_tabs(Fl_Widget* w, void* v);

	//! The current QSO record.
	record* qso_;

	// Widgets
	qso_details* details_;       //!< Displays QSO callsign "Worked before" status.
	qso_dxcc* dxcc_;             //!< Displays "Worked before" status for entity, zones etc.
	qso_qsl_vwr* qsl_;           //!< Displays and controls QSL information for current qSO record.
	contest_scorer* contest_;    //!< Displays current contest information.

	//! Used to reopen the tab that was opened when the app was shut down
	int default_tab_;
};

