#include "display.h"
#include "engine.h"
#include "key_handler.h"
#include "decoder.h"
#include "wave_gen.h"

#include <string>

#include <FL/Fl.H>

using namespace std;

// Program strings
string COPYRIGHT = "\302\251 Philip Rose GM3ZZA 2018. All rights reserved.";
string PROGRAM_ID = "ZZAKEYER";
string PROG_ID = "ZKY";
string PROGRAM_VERSION = "0.0.1";
string VENDOR = "GM3ZZA";

engine* engine_ = nullptr;
key_handler* key_handler_ = nullptr;
display* display_ = nullptr;
wave_gen* wave_gen_ = nullptr;
decoder* decoder_ = nullptr;

int main(int argc, char** argv)
{
	// Set default font size for all widgets
	FL_NORMAL_SIZE = 10;
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, "Courier New");
	Fl::set_font(FL_COURIER_BOLD, "Courier New Bold");
	Fl::set_font(FL_COURIER_ITALIC, "Courier New Italic");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "Courier New Bold Italic");

	// Initialise thread handling in FLTK
	Fl::lock();

	// Create the various units
	engine_ = new engine;
	key_handler_ = new key_handler;
	wave_gen_ = new wave_gen;
	decoder_ = new decoder;

	display_ = new display(1000, 1000);

	display_->show(argc, argv);
	// Start decode
	decoder_->start();

	// Connect wave_gen to source of signals
	wave_gen_->callback(engine::cb_signal, engine_);
	// Start wave_gen
	wave_gen_->initialise_pa();

	return Fl::run();
}