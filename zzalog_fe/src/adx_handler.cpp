#include "adx_handler.h"

#include "book.h"
#include "main.h"
#include "record.h"
#include "status.h"

// pugixml
#include "pugixml.hpp"

using xml_document = pugi::xml_document;
using xml_node = pugi::xml_node;
using xml_attribute = pugi::xml_attribute;

	//! Constructor
adx_handler::adx_handler() {
	loading_ = false;
	storing_ = false;
}

//! Destructor
adx_handler::~adx_handler() {

}

//! Load data from input stream \a in to book \a book.
bool adx_handler::load_book(book* my_book, std::istream& in) {
	status_->misc_status(ST_NOTE, "LOG: Started loading ADX");
	loading_ = true;
	xml_document doc;
	char msg[128];
	pugi::xml_parse_result result = doc.load(in, pugi::parse_default | pugi::parse_declaration);
	if (result.status != pugi::status_ok) {
		// Parsing the XML failed for some reason
		snprintf(msg, sizeof(msg), "LOG: Loading ADX failed: %s", 
			result.description());
		status_->misc_status(ST_ERROR, msg);
		loading_ = false;
		return false;
	}
	status_->misc_status(ST_OK, "LOG: Loaded ADX OK");
	// Top node is ADX - 
	xml_node top = doc.document_element();
	// Process header
	record* header = new record();
	xml_node n_header = top.child("HEADER");
	if (n_header) {
		if (!load_record(header, n_header)) {
			status_->misc_status(ST_ERROR, "LOG: Loading ADX failed: bad header");
			loading_ = false;
			return false;
		}
		my_book->header(header);
	}
	header->item("APP_ZZA_NUMRECORDS", total_records_);
	if (total_records_ == 0) {
		// None supplied -assume 10K
		total_records_ = 10000;
	}
	num_records_ = 0;
	status_->progress(total_records_, my_book->book_type(), "Parsing ADX", "records");
	xml_node n_records = top.child("RECORDS");
	// Convert each record in turn
	for (auto n_qso : n_records.children("RECORD")) {
		record* qso = new record();
		num_records_++;
		if (!load_record(qso, n_qso)) {
			status_->misc_status(ST_ERROR, "LOG: Load ADX failed: Bad record");
			status_->progress("ADX conversion failed", my_book->book_type());
			loading_ = false;
			return false;
		}
		my_book->insert_record(qso);
		status_->progress(num_records_, my_book->book_type());
	}
	if (num_records_ < total_records_) {
		status_->progress("ADX conversion complete", my_book->book_type());
	}
	loading_ = false;
	return true;
}

//! Generate XML for the records in book and send them to the output stream.

//! \param book the data to be written.
//! \param os output stream.
//! \param clean mark records as clean after writing.
//! \return true if successful, false if not.
bool adx_handler::store_book(book* my_book, std::ostream& os, bool clean) {
	char msg[128];
	status_->misc_status(ST_NOTE, "LOG: Started storing ADX");
	status_->progress(my_book->size(), my_book->book_type(), "Storing ADX", "records");
	num_records_ = 0;
	// Generate the new ADX document
	xml_document doc;
	// Add the declaration
	auto n_decl = doc.append_child(pugi::node_declaration);
	n_decl.append_attribute("version") = "1.0";
	n_decl.append_attribute("encoding") = "UTF-8";

	// Top level node
	auto root = doc.append_child("ADX");

	// Store HEADER
	record* header = my_book->header();
	xml_node n_header = root.append_child("HEADER");
	if (!store_record(header, n_header)) {
		status_->misc_status(ST_ERROR, "LOG: Storing ADX failed: Error in header");
		status_->progress("Storing ADX failed", my_book->book_type());
		return false;
	}
	num_records_++;
	// Mark the header clean
	if (clean) {
		my_book->delete_dirty_record(header);
	}

	// Store RECORDS
	xml_node n_records = root.append_child("RECORDS");
	for (auto qso : *my_book) {
		xml_node n_qso = n_records.append_child("RECORD");
		if (!store_record(qso, n_qso)) {
			snprintf(msg, sizeof(msg),
				"LOG: Storing ADX failed: Error in record %s %s %s",
				qso->item("QSO_DATE").c_str(),
				qso->item("TIME_ON").c_str(),
				qso->item("CALL").c_str());
			status_->misc_status(ST_ERROR, msg);
			status_->progress("Storing ADX failed", my_book->book_type());
			return false;
		}
		// Mark the QSO clean
		if (clean) {
			my_book->delete_dirty_record(qso);
		}
		status_->progress(num_records_, my_book->book_type());
		num_records_++;
	}
	// Save the data
	doc.save(os, "  ");

	// Done!
	status_->misc_status(ST_OK, "LOG: Finished storing ADX");
	return true;
}

//! Load the recotd from the XML node
bool adx_handler::load_record(record* rec, xml_node& node) {
	char msg[128];
	for (auto n_field : node.children()) {
		std::string name = n_field.name();
		std::string value = n_field.text().as_string();
		if (name == "USERDEF") {
			// Ignore USERDEF: both header and record
			snprintf(msg, sizeof(msg), "LOG: USERDEF %s ignored",
				name.c_str());
			status_->misc_status(ST_WARNING, msg);
		}
		else if (name == "APP") {
			// APP
			std::string pid = n_field.attribute("PROGRAMID").as_string();
			std::string field = n_field.attribute("FIELDNAME").as_string();
			if (pid == "EQSL") {
				if (field == "SWL") {
					rec->item("SWL", value);
				}
			}
			else if (pid == "ZZA" || pid == "QRZCOM") {
				std::string alias = "APP_" + pid + "_" + field;
				rec->item(alias, value);
			}
		}
		else {
			rec->item(name, value);
		}
	}
	return true;
}

bool adx_handler::store_record(record* qso, xml_node& node) {
	for (auto field : *qso) {
		// Check if it is ann APP... field
		if (field.first.substr(0, 3) == "APP") {
			size_t pos = 4;
			while (field.first[pos] != '_') pos++;
			std::string pid = field.first.substr(4, pos - 4);
			std::string name = field.first.substr(pos + 1);
			xml_node n_field = node.append_child("APP");
			n_field.append_attribute("PROGRAMID") = pid;
			n_field.append_attribute("FIELDNAME") = name;
			n_field.append_attribute("TYPE") = "S";
			if (!n_field.text().set(field.second)) return false;
		}
		else {
			if(!node.append_child(field.first).text().set(field.second))
				return false;
		}
	}
	return true;
}

bool adx_handler::loading() { return loading_; }
bool adx_handler::storing() { return storing_; }

double adx_handler::progress() {
	return (double)num_records_ / (double(total_records_));
}
