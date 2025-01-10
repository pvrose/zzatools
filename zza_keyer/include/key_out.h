#pragma once

#include <map>

using namespace std;

class key_out
{
public:
	key_out();
	~key_out();

	// Set the key value
	void set_key(bool v);

	// Get the key value
	bool get_key();

	typedef void cb_keyout(void* v);

	// Add the object to the callback list - notify when key changes state
	void add_callback(void* object, cb_keyout* cb);
	// Remove the object from the callback list
	void remove_callback(void* object);


protected:

	// The combined key value
	bool the_key_;

	// The list of callbacks
	map <void*, cb_keyout*> callbacks_;

};

