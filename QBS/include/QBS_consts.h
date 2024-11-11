#pragma once

#include "drawing.h"

#include <string>

using namespace std;
//using namespace zzalib;

const string COPYRIGHT = "© Philip Rose GM3ZZA 2022";
const string PROGRAM_ID = "QBS";
const string PROG_ID = "QBS";
const string VENDOR = "GM3ZZA";

// Tip window dimensions
const unsigned int TIP_WIDTH = 200;

enum command_t : char {
	INHERIT = 'I',               // Inherit cards (not sent or kept)     
	CARDS = 'C',                 // Receive cards  
	BATCH = 'B',                 // New batch 
	TOTALISE = 'T',              // Sum all cards and calls in batch
	SASES = 'E',                 // Receive SASEs
	OUTPUT = 'O',                // Move cards to out-box
	USE = 'U',                   // Use SASEs
	KEEP = 'K',                  // Keep cards to next batch
	POST = 'P',                  // Post out-box
	DISPOSE = 'D',               // Move cards to disposal queue
	RECYCLE = 'R',               // Move cards from disposal queue to blue bin
	HAM_INFO = 'H',              // Ham data
	COMMENT = '#',               // Used for comments
	ADJUST = 'A'                 // Adjust number of cards
};

enum process_mode_t {
	INITIAL,                     // Initial
	DORMANT,                     // Waiting to receive a batch
	LOG_CARD,                    // Log cards received ad-hoc
	LOG_SASE,                    // Log SASEs received
	LOG_BATCH,                   // Log new batch received
	SORTING,                     // Received cards - sorting into callsigns
	PROCESSING,                  // Processing a callsigns cards - for posting, holding or pending disposal
	POSTING,                     // Maerk cards destined forposting as posted
	FINISHING,                   // Mark cards as pending disposal
	RECYCLING,                   // Mark cards as having been disposed
	BATCH_SUMMARY,               // Produce batch summary report
	BATCH_REPORT,                // Produce batch callsign report
};

enum reading_mode_t {
	IMPORT,
	READING,
	WRITING
};

enum navigate_t {
	PREV_FIRST,         // Go to beginning
	PREV_MAJOR,         // major step back
	PREV_MINOR,         // minor step back
	NEXT_MAJOR,         // major step forward
	NEXT_MINOR,         // minor step forward
	NEXT_LAST           // Go to end
};
