/*! Types and constants used by file related classes
*/
#ifndef __FILES__
#define __FILES__

//! Results from loading a file
enum load_result_t {
	LR_GOOD,     //!< loaded OK.
	LR_BAD,      //!< failed to complete loading
	LR_EOF,      //!< EOF read
};

#endif
