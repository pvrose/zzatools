#include "sark_window.h"

#include <cstdio>

#include <FL/Fl_Preferences.H>
// included to allow windows specifics to be called
#include <FL/platform.H>

const char* COPYRIGHT = "\xA9 Philip Rose GM3ZZA 2023. All rights reserved.";
const char* PROGRAM_ID = "SARKAN";
const char* PROG_ID = "SAN";
const char* PROGRAM_TITLE = "SARK-100 Analyser";
const char* VERSION = "0.1.0";
const char* PROGRAM_VERSION = VERSION;
const char* VENDOR = "GM3ZZA";
extern int FL_NORMAL_SIZE;

sark_window* window_;

int main(int argc, char** argv) {
    char title[128];
    snprintf(title, sizeof(title), "%s %s", PROGRAM_TITLE, PROGRAM_VERSION);
    printf("%s\n", title);
    printf("%s\n", COPYRIGHT);
    // Change FLTK default
    FL_NORMAL_SIZE = 11;

    window_ = new sark_window(10, 10, title);

    window_->show(argc, argv);
    return Fl::run();
};