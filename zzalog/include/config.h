#ifndef __CONFIG__
#define __CONFIG__

#include <set>
#include <vector>

#include <FL/Fl_Window.H>

using namespace std;

class Fl_Widget;
class Fl_Tabs;

class page_dialog;

// config window default sizes
const int WCONFIG = 600;
const int HCONFIG = 500;


	// This class provides a window to display all the config dialogs as separate tabbed panes
	class config : public Fl_Window

	{

	public:

		// The tab types in the config dialog - order is speific!
		enum cfg_dialog_t {
			DLG_WEB,
			DLG_COLUMN,
			DLG_STATION,
			DLG_USER,
			DLG_QSLE,
			DLG_CONTEST,
			DLG_ALL,
			DLG_X
		};

	public:
		config(int W, int H, const char* label = nullptr);
		~config();

		// callbacks
		static void cb_bn_cal(Fl_Widget* w, long arg);
		// Tab released
		static void cb_tab(Fl_Widget* w, void* v);
		void update();
		// Test whether any we've had OK or Cancel
		bool active();
		// Clear active flag
		void inactive();
		// Set the tab
		void set_tab(cfg_dialog_t active);
		// Set and Get the tab
		Fl_Widget* get_tab(cfg_dialog_t active);

		// protected methods
	protected:
		// Set the tab and the label
		void set_label(cfg_dialog_t active);
		// 
		void enable_widgets();

		// protected attributes
	protected:
		// The set of tabs
		Fl_Tabs* tabs_;
		// The currently active dialog
		Fl_Widget * settings_view_;
		// Any widgets that need updating on record selection
		set<page_dialog*> updatable_views_;
		// Active flag
		bool active_;
		// List of children
		vector<cfg_dialog_t> children_ids_;



	};
#endif
