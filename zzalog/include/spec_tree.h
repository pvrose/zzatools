#ifndef __ADIF_TREE__
#define __ADIF_TREE__

#include "view.h"
#include "fields.h"

#include <string>

#include <FL/Enumerations.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>

using namespace std;

typedef size_t qso_num_t;
struct spec_dataset;


	//! This class provides a view that draws the ADIF specification data into a tree
	class spec_tree :
		public Fl_Tree, public view

	{
	public:
		//! Constructor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		//! \param app Used by view to format this widget.
		spec_tree(int X, int Y, int W, int H, const char* L, field_app_t app);
		//! Destructor
		~spec_tree();

		//! Inherited from view - only \p hint is used if format has been changed.
		virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0);
		//! Inherited from Fl_Tree to allow keyboard F1 to open userguide
		virtual int handle(int event);
		//! Delete the tree
		void delete_tree();

		// methods
		//! Populate the tree control with the data from the selected prefixes.

		//! \param activate Show this widget as the active tab.
		void populate_tree(bool activate);
		//! Change font
		void set_font(Fl_Font font, Fl_Fontsize size);


	protected:
		//! Add the ADIF spec item \p dataset with \p name to the tree at node \p parent.
		void insert_adif_spec(Fl_Tree_Item* parent, const spec_dataset& dataset, const string& name);

		// Attributes
		//! Hang point for primary administrative subdivision
		Fl_Tree_Item* pas_item_;
		//! Hang point for secondary administrative subdivision
		Fl_Tree_Item* sas_item_;
		//! Hang point for submodes
		Fl_Tree_Item* sub_item_;
		//! Font used
		Fl_Font font_;
		//! Font size used.
		Fl_Fontsize fontsize_;


	};
#endif
