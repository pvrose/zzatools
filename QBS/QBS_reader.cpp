#include "QBS_reader.h"
#include "QBS_window.h"
#include "../zzalib/utils.h"

#include <iostream>

#include <FL/Fl_Window.H>

using namespace std;
using namespace zzalib;

QBS_reader::QBS_reader() :
	xml_reader()
{
}

QBS_reader::~QBS_reader() {

}

bool QBS_reader::load_data(QBS_data* data, istream& in) {
	in_ = &in;
	the_data_ = data;

	// calculate the file size
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	file_size_ = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);

	// Start
	cout << "Reading file" << endl;

	// Call the XML parser
	if (parse(in)) {
		// Read successful - complete progress
		cout << "Read complete" << endl;
		return true;

	}
	else {
		// Read failed - report failure
		cout << "Read failed" << endl;
		return false;
	}
}

// Overloaded XML handlers
// Start XML element <name [attributes]> 
bool QBS_reader::start_element(string name, map<string, string>* attributes) {
	// Default to upper case for all text comparisons
	string element_name = zzalib::to_lower(name);
	// decode OK
	bool error = false;

	// Start the specific element types
	if (element_name == "qbs") {
		error |= start_qbs();
	} else if (element_name == "amateurs") {
		error |= start_amateurs();
	} else if (element_name == "amateur") {
		error |= start_amateur(attributes);
	} else if (element_name == "batch_val") {
		error |= start_batch_val(attributes);
	} else if (element_name == "sases") {
		error |= start_sase();
	} else if (element_name == "cards") {
		error |= start_card();
	} else if (element_name == "batches") {
		error |= start_batches();
	} else if (element_name == "batch_def") {
		error |= start_batch_def(attributes);
	}
	else {
		error |= start_value(element_name);
	}

	// Tidy up attributes as we no longer need them
	delete attributes;

	if (error) {
		// Start element reported an error - report it to user
		char message[128];
		sprintf(message, "XML parsing incorrect for element type %s", element_name.c_str());
		cout << message << endl;
		return true;
	}

	return false;
}

// End XML element detected
bool QBS_reader::end_element(string name) {
	// Get the element name and convert it to upper case
	string element_name = to_lower(name);

	qbs_element_t element_type = elements_.back();
	elements_.pop_back();
	qbs_element_t encloser = elements_.back();

	bool ok = true;

	if (element_name == item_name_) {
		switch (encloser) {
		case QBS_AMATEUR:
		{
			QBS_data::ham_info_t* info = the_data_->get_info();

			if (item_name_ == "RSGB") {
				info->rsgb_id = stoi(value_);
			}
			else if (item_name_ == "INFO") {
				info->info = value_;
			}
			else if (item_name_ == "NAME") {
				info->name = value_;
			}
			else if (item_name_ == "E_MAIL") {
				info->e_mail = value_;
			}
			else if (item_name_ == "ADDRESS") {
				info->postal_address = value_;
			}
			else {
				printf("Unexpected item %s found in AMATEUR %s",
					element_name.c_str(), callsign_.c_str());
				ok = false;
			}
			break;
		}
		case QBS_SASE:
		{
			int n_value = stoi(value_);
			if (item_name_ == "RECEIVE") {
				the_data_->set_count(batch_id_, QI_SASES, QD_RECEIVE, n_value);
			}
			else if (item_name_ == "SEND") {
				the_data_->set_count(batch_id_, QI_SASES, QD_SEND, n_value);
			}
			else if (item_name_ == "DISPOSE") {
				the_data_->set_count(batch_id_, QI_SASES, QD_DISPOSE, n_value);
			}
			else {
				printf("Unexpected item %s found in SASES %s %s",
					element_name.c_str(), callsign_.c_str(), batch_id_.c_str());
				ok = false;
			}
			break;
		}
		case QBS_CARD:
		{
			int n_value = stoi(value_);
			if (item_name_ == "RECEIVE") {
				the_data_->set_count(batch_id_, QI_SASES, QD_RECEIVE, n_value);
			}
			else if (item_name_ == "SEND") {
				the_data_->set_count(batch_id_, QI_SASES, QD_SEND, n_value);
			}
			else if (item_name_ == "DISPOSE") {
				the_data_->set_count(batch_id_, QI_SASES, QD_DISPOSE, n_value);
			}
			else {
				printf("Unexpected item %s found in CARDS %s %s",
					element_name.c_str(), callsign_.c_str(), batch_id_.c_str());
				ok = false;
			}
			break;
		}
		case QBS_BATCH_DEF:
		{
			QBS_data::batch_data_t* b_data = the_data_->batch_data(batch_id_, true);
			if (item_name_ == "RECEIVED") {
				b_data->date_rcvd = value_;
			}
			else if (item_name_ == "DISPOSED") {
				b_data->date_disposed = value_;
			}
			else if (item_name_ == "WEIGHT") {
				b_data->weight_disposed = stof(value_);
			}
			else {
				printf("Unexpected item %s found in BATCH_DEF %s",
					element_name.c_str(), batch_id_.c_str());
				ok = false;
			}
		}
		}
	}
	else {
		switch (element_type) {
		case QBS:
			// Check expected name
			if (element_name != "qbs") {
				printf("End of QBS out of context.");
				ok = false;
			}
			break;
		case QBS_AMATEURS:
			// Check expected name
			if (element_name != "amateurs") {
				printf("End of AMATEURS out of context.");
				ok = false;
			}
			break;
		case QBS_AMATEUR:
			// Check expected
			if (element_name != "amateur") {
				printf("End of AMATEURS out of context.");
				ok = false;
			}
			callsign_ = "";
			break;
		case QBS_SASE:
			// Check expected
			if (element_name != "sases") {
				printf("End of SASE out of context.");
				ok = false;
			}
			break;
		case QBS_CARD:
			// Check expected
			if (element_name != "cards") {
				printf("End of SASE out of context.");
				ok = false;
			}
			break;
		case  QBS_BATCH_VAL:
			// Check expected
			if (element_name != "batch_val") {
				printf("End of BATCH_VAL out of context.");
				ok = false;
			}
			break;
		case QBS_BATCHES:
			// Check expected
			if (element_name != "batches") {
				printf("End of BATCHES out of context.");
				ok = false;
			}
			break;
		case QBS_BATCH_DEF:
			// Check expected
			if (element_name != "batch_def") {
				printf("End of BATCH_DEF out of context.");
				ok = false;
			}
			break;
		}
	}
	return !ok;
}

// XML data characters received
bool QBS_reader::characters(string content) {
	if (!elements_.empty()) {
		// We are processing elements - look at top of element stack
		// No element type expects non-whitespace data
		// TODO check white-space
	}
	return true;
}

// Explicitly ignore declaration tags - except all
bool QBS_reader::declaration(xml_element::element_t element_type, string name, string content) {
	return true;
}

// Explicitly ignore processing instructions
bool QBS_reader::processing_instr(string name, string content) {
	return true;
}

// Start QBS element
bool QBS_reader::start_qbs() {
	if (!elements_.empty()) {
		// Error
		printf("Top level QBS element found out of context.\n");
		return true;
	}
	// Add to stack
	elements_.push_back(QBS);
	// Clear the database
	the_data_->clear();
	return false;
}

// Start AMATEURS element
bool QBS_reader::start_amateurs() {
	if (elements_.empty() || elements_.back() != QBS) {
		printf("QBS_AMATEURS element found out of context.\n");
		return true;
	}
	// Note that we are processing <AMATEURS>...</AMATEURS>
	elements_.push_back(QBS_AMATEURS);
	return false;
}

// Start AMATEUR element: attributes:-
// <AMATEUR CALLSIGN="s">elements</AMATEUR>
bool QBS_reader::start_amateur(map<string, string>* attributes) {
	if (elements_.empty() || elements_.back() != QBS_AMATEURS) {
		printf("QBS_AMATEUR element found out of context.\n");
		return true;
	}
	elements_.push_back(QBS_AMATEUR);
	// Process attributes
	callsign_ = (*attributes)["CALLSIGN"];
	if (the_data_->set_record(callsign_, false)) {
		printf("Duplicate record for callsign %s - overwriting data\n", callsign_.c_str());
	}
	else {
		the_data_->set_record(callsign_, true);
	}
	return false;
}

// Start BATCH_VAL element
// <BATCH_VAL NAME=s>elements</BATCH>
bool QBS_reader::start_batch_val(map<string, string>* attributes) {
	if (elements_.empty() || elements_.back() != QBS_AMATEUR) {
		printf("QBS_BATCH_VAL element found out of context.\n");
		return true;
	}
	elements_.push_back(QBS_BATCH_VAL);
	string* batch_id = &(*attributes)["NAME"];
	return false;
}

// Start SASE element
// <SASES SEND=n RECEIVE=n DISPOSE=n KEEP=n />
bool QBS_reader::start_sase() {
	if (elements_.empty() || elements_.back() != QBS_BATCH_VAL) {
		printf("QBS_SASE element found out of context.\n");
		return true;
	}
	elements_.push_back(QBS_SASE);
	return false;
}

// Start CARD element
// <CARDS SEND=n RECEIVE=n DISPOSE=n KEEP=n />
bool QBS_reader::start_card() {
	if (elements_.empty() || elements_.back() != QBS_BATCH_VAL) {
		printf("QBS_CARD element found out of context.\n");
		return true;
	}
	elements_.push_back(QBS_CARD);
	return false;
}

// Start BATCHES element
bool QBS_reader::start_batches() {
	if (elements_.empty() || elements_.back() != QBS) {
		printf("QBS_BATCHES element found out of context.\n");
		return true;
	}
	// Note that we are processing <BATCHES>...</BATCHES>
	elements_.push_back(QBS_BATCHES);
	return false;
}

// Start BATCH_DEF element
// <BATCH_DEF NAME=s>
bool QBS_reader::start_batch_def(map<string, string>* attributes) {
	if (elements_.empty() || elements_.back() != QBS_BATCHES) {
		printf("QBS_BATCH_DEF element found out of context.\n");
		return true;
	}
	elements_.push_back(QBS_BATCH_DEF);
	batch_id_ = (*attributes)["NAME"];
	QBS_data::batch_data_t* record = the_data_->batch_data(batch_id_, false);
	if (record != nullptr) {
		printf("Duplicate record for batch %s - overwriting data\n", batch_id_.c_str());
	}
	else {
		record = the_data_->batch_data(batch_id_, true);
	}
	return false;
}

// Start item elements
// <ITEM_NAME><DATA></ITEM_NAME>
bool QBS_reader::start_value(string name) {
	if (elements_.empty() || 
		(elements_.back() != QBS_AMATEUR && 
			elements_.back() != QBS_SASE &&
			elements_.back() != QBS_CARD &&
			elements_.back() != QBS_BATCH_DEF)) {
		// We must be processing specific elements
		char temp[128];
		sprintf(temp, "Field element %s found out of context.", name.c_str());
		string message = temp;
		if (!report_error(message, true)) {
			return true;
		}
	}
	// We are in a field element
	elements_.push_back(QBS_VALUE);
	// Capitalise field-name
	item_name_ = to_upper(name);
	value_ = "";
	return false;
}

