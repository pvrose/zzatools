#pragma once

#include <chrono>
#include <list>
#include <map>

class Fl_Text_Display;
class Fl_Window;

using namespace std;
using namespace std::chrono;


class logger
{
public:
	logger();
	~logger();

	// Log the event
	void log_event(char* event);

	// Set enable
	void enable(bool v);
	bool enable();
	// Clear log
	void clear();

	// Display log
	void display();

protected:
	Fl_Text_Display* viewer_;
	Fl_Window* wv_;
	// The log
	list<char*> log_;
	// Enable flag
	bool enabled_;

};

