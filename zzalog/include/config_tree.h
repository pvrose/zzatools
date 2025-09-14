#ifndef __CONFIG_TREE__
#define __CONFIG_TREE__

#include <string>
#include <FL/Fl_Tree.H>



class Fl_Tree_Item;
class Fl_Preferences;



	//! This class implements a tree that displays the contents of the settings file
	class config_tree :
		public Fl_Tree
	{
	public:
		//! Constructor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		config_tree(int X, int Y, int W, int H, const char* L = nullptr);
		//! Destructor.
		virtual ~config_tree();

		//! Override Fl_Tree::handle().
		 
		//! Intercept click event to gain focus for keyboard F1 to open userguide.
		virtual int handle(int event);


		//! Create the view.
		void create_tree();

	protected:
		//! Type of node in the tree
		enum node_t {
			LEAF,      //!< Has no further subdivision
			BRANCH     //!< has child modes
		};

		//! Add a leaf node at the current point.
		
		//! \param parent the point at which to add a leaf node.
		//! \param label the name of the setting.
		//! \param value the value of the setting.
		void add_leaf(Fl_Tree_Item* parent, std::string label, std::string value);
		//! Add a new branch at the current point.
		
		//! \param parent the point at which to add a new branch.
		//! \param settings the group of settings that form this branch.
		void add_branch(Fl_Tree_Item* parent, Fl_Preferences& settings);
		//! Delete the tree.
		void delete_tree();
	};

#endif
