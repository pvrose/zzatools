#ifndef __CONFIG__
#define __CONFIG__

#include <set>
#include <vector>

#include <FL/Fl_Window.H>



class Fl_Widget;
class Fl_Tabs;

class page_dialog;

// config window default sizes
const int WCONFIG = 600;
const int HCONFIG = 500;


	//! This class provides a window to display all the config dialogs as separate tabbed panes.
	class config : public Fl_Window

	{

	public:

		//! Identifiers for each configuration tab - order is speific!
		enum cfg_dialog_t {
			DLG_WEB,        //!< Web location dialog.
			DLG_COLUMN,     //!< Columns (aka fields) format dialog.
			DLG_USER,       //!< User formatting configuration dialog.
			DLG_QSLE,       //!< QSL Editor.
			DLG_CONTEST,    //!< Contest setup dialog.
			DLG_ALL,        //!< Tree view of settiings.
			DLG_X
		};

	public:
		//! Constructor.
		
		//! \param W width. 
		//! \param H height.
		//! \param L label.
		config(int W, int H, const char* L = nullptr);
		//! Destructor.
		~config();

		//! Callback for OK, CANCEL or SAVE buttons
		//! 
		//! - SAVE: Action the changes and remain open
		//! - OK: Action the changes and close the dialog.
		//! - CANCEL: Do not action the changes.
		static void cb_bn_cal(Fl_Widget* w, long arg);
		//! Callback when a different tab is selected.
		static void cb_tab(Fl_Widget* w, void* v);
		//! Update any dialog that requires updating with selected record change.
		void update();
		//! Change the active tab.
		
		//! \param active identifier (cfg_dialog_t) of the new tab,
		void set_tab(cfg_dialog_t active);
		//! Change the activve tab.
		
		//! \param active identifier (cfg_dialog_t) of the new tab,
		//! \return pointer to the new tab.
		Fl_Widget* get_tab(cfg_dialog_t active);

		// protected methods
	protected:
		//! Set the window label to reflect the selected tab
		
		//! \param active identifier (cfg_dialog_t) of the tab whose description to std::set as the window label.
		void set_label(cfg_dialog_t active);
		//! Format the tabs according to selection. 
		 
		//! Uses ZZALOG standard for
		//! label formats of Fl_Tabs instances.
		void enable_widgets();

		// protected attributes
	protected:
		//! The main component with its own component tabs.
		Fl_Tabs* tabs_;
		//! The currently active dialog
		Fl_Widget * settings_view_;
		//! Any widgets that need updating on record selection
		std::set<page_dialog*> updatable_views_;
		//! List of the intantiated tabs.
		std::vector<cfg_dialog_t> children_ids_;


	};
#endif
