// zzaportm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "window.h"

#include <iostream>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>

using namespace std;
using namespace zzaportm;

const string VENDOR = "GM3ZZA";
const string PROGRAM_ID = "ZZAPORTM";

Fl_Preferences* settings_ = nullptr;

int main(int argc, char** argv)
{
	// Create the settings before anything else 
	settings_ = new Fl_Preferences(Fl_Preferences::USER, VENDOR.c_str(), PROGRAM_ID.c_str());

	window* main_window = new window(10, 10, "ZZAPORTM: Port tester");

	int code = Fl::run();

	// Delete in order of construction
	delete main_window;
	delete settings_;

	return code;
}

