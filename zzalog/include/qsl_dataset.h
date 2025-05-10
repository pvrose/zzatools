#pragma once

#include "qsl_data.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>

using namespace std;

// Class to manage QSL designs

class qsl_dataset
{
public:

	qsl_dataset();
	~qsl_dataset();

	// Return the QSL design associated with the callsign and QSL type
	qsl_data* get_card(string callsign, qsl_data::qsl_type type, bool create);
	// Get the path to settings/Datapath/QSLs
	string get_path();

	// Mark data dirty
	void dirty(qsl_data* card);
	// Store carddesigns
	void save_data();
	// Save XML data
	void save_xml(Fl_Preferences& settings);
	// Load data from file
	void load_items(qsl_data* data);


protected:
	// Read card designs
	void load_data();
	//// Read from preferences
	//void load_prefs(Fl_Preferences& settings);
	// Read from XML fiel
	bool load_xml(Fl_Preferences& settings);
	// Get XML file from settings
	string xml_file(Fl_Preferences& settings);

	map<qsl_data::qsl_type, map<string, qsl_data*>* > data_;
	// PAth to QSL data
	string qsl_path_;

};


