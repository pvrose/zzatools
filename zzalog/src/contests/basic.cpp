#include "contests/basic.h"

#include "contest_algorithm.h"
#include "contest_scorer.h"
#include "record.h"
#include "stn_data.h"

#include "utils.h"

extern std::map<std::string, contest_algorithm*>* algorithms_;

contest_algorithm* basic_ = new contests::basic;

// Constructor - add algorithmic specific data here
contests::basic::basic() : contest_algorithm() {
	// Add the QSO fields used in scoring and exchanges (RX)
	rx_items_ = { "RST_RCVD", "SRX" };
	// Add the QSO fields used in scoring and exchanges (TX)
	tx_items_ = { "RST_SENT", "STX" };
	// Add this algorithm to the std::list of algorithms - 
	if (algorithms_ == nullptr) algorithms_ = new std::map<std::string, contest_algorithm*>;
	(*algorithms_)["Basic"] = this;

}

// Algorithm specific method to split text into a number of fields
void contests::basic::parse_exchange(record* qso, std::string text) {
	std::vector<std::string> words;
	split_line(text, words, ' ');
	int ix = 0;
	for (auto it : rx_items_) {
		qso->item(it, words[ix]);
		ix++;
	}
}

// Algorithm specific method to generate text from a number of fieds
std::string contests::basic::generate_exchange(record* qso) {
	std::string result = "";
	int ix = 0;
	set_default_rst(qso);
	qso->item("MY_DXCC", my_info_->data.at(DXCC_ID));
	for (auto it : tx_items_) {
		result += qso->item(it);
		ix++;
		if (ix < tx_items_.size()) result += ' ';
	}
	return result;
}

// Algorithm specific method to score an individual QSO
score_result contests::basic::score_qso(record* qso, std::set<std::string>& multipliers) {
	// Multiplier is number of DXCCs worked on each band
	std::string multiplier = qso->item("DXCC") + " " + qso->item("BAND");
	score_result result;
	if (multipliers.find(multiplier) == multipliers.end()) {
		result.multiplier = 1;
		multipliers.insert(multiplier);
	}
	// QSO points - 1 per QSO in different DXCC
	if (qso->item("DXCC") != my_info_->data.at(DXCC_ID)) {
		result.qso_points = 1;
	}
	return result;
}
