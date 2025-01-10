#include "key_out.h"

key_out::key_out() :
	the_key_(false)
{
}

key_out::~key_out() {

}

// Set the key value
void key_out::set_key(bool v) {
	bool changed = (v != the_key_);
	the_key_ = v;
	// Inform all objects that want to know that the value has changed
	if (changed) {
		for (auto it = callbacks_.begin(); it != callbacks_.end(); it++) {
			it->second(it->first);
		}
	}
}

// Get the key value
bool key_out::get_key() {
	return the_key_;
}

// Add the object to the callback list - notify when key changes state
void key_out::add_callback(void* object, cb_keyout* cb) {
	callbacks_[object] = cb;
}

// Remove the object from the callback list
void key_out::remove_callback(void* object) {
	if (callbacks_.find(object) != callbacks_.end()) {
		callbacks_.erase(object);
	}
}
