#ifndef __ADIF_TREE__
#define __ADIF_TREE__

#include "view.h"
#include "spec_data.h"
#include "fields.h"

#include <string>

#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Widget.H>

using namespace std;


	// This class provides a view that draws the ADIF specification data into a tree
	class spec_tree :
		public Fl_Tree, public view

	{
	public:
		spec_tree(int X, int Y, int W, int H, const char* label, field_ordering_t app);
		~spec_tree();

		// inherited from view
		virtual void update(hint_t hint, record_num_t record_num_1, record_num_t record_num_2 = 0);
		// Delete the tree
		void delete_tree();

		// methods
		// Populate the tree control with the data from the selected prefixes
		void populate_tree(bool activate);
		// Change font
		void set_font(Fl_Font font, Fl_Fontsize size);


	protected:
		// Add the ADIF spec item to the tree
		void insert_adif_spec(Fl_Tree_Item* parent, const spec_dataset& dataset, const string& name);

		// Attributes
		// Hang point for primary administrative subdivision
		Fl_Tree_Item* pas_item_;
		// Hang point for secondary administrative subdivision
		Fl_Tree_Item* sas_item_;
		// Hang point for submodes
		Fl_Tree_Item* sub_item_;
		// Font and size
		Fl_Font font_;
		Fl_Fontsize fontsize_;


	};
#endif
