#pragma once

#include <string>

using namespace std;

class record;

//! This class generates an e-mail to send a QSL imgae to a QSO partner.
class qsl_emailer
{
public:
	//! Constructor.
	qsl_emailer();
	//! Destructor.
	~qsl_emailer();

	//! Generate the e-mail for \p qso.
	bool generate_email(record* qso);

	//! Send the generated e-mail.
	bool send_email();

protected:

	//! Read the default email settings
	void load_values();
	//! QSO record to be QSL'd by e-mail.
	record* qso_;

	//! e-mail server
	string email_url_;
	//! user account
	string email_user_;
	//! password
	string email_password_;
	//! To address
	string to_address_;
	//! Cc address
	string cc_address_;
	//! Subject
	string subject_;
	//! QSL card attachment - filename.
	string qsl_filename_;
	//! e-mail text body
	string text_body_;

};

