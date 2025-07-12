#include "contests/basic.h"

#include "contest_algorithm.h"
#include "contest_scorer.h"
#include "record.h"
#include "stn_data.h"

#include "utils.h"

using namespace contests;

basic::basic() : contest_algorithm() {
	rx_items_ = { "RST_RCVD", "SRX" };
	tx_items_ = { "RST_SENT", "STX" };
}

// Algorithm specific method to split text into a number of fields
void basic::parse_exchange(record* qso, string text) {
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
string basic::generate_exchange(record* qso) {
	string result = "";
	int ix = 0;
	set_default_rst(qso);
	qso->item("MY_DXCC", my_info_->data.at(DXCC_ID));
	for (auto it : tx_items_) {
		result += qso->item(it);
		ix++;
		if (ix < tx_items_.size()) result += ' ';
	}
	update_tx_items(qso);
	return result;
}

// Algorithm specific method to score an individual QSO
score_result basic::score_qso(record* qso, const set<string>& multipliers) {
	// Multiplier is number of DXCCs worked on each band
	string multiplier = qso->item("DXCC") + " " + qso->item("BAND");
	score_result result;
	if (multipliers.find(multiplier) == multipliers.end()) {
		result.multiplier = 1;
	}
	// QSO points - 1 per QSO in different DXCC
	if (qso->item("DXCC") != my_info_->data.at(DXCC_ID)) {
		result.qso_points = 1;
	}
	return result;
}
