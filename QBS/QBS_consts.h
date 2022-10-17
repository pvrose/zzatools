#pragma once

#include "../zzalib/drawing.h"
#include <string>

using namespace std;
using namespace zzalib;

const string COPYRIGHT = "© Philip Rose GM3ZZA 2022";
const string PROGRAM_ID = "QBS";
const string PROG_ID = "QBS";
const string VENDOR = "GM3ZZA";

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
	COMMENT = '#'                // Used for comments
};

//enum subcommand_t : char {
//	RSGB = 'R',                  // RSGB number
//	INFO = 'I',                  // Notes/information
//	NAME = 'N',                  // Name
//	EMAIL = 'E',                 // e-Mail
//	POSTAL = 'P'                 // POstal address
//};

enum process_mode_t {
	INITIAL,                     // Initial
	IMPORT,                      // Importing CSV file
	READING,                     // Reading existing QBS file
	DORMANT,                     // Waiting to receive a batch
	ACTIVE                       // Received a batch - allow dialogs
};

enum action_t {
	NONE = 0,                    // No action
	RECEIVE_BATCH,               // Receive a batch of cards (from bureau)
	RECEIVE_CARD,                // Receive individual cards 
	RECEIVE_SASE,                // Receive envelopes
	SEND_CARDS,                  // stuff envelopes
	DISPOSE_CARDS,               // Mark cards for recycling
	POST_CARDS,                  // Post stuffed envelopes
	RECYCLE_CARDS,               // Recycle cards
	DISPOSE_SASE                 // Recycle envelopes
};

enum navigate_t {
	SUMM,               // display sum of all items
	PREV_MAJOR,         // major step back
	PREV_MINOR,         // minor step back
	NEXT_MAJOR,         // major step forward
	NEXT_MINOR          // minor step forward
};
