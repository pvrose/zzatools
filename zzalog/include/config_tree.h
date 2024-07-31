#ifndef __CONFIG_TREE__
#define __CONFIG_TREE__

#include <string>
#include <FL/Fl_Tree.H>

using namespace std;

class Fl_Tree_Item;
class Fl_Preferences;



	// This class implements a tree that displays the contents of the settings file
	class config_tree :
		public Fl_Tree
	{
	public:
		config_tree(int X, int Y, int W, int H, const char* label = nullptr);
		virtual ~config_tree();
		// Create the view
		void create_tree();

	protected:
		// Type of node in the tree
		enum node_t {
			LEAF,      // Has no further subdivision
			BRANCH     // has child modes
		};

		// Add a leaf node at the current point
		void add_leaf(Fl_Tree_Item* parent, string label, string value);
		// Add a new branch at the current point
		void add_branch(Fl_Tree_Item* parent, Fl_Preferences* settings);
		// Delete the tree
		void delete_tree();
	};

#endif
