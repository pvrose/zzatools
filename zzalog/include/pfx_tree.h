#ifndef __PREFIX_TREE__
#define __PREFIX_TREE__

#include "view.h"
#include "prefix.h"
#include "record.h"
#include "fields.h"

#include <string>
#include <vector>
#include <list>

#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>

using namespace std;



	enum report_filter_t;
	enum report_item_t;

	// This class displays the prefix database in tree format
	class pfx_tree :
		public Fl_Tree, public view
	{
	public:
		// Constructor uses the constructor parameters for both the widget and view
		pfx_tree(int X, int Y, int W, int H, const char* label, field_ordering_t app);
		virtual ~pfx_tree();

	public:
		// inherited from view
		virtual void update(hint_t hint, unsigned int record_num_1, unsigned int record_num_2 = 0);
		// Delete the tree
		void delete_tree();

		// methods
		// Populate the tree control with the data from the selected prefixes
		void populate_tree(bool activate);
		// Add a prefix at the indicated item handle to the tree top-down
		void insert_child(Fl_Tree_Item* parent, prefix* prefix);
		// Add a prefix at the indicated item handle in the tree bottom-up
		void insert_parent(Fl_Tree_Item*& child, prefix* prefix);
		// Scan the tree from the indicated item to get the item below it matches the text 
		Fl_Tree_Item* get_item(Fl_Tree_Item* item, string text);
		// Convert the prefix data into the text string displayed in the tree
		void get_data(prefix* prefix, vector<string>& asData);
		// Add the prefix and its children to the tree
		void add_prefix(record* record, bool special);

		// Set the filter to display - from menu
		void set_filter(report_filter_t filter);
		// Set the items to display - from menu
		void set_items(report_item_t items);
		// Get the items
		report_item_t get_items();
		// Add details
		void add_details(bool enable);
		// Change font
		void set_font(Fl_Font font, Fl_Fontsize size);

		// attributes
		// Filter for reference data
		report_filter_t filter_;
		// Items to display
		report_item_t items_;
		// include all fields of a prefix data record
		bool include_fields_;
		// number of current selected record
		int record_num_;
		// list of prefixes selected by filter
		vector<prefix*> prefixes_;
		// Font and size
		Fl_Font font_;
		Fl_Fontsize fontsize_;

	};
#endif