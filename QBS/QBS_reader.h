#pragma once

#include "../zzalib/xml_reader.h"
#include "QBS_data.h"
#include <map>
#include <string>
#include <list>

using namespace std;
using namespace zzalib;

class QBS_reader :
    public xml_reader
{
public:

	enum qbs_element_t {
		QBS_NONE,		// Not in an element
		QBS,			// <QBS>......</QBS>
		QBS_AMATEURS,   // <amateurs>...</amateurs>
		QBS_AMATEUR,    // <amateur callsign=(s)>...</amateur>
		QBS_SASE,       // <sase>..</sase>
		QBS_CARD,       // <card>..</card>
		QBS_VALUE,      // <[value]>...</[value]>
			// used for info items in AMATEUR, BATCH_VAL and BATCH_DEF
        QBS_BATCH_VAL,  // <batch_val name=(s)>...</batch_val>
		QBS_BATCHES,    // <batches>...</batches>
		QBS_BATCH_DEF   // <batch_def name=(s)>...</batch_def>
	};

	QBS_reader();
	~QBS_reader();

	// load data to QBS_data
	bool load_data(QBS_data* data, istream& in);

	// Overloadable XML handlers
	// Start 
	virtual bool start_element(string name, map<string, string>* attributes);
	// End
	virtual bool end_element(string name);
	// Special element
	virtual bool declaration(xml_element::element_t element_type, string name, string content);
	// Processing instruction
	virtual bool processing_instr(string name, string content);
	// characters
	virtual bool characters(string content);

protected:
	// Start QBS element
	bool start_qbs();
	// Start amateurs element
	bool start_amateurs();
	// Start amateur element
	bool start_amateur(map<string, string>* attributes);
	// Start batch_val element
	bool start_batch_val(map<string, string>* attributes);
	// Start SASE element
	bool start_sase();
	// Start Card element
	bool start_card();
	// Start batches element
	bool start_batches();
	// Start batch_def element
	bool start_batch_def(map<string, string>* attributes);
	// Start individual value items
	bool start_value(string name);

	QBS_data* the_data_;

	// Current callsign
	string callsign_;
	// Current batch name
	string batch_id_;
	// Element processing stack
	list<qbs_element_t> elements_;
	// counts for progress
	long file_size_;
	long previous_count_;
	// Input data stream
	istream* in_;
	// Non-specific element
	string item_name_;
	// Current item value
	string value_;


	

};

