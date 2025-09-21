ZZALOG is an amateur radio logging application developed bt Philip Rose for personal use.

I am now making this publicly available.

Windows Installation
--------------------

A file ZZALOG-3.6.7.msi is available from https://sourceforge/projects/zzalog. 
Please follow instructions to install this. ZZALOG will be installed in 
"C:\Program Files\GM3ZZA\ZZALOG\". 
Data used by ZZALOG will be installed in "C:\ProgramData\GM3ZZA\ZZALOG". 

The source may be cloned from https://github.com/pvrose/zzatools. 
This includes an MSVC solution containing the project. 
However this has dependencies on the following libraries: 
FLTK, HAMLIB and LIBCURL and the flow to install these
libraries has not yet been tested on a clean machine. 
The documentation also requires the use of Doxygen and again 
the flow has not yet been tested on a clean machine.

Linux Installation
------------------

ZZALOG is available in source form. It may be cloned from 
"https://github.com/pvrose/zzatools". 
It has dependencies on the external libraries: fltk, hamlib, libcurl and 
nlohmann/json.

To compile:

cd [install directory]
make

To generate documentation:

make documents

To install the application and any necessary data

make install 

Data for the application will be installed in /etc/fltk/GM3ZZA/ZZALOG.