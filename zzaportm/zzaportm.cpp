// zzaportm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "window.h"

#include <iostream>
#include <string>

#include <FL/Fl.H>

using namespace std;
using namespace zzaportm;

int main(int argc, char** argv)
{
	window* main_window = new window(10, 10, "ZZAPORTM: Port tester");

	return Fl::run();
}

