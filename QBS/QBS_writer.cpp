#include "QBS_writer.h"

using namespace zzalib;

// Constructor
QBS_writer::QBS_writer()
{
	dir_names_.clear();
	dir_names_[QD_SEND] = "SEND";
	dir_names_[QD_RECEIVE] = "RECEIVE";
	dir_names_[QD_DISPOSE] = "DISPOSE";
	dir_names_[QD_KEEP] = "KEEP";
	dir_names_[QD_INITIAL] = "INITIAL";
	dir_names_[QD_BRF] = "BRF";
}

// Destructor
QBS_writer::~QBS_writer()
{

}

bool QBS_writer::store_data(QBS_data* database, ostream& os) {
	// Get the data
	the_data_ = database;

	printf("Starting XML generation");
	if (write_element(QBS_reader::QBS_NONE)) {
		printf("Ended XML generation - writing to file");
		if (data(os)) {
			printf("Written file");
			return true;
		}
	}

	return false;
}

bool QBS_writer::write_element(QBS_reader::qbs_element_t element) {

	string name;
	string data;
	map<string, string>* attributes;
	bool ok = true;

	switch (element) {
	case QBS_reader::QBS_NONE:
		// Initial prolog <?xml version="1.0" encoding="utf-8" ?>
		name = "xml";
		data = "version=\"1.0\" encoding=\"utf-8\" ";
		ok = process_instr(name, data);
		// Top-level element
		ok &= write_element(QBS_reader::QBS);
		break;
	case QBS_reader::QBS:
		// QBS element
		name = "QBS";
		ok = start_element(name, nullptr);
		ok &= write_element(QBS_reader::QBS_AMATEURS);
		ok &= write_element(QBS_reader::QBS_BATCHES);
		ok &= end_element(name);
		break;
	case QBS_reader::QBS_AMATEURS:
		// QBS_AMATEURS
		name = "AMATEURS";
		ok = start_element(name, nullptr);
		the_data_->set_ham();
		do {
			callsign_ = the_data_->current_ham();
			ok &= write_element(QBS_reader::QBS_AMATEUR);
		} while (the_data_->next_ham());
		break;
	case QBS_reader::QBS_AMATEUR:
		// QBS_AMATEUR
		name = "AMATEUR";
		// Attributes
		attributes = new map<string, string>;
		attributes->clear();
		(*attributes)["CALLSIGN"] = callsign_;
		ok &= start_element(name, attributes);
		{
		QBS_data::ham_info_t* info = the_data_->get_info();
		if (info->rsgb_id > 0) ok &= write_item("RSGB", to_string(info->rsgb_id));
		if (info->info.length()) ok &= write_item("INFO", info->info);
		if (info->name.length()) ok &= write_item("NAME", info->name);
		if (info->e_mail.length()) ok &= write_item("E_MAIL", info->e_mail);
		if (info->postal_address.length()) ok &= write_item("ADDRESS", info->postal_address);
		}
		// Output SASEs and cards
		if (the_data_->has_counts(QI_CARDS)) {
			ok &= write_element(QBS_reader::QBS_CARD);
		}
		if (the_data_->has_counts(QI_SASES)) {
			ok &= write_element(QBS_reader::QBS_SASE);
		}
		ok &= end_element(name);

		break;
	case QBS_reader::QBS_BATCH_VAL:
		// QBS_BATCH_VAL
		name = "BATCH_VAL";
		// Attributes
		attributes = new map<string, string>;
		attributes->clear();
		(*attributes)["NAME"] = batch_id_;
		ok &= start_element(name, attributes);
		for (auto it = the_data_->core_dirs(type_).begin();
			it != the_data_->core_dirs(type_).end(); it++) {
		if (the_data_->get_count(type_, batch_id_, *it) != 0) {
			write_item(dir_names_[*it],
				to_string(the_data_->get_count(type_, batch_id_, *it)));
		}
		break;
	case QBS_reader::QBS_SASE:
		// qBS_SASE
		name = "SASES";
		type_ = QI_SASES;
		// Attributes
		ok &= start_element(name, nullptr);
		ok &= write_batches();
		ok &= end_element(name);
		break;
	case QBS_reader::QBS_CARD:
		// qBS_SASE
		name = "CARDS";
		type_ = QI_CARDS;
		ok &= start_element(name, nullptr);
		ok &= write_batches();
		ok &= end_element(name);
		break;
		break;
	case QBS_reader::QBS_BATCHES:
		// BATCH
		name = "BATCHES";
		ok = start_element(name, nullptr);
		for (auto it_b = the_data_->batch_data_.begin();
			it_b != the_data_->batch_data_.end() && ok; it_b++) {
			b_times_ = &(*it_b).second;
			batch_id_ = (*it_b).first;
			ok &= write_element(QBS_reader::QBS_BATCH_DEF);
		}
		ok &= end_element(name);
		break;
	case QBS_reader::QBS_BATCH_DEF:
		// BATCH_DEF
		name = "BATCH_DEF";
		// attributes
		attributes = new map<string, string>;
		(*attributes)["NAME"] = batch_id_;
		if (b_times_->date_rcvd.length())
			write_item("RECEIVED", b_times_->date_rcvd);
		if (b_times_->date_disposed.length())
			write_item("DISPOSED", b_times_->date_disposed);
		if (!isnan(b_times_->weight_disposed))
			write_item("WEIGHT", to_string(b_times_->weight_disposed));
		ok &= start_element(name, attributes);

		ok &= end_element(name);
		break;
	case QBS_reader::QBS_VALUE:
		// ITEM element
		ok = start_element(item_name_, nullptr);
		ok &= characters(value_);
		ok &= end_element(item_name_);
		break;
	}
	if (!ok) {
		printf("Error wring XML - element %s", name.c_str());
	}
	return ok;
}

	bool QBS_writer::write_item(string name, string value) {
		item_name_ = name;
		value_ = value;
		return write_element(QBS_reader::QBS_VALUE);
	}

	bool QBS_writer::write_batches() {
		bool ok = true;
		string batch = the_data_->first_batch();
		bool done = false;
		while (!done) {
			if (the_data_->has_counts(type_, batch)) {
				batch_id_ = batch;
				ok &= write_element(QBS_reader::QBS_BATCH_VAL);
			}
			if (batch == the_data_->current_batch()) done = true;
			batch = the_data_->next_batch(batch);
		}

		return ok;
	}