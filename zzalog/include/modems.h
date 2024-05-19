#ifndef __MODEMS__
#define __MODEMS__

#include <string>

using namespace std;

// List of modem apps
enum modem_t {
    FLDIGI,
    WSJTX,
    NUMBER_APPS
};

const string MODEM_NAMES[NUMBER_APPS] = { "FlDigi", "WSJT-X"};

#endif