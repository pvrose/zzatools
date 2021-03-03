#include "config_tree.h"
#include "../zzalib/drawing.h"

#include <map>

#include <FL/fl_draw.H>
using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;

// Constructor
config_tree::config_tree(int X, int Y, int W, int H, const char* label) :
	Fl_Tree(X, Y, W, H, label)
{
	// Set the tree parameters
	sortorder(FL_TREE_SORT_ASCENDING);
	item_labelfont(FONT);
	item_labelsize(FONT_SIZE);
	// Create the tree
	create_tree();
}

// Destructor
config_tree::~config_tree()
{
	delete_tree();
}

// Called to (re)create the tree
void config_tree::create_tree() {
	// Remove existing data
	delete_tree();
	// Create the root branch
	add_branch(nullptr, settings_);
}

// Add a leaf node - label plus value
void config_tree::add_leaf(Fl_Tree_Item* parent, string label, string value) {
	Fl_Tree_Item* leaf;
	char text[256];
	sprintf(text, "%s: %s", label.c_str(), value.c_str());
	if (parent == nullptr) {
		// Adding the root node
		leaf = add(text);
	}
	else {
		// Adding a leaf to a branch
		leaf = parent->add(prefs(), text);
	}
	if (leaf) {
		// Set its attributes
		leaf->labelsize(FONT_SIZE);
		leaf->labelcolor(FL_BLUE);
	}
}

// Add a branch node - adds all the children as well 
void config_tree::add_branch(Fl_Tree_Item* parent, Fl_Preferences* settings) {
	Fl_Tree_Item* branch = nullptr;
	if (parent != nullptr) {
		// Add the branch with the name of the settings group
		branch = parent->add(prefs(), settings->name());
	}
	else {	
		// Create the root item as the branch
		branch = new Fl_Tree_Item(this);
		root(branch);
		branch->labelfont(item_labelfont() | FL_BOLD);
		root_label("All Settings");
	}
	if (branch) {
		// Set attributes
		branch->labelsize(FONT_SIZE);
	}
	// now get the nodes in this settings group
	// Entries
	int num_entries = settings->entries();
	// For each entry
	for (int i = 0; i < num_entries; i++) {
		// Add a leaf with it's name and value
		char *temp;
		settings->get(settings->entry(i), temp, "");
		string value = temp;
		free(temp);
		add_leaf(branch, settings->entry(i), value);
	}
	// Further settings groups
	int num_groups = settings->groups();
	// For each group
	for (int i = 0; i < num_groups; i++) {
		// Add a branch for that group
		Fl_Preferences group_settings(settings, i);
		add_branch(branch, &group_settings);
	}
}

// Delete the tree.
void config_tree::delete_tree() {
	clear();
}