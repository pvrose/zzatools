#include "qsl_emailer.h"
#include "record.h"
#include "png_writer.h"
#include "status.h"
#include "url_handler.h"

#include <string>
#include <cstdio>

#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>

using namespace std;

extern Fl_Preferences* settings_;
extern status* status_;
extern url_handler* url_handler_;

const string SUBJECT = "QSL for <CALL> <QSO_DATE> <BAND> <MODE> from <STATION_CALLSIGN>";
const string BODY = 
"Dear <NAME>,\n\n"
"Confirming our <BAND> <MODE> contact on <QSO_DATE> <TIME_ON>Z\n"
"Please find attached a QSL card image for our QSO.\n\n"
"I hope to see you further down the log.\n"
"73 <MY_NAME> <STATION_CALLSIGN>\n";

qsl_emailer::qsl_emailer() {
	load_values();
}

qsl_emailer::~qsl_emailer() {
}

void qsl_emailer::load_values() {
	Fl_Preferences email_settings(settings_, "e-Mail");
	char* temp;
	// e-Mail settings
	email_settings.get("Server", temp, "");
	email_url_ = temp;
	free(temp);
	email_settings.get("Account", temp, "");
	email_user_ = temp;
	free(temp);
	email_settings.get("Password", temp, "");
	email_password_ = temp;
	free(temp);
}

bool qsl_emailer::generate_email(record* qso) {
	char msg[128];
	// Copy the QSO as I want to make unsaved changes
	qso_ = new record(*qso);
	// Firstly look in record for e-mail address
	to_address_ = qso_->item("EMAIL");
	if (to_address_.length() == 0 || to_address_.find('@') == string::npos) {
		to_address_ = fl_input("Invalid or no e-mail address recorded in log for %s, please enter",
			"", qso_->item("CALL").c_str());
	}
	// Set record name to "Op" if it's not in the record
	if (qso_->item("NAME").length()) {
		qso_->item("NAME", "Op");
	}
	subject_ = qso_->item_merge(SUBJECT, true);
	text_body_ = qso_->item_merge(BODY, true);
	qsl_filename_ = png_writer::png_filename(qso_);
	// Test the file
	FILE* f = fopen(qsl_filename_.c_str(), "rb");
	if (f) {
		fclose(f);
	}
	else {
		snprintf(msg, sizeof(msg), "QSL: PNG file %s has not yet been generated - not sending QSL", qsl_filename_.c_str());
		status_->misc_status(ST_WARNING, msg);
		delete qso_;
		return false;
	}
	snprintf(msg, sizeof(msg), "QSL: Generated e-mail parameters for QSO %s %s %s %s %s",
		qso_->item("CALL").c_str(),
		qso_->item("BAND").c_str(),
		qso_->item("MODE").c_str(),
		qso_->item("QSO_DATE").c_str(),
		qso_->item("TIME_ON").c_str());
	status_->misc_status(ST_OK, msg);
	delete qso_;
	return true;
}

// Send the e-mail
bool qsl_emailer::send_email() {
	if (url_handler_->send_email(
		email_url_,
		email_user_,
		email_password_,
		{ to_address_ },
		{ },
		{ },
		subject_,
		text_body_,
		//{ qsl_filename_ })) {
		{ })) {
		status_->misc_status(ST_OK, "QSL: e-Mail successfully sent");
		return true;
	}
	else {
		status_->misc_status(ST_ERROR, "QSL: e-Mail attempt not successful");
		return false;
	}
}