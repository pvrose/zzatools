#ifndef __CALLBACK__
#define __CALLBACK__

#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wunused-function"
#endif


#include "utils.h"
#include "drawing.h"

#include <string>
#include <stdexcept>
#include <cmath>

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>

using namespace std;

extern bool DEBUG_ERRORS;

	//! Datatypes to pass to radio and or_check button callbacks 
	struct radio_param_t {
		//! Value assigned to radio button
		unsigned int value;
		//! The integer object to receive the value
		int* attribute;
		//! Default constructor
		radio_param_t() {
			value = 0;
			attribute = nullptr;
		}
		//! Constructor to initialise structure
		radio_param_t(unsigned int a, int* b) {
			value = a;
			attribute = b;
		}
	};

	// Common callbacks

	//! Template callback to get a DATA value from a WIDGET widget - compiler should catch incompatible value types
	template <class WIDGET, class DATA>
	static void cb_value(Fl_Widget* w, void* v) {
		DATA value = (DATA)((WIDGET*)w)->value();
		DATA* target = (DATA*)v;
		*target = value;
	}
	//! Template callback to get text as a DATA item from a WIDGET widget
	template <class WIDGET, class DATA>
	static void cb_text(Fl_Widget* w, void* v) {
		DATA text = ((WIDGET*)w)->text();
		DATA* target = (DATA*)v;
		*target = text;
	}
	//! Template callback to get an enum (DATA) item from a WIDGET widget that has int value
	template <class WIDGET, class DATA>
	static void cb_value_enum(Fl_Widget* w, void* v) {
		int value = ((WIDGET*)w)->value();
		DATA* target = (DATA*)v;
		*target = (DATA)value;
	}
	//! Callback to get an int value from a WIDGET widget that has char* value - returns 0 if first character is non-integer
	template <class WIDGET>
	static void cb_value_int(Fl_Widget* w, void* v) {
		const char* value = ((WIDGET*)w)->value();
		int* target = (int*)v;
		int i;
		try {
			i = stoi(value);
		}
		catch (invalid_argument&) {
			i = 0;
		}
		*target = i;
	}
	//! Callback to get an ull/uint64_t value from a WIDGET widget that has char* value - returns 0 if first character is non-integer
	template <class WIDGET>
	static void cb_value_ull(Fl_Widget* w, void* v) {
		const char* value = ((WIDGET*)w)->value();
		unsigned long long* target = (unsigned long long*)v;
		unsigned long long i;
		try {
			i = stoull(value);
		}
		catch (invalid_argument&) {
			i = 0;
		}
		*target = i;
	}
	//! Callback to get a float value from a widget that has char* - returns nan if not numeric
	template <class WIDGET>
	static void cb_value_float(Fl_Widget* w, void* v) {
		const char* value = ((WIDGET*)w)->value();
		float* target = (float*)v;
		float f;
		try {
			f = stof(value);
		}
		catch (invalid_argument&) {
			f = (float)nan("");
		}
		*target = f;
	}
	//! Callback to get a double value from a widget that has char* - returns nan if not numeric
	template <class WIDGET>
	static void cb_value_double(Fl_Widget* w, void* v) {
		const char* value = ((WIDGET*)w)->value();
		double* target = (double*)v;
		double d;
		try {
			d = stod(value);
		}
		catch (invalid_argument&) {
			d = nan("");
		}
		*target = d;
	}
	//! Callback to get an tm value from a widget that has char*
	template <class WIDGET>
	static void cb_value_tm(Fl_Widget* w, void* v) {
		const char* value = ((WIDGET*)w)->value();
		tm* target = (tm*)v;
		string_to_tm(string(value), *target, "%Y%m%d");
	}
	//! Callback to get a radio value from a set of radio buttons
	static void cb_radio(Fl_Widget* w, void* v) {
		radio_param_t* param = (radio_param_t*)v;
		*(param->attribute) = param->value;
	}
	//! Tooltip timer callback - deletes the tip window
	static void cb_timer_tip(void* v) {
		// delete the tip window displayed
		Fl_Widget* w = (Fl_Widget*)v;
		Fl::delete_widget(w);
	}

	//! Callback that ors a number of check boxes (Fl_Check_Button or Fl_Light_Button) that implement a bit-wise attribute
	template <class WIDGET>
	static void cb_ch_or(Fl_Widget* w, void* v) {
		radio_param_t* param = (radio_param_t*)v;
		int value = ((WIDGET*)w)->value();
		if (value) {
			// set the bit indicated by param->value
			*(param->attribute) |= param->value;
		}
		else {
			// clear the bit indicated by param->value
			*(param->attribute) &= ~param->value;
		}
	}

	//! Callback to provide standard behaviour when clicking in an Fl_Tree
	
	//! Open a tree item: But keep the children close
	//! Select an item: open it and its children
	//! Deselect: close it and its children
	static void cb_tree(Fl_Widget* w, void* v) {
		Fl_Tree* that = (Fl_Tree*)w;
		Fl_Tree_Item* item = that->callback_item();
		switch (that->callback_reason()) {
		case FL_TREE_REASON_OPENED:
			// Keep the children closed
			for (int i = 0; i < item->children(); i++) {
				item->child(i)->close();
			}
			break;
		case FL_TREE_REASON_CLOSED:
			// De-select the item so next time it's clicked it get selected
			item->select(false);
			break;
		case FL_TREE_REASON_SELECTED:
			// open item and children (may need to recurse this all the way
			item->open();
			for (int i = 0; i < item->children(); i++) {
				Fl_Tree_Item* item_i = item->child(i);
				item_i->open();
				for (int j = 0; j < item_i->children(); j++) {
					Fl_Tree_Item* item_j = item_i->child(j);
					item_j->open();
					for (int k = 0; k < item_j->children(); k++) {
						Fl_Tree_Item* item_k = item_j->child(k);
						item_k->open();
						for (int l = 0; l < item_k->children(); l++) {
							Fl_Tree_Item* item_l = item_k->child(l);
							item_l->open();
						}
					}
				}
			}
			break;
			// This causes problems if try and open a child of the current selection
			//case FL_TREE_REASON_DESELECTED:
			//	// Close item and all children
			//	item->close();
			//	break;
		default: // Do nothing
			break;
		}
	}

	//! Returns the selected text into the string pointed at by v
	static void cb_choice_text(Fl_Widget* w, void* v) {
		Fl_Choice* choice = (Fl_Choice*)w;
		string* enum_value = (string*)v;
		char temp[128];
		choice->item_pathname(temp, sizeof(temp) - 1);
		// If there is a value get its text - note as pathname it will be preceded by a '/'.
		if (temp[0] != 0) {
			char* first_slash = strchr(temp, '/');
			*enum_value = &first_slash[1];
		}
		else {
			*enum_value = "";
		}
	}

#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
#endif
