#include "contests/iaru_hf.h"

#include "contest_scorer.h"
#include "qso_manager.h"
#include "record.h"
#include "stn_data.h"

#include "utils.h"

using namespace contests;

extern stn_data* stn_data_;
extern map<string, contest_algorithm*>* algorithms_;

contest_algorithm* iaru_hf_ = new contests::iaru_hf;

// Constructor - add algorithmic specific data here
iaru_hf::iaru_hf() : contest_algorithm() {
	// Add the QSO fields used in scoring and exchanges (RX)
	rx_items_ = { "RST_RCVD", "ITUZ", "CONT"};
	// Add the QSO fields used in scoring and exchanges (TX)
	tx_items_ = { "RST_SENT", "MY_ITU_ZONE", "APP_ZZA_MY_CONT" };
	// Add this algorithm to the list of algorithms
	if (algorithms_ == nullptr) algorithms_ = new map<string, contest_algorithm*>;
	(*algorithms_)["IARU-HF"] = this;
}

// Algorithm specific method to split text into a number of fields
void iaru_hf::parse_exchange(record* qso, string text) {
	vector<string> words;
	split_line(text, words, ' ');
	int ix = 0;
	for (auto it : rx_items_) {
		qso->item(it, words[ix]);
		ix++;
	}
	update_rx_items(qso);
}

// Algorithm specific method to generate text from a number of fieds
string iaru_hf::generate_exchange(record* qso) {
	string result = "";
	int ix = 0;
	set_default_rst(qso);
	qso->item("APP_ZZA_MY_CONT", my_info_->data.at(CONTINENT));
	qso->item("MY_ITU_ZONE", my_info_->data.at(ITU_ZONE));
	for (auto it : tx_items_) {
		result += qso->item(it);
		ix++;
		if (ix < tx_items_.size()) result += ' ';
	}
	update_tx_items(qso);
	return result;
}

// Algorithm specific method to score an individual QSO
// IARU HF 
// 1 pt per QSO in same ITU zone
// 1 pt per QSO with HQ or IARU official (ITUZ != numeric)
// 3 pt per QSO same continent (different ITUZ)
// 5 pt per qSO different continent and different ITUZ
// Multiplier = count(ITUZ x BAND)
// Total = QSO points * multiplier

score_result iaru_hf::score_qso(record* qso, const set<string>& multipliers) {
	// ITU Zone or otherwise
	string ituz = qso->item("ITUZ");
	string cont = qso->item("CONT");
	bool itu_station = !is_integer(ituz);
	score_result res;
	string multiplier = ituz + " " + qso->item("BAND");
	if (multipliers.find(multiplier) == multipliers.end()) {
		res.multiplier = 1;
	}
	// QSO points
	if (ituz == my_info_->data.at(ITU_ZONE)) res.qso_points = 1;
	else if (itu_station) res.qso_points = 1;
	else if (cont == my_info_->data.at(CONTINENT)) res.qso_points = 3;
	else res.qso_points = 5;
	return res;
}
