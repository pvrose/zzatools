#include "qsl_html_view.h"

using namespace zzalog;

qsl_html_view::qsl_html_view(int X, int Y, int W, int H, const char* l) :
	Fl_Help_View(X, Y, W, H, l)
{

}

void qsl_html_view::value(const char* val, record** records, int num_records) {
	// Creat a new string 3x the size to contain processed HTML
	int val_length = strlen(val);
	char* dval = new char[val_length * 3];

	// Pointers to current processing position
	// Source string
	const char* spos = val;
	// Destination string
	char* dpos = dval;
	// Repeat position in source string
	const char* rpos = spos;
	// Temporary pointer 
	const char* tpos;

	int cur_record = 0;
	bool not_done = true;
	bool increment_record = false;
	string field_name;
	string field_val;

	while (not_done) {
		switch (*spos) {
		case '\0':
			// End of data
			not_done = false;
			*dpos = *spos;
			break;
		case '{':
			// Start of field name - find the end
			tpos = strchr(spos, '}');
			if (tpos == nullptr) {
				/* TODO Error */
			} else {
				// Copy field value to destination
				// move source and destination postitions accordingly
				field_name = string(spos + 1, (tpos - spos - 2));
				field_val = records[cur_record]->item(field_name, true, true);
				strcpy(dpos, field_val.c_str());
				dpos += field_val.length();
				spos = tpos + 1;
				// Individual QSO record
				if (field_name == "QSO_DATE") {
					increment_record = true;
				}
			}
			break;
		case '<':
			if (strncmp(spos, "TR", 2) == 0) {
				// Is it <TR....? - table row start - remember source position
				rpos = spos;
			}
			else if (strncmp(spos, "/TR", 3) == 0) {
				// Is it </TR.. = table row end - if we are processing QSO data....
				if (increment_record) {
					if (cur_record == num_records) {
						cur_record = 0;
						increment_record = false;
					}
					else {
						// Incement record number, reset source to beginning of row
						cur_record++;
						increment_record = false;
						spos = rpos;
					}
				}
			}
			// Copy source to destination and step both
			*dpos = *spos;
			spos++;
			dpos++;
			break;
		default:
			// Copy source to destination and step both
			*dpos = *spos;
			spos++;
			dpos++;
			break;
		}
	}
	// TODO - 
	// copy destination to help_view 
}