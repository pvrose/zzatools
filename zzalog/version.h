#ifndef __VERSION__
#define __VERSION__

#include <string>

using namespace std;

// Program strings
const string COPYRIGHT = "© Philip Rose GM3ZZA 2018. All rights reserved.\n (Prefix data, DX Atlas & OmniRig interfaces © Alex Shovkoplyas, VE3NEA.)";
const string PROGRAM_ID = "ZZALOG";
const string PROG_ID = "ZLG";
const string VERSION = "3.1.23";
#ifdef _DEBUG
const string PROGRAM_VERSION = VERSION + " (Debug)";
#else
const string PROGRAM_VERSION = VERSION;
#endif
const string VENDOR = "GM3ZZA";


#endif
