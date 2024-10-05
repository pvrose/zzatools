#pragma once

#include <string>

using namespace std;

class record;

// This class generates an e-mail to send a QSL imgae to a QSO partner
class qsl_emailer
{
public:

	qsl_emailer();
	~qsl_emailer();

	bool generate_email(record* qso);

	bool send_email();

protected:

	// Read the default email settings
	void load_values();

	record* qso_;

	// e-mail server
	string email_url_;
	// user account
	string email_user_;
	// password
	string email_password_;
	// To address
	string to_address_;
	// Subject
	string subject_;
	// QSL card attachment
	string qsl_filename_;
	// e-mail text body
	string text_body_;

};

