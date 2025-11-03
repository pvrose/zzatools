#pragma once

#include <string>



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
	std::string email_url_;
	//! user account
	std::string email_user_;
	//! password
	std::string email_password_;
	//! To address
	std::string to_address_;
	//! Cc address
	std::string cc_address_;
	//! Subject
	std::string subject_;
	//! QSL card attachment - filename.
	std::string qsl_filename_;
	//! e-mail text body
	std::string text_body_;

};

