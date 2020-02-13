#ifndef __RIG_DLG73__
#define __RIG_DLG73__

#include "../zzalib/win_dialog.h"

#include <string>

#include <FL/Fl_Window.H>

using namespace zzalib;
using namespace std;

namespace zza7300 {

	class rig_dlg73 : public win_dialog
	{

	public:
		rig_dlg73(int W, int H, const char* label);
		~rig_dlg73();

	protected:
		// Create the form
		void create_form();
		// Load data from settings
		void load_values();
		// Save data to settings
		void save_values();

		// OK Button
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// Cancel button
		static void cb_bn_cancel(Fl_Widget* w, void* v);

		// FLRig parameters
		// 4-byte IPv4 address
		int ip_address_[4];
		// Port number
		int ip_port_;
		// XML-RPC resource identifier
		string ip_resource_;

	};

}
#endif