#include "logger.h"

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>

#include <iostream>

logger::logger() {
}

logger::~logger() {
	clear();
}

// Log the event
void logger::log_event(char* event) {
	if (enabled_) {
		system_clock::time_point tp = system_clock::now();
		milliseconds ms = duration_cast<milliseconds>(tp.time_since_epoch());
		char* line = new char[128];
		snprintf(line, 128, "%lld: %s", ms.count(), event);
		log_.push_back(line);
		//cout << line;;
	}
}

// Set enable
void logger::enable(bool v) {
	enabled_ = v;
}

bool logger::enable() { return enabled_; }

// Clear log
void logger::clear() {
	for (auto it = log_.begin(); it != log_.end(); it++) {
		delete *it;
	}
	log_.clear();
}

// Display log
void logger::display() {
	Fl_Window* w = new Fl_Window(200, 200);
	Fl_Text_Display* td = new Fl_Text_Display(0, 0, 200, 200);
	Fl_Text_Buffer* b = new Fl_Text_Buffer;
	td->buffer(b);
	for (auto it = log_.begin(); it != log_.end(); it++) {
		b->append(*it);
	}
	w->resizable(td);
	w->end();
	w->show();
}
