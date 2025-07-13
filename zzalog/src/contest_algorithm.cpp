#include "contest_algorithm.h"

#include "contest_scorer.h"
#include "qso_manager.h"
#include "record.h"
#include "spec_data.h"
#include "stn_data.h"

#include "contests/basic.h"
#include "contests/iaru_hf.h"

#include <FL/Fl_Output.H>

typedef vector<string> field_list;

extern qso_manager* qso_manager_;
extern spec_data* spec_data_;
extern stn_data* stn_data_;

map< string, contest_algorithm* >* algorithms_ = nullptr;

// Constructor - should be overriden by the algoritmic specific
contest_algorithm::contest_algorithm() {
	my_info_ = nullptr;
	scorer_ = nullptr;
	rx_items_.clear();
	tx_items_.clear();

}


contest_algorithm::~contest_algorithm() {
	rx_items_.clear();
	tx_items_.clear();
}

void contest_algorithm::attach(contest_scorer* cs) {
	scorer_ = cs;
	int ix = 0;
	for (auto it : rx_items_) {
		scorer_->w_rx_items_[ix]->copy_label(it.c_str());
		ix++;
	}
	for (; ix < NUMBER_FIELDS; ix++) {
		scorer_->w_rx_items_[ix]->hide();
	}
	ix = 0;
	for (auto it : tx_items_) {
		scorer_->w_tx_items_[ix]->copy_label(it.c_str());
		ix++;
	}
	for (; ix < NUMBER_FIELDS; ix++) {
		scorer_->w_tx_items_[ix]->hide();
	}
	my_info_ = stn_data_->get_qth(qso_manager_->get_default(qso_manager::QTH));
}

void contest_algorithm::set_default_rst(record* qso) {
	string contest_mode = spec_data_->dxcc_mode(qso->item("MODE"));
	if (contest_mode == "CW" || contest_mode == "DATA") {
		// CW/Data
		qso->item("RST_SENT", string("599"));
	}
	else {
		// Phone
		qso->item("RST_SENT", string("59"));
	}

}

// Update scorer from QSO
void contest_algorithm::update_rx_items(record* qso) {
	int ix = 0;
	for (auto it : rx_items_) {
		scorer_->w_rx_items_[ix]->value(qso->item(it).c_str());
		ix++;
	}
}

// Update scorer from QSO
void contest_algorithm::update_tx_items(record* qso) {
	int ix = 0;
	for (auto it : tx_items_) {
		scorer_->w_tx_items_[ix]->value(qso->item(it).c_str());
		ix++;
	}
}

// Return all fields used in algorithm
field_list contest_algorithm::fields() {
	vector<string> result;
	for (auto it : rx_items_) {
		result.push_back(it);
	}
	for (auto it : tx_items_) {
		result.push_back(it);
	}
	return result;
}
