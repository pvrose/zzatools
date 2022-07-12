#include "qsl_html_view.h"

using namespace zzalog;

qsl_html_view::qsl_html_view(int X, int Y, int W, int H, const char* l) :
	Fl_Help_View(X, Y, W, H, l)
{
	box(FL_NO_BOX);
}

void qsl_html_view::value(const char* val, record** records, int num_records) {
	// Creat a new string 3x the size to contain processed HTML
	int val_length = strlen(val);
	char* dval = new char[val_length * 3];
	memset(dval, 0, val_length);

	// Pointers to current processing position
	// Source string
	const char* spos = val;
	// Destination string
	char* dpos = dval;
	// Temporary pointer 
	const char* tpos;

	int cur_record = 0;
	bool not_done = true;
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
				bool multiple_qsos = (*(spos + 1) == '*');
				cur_record = 0;
				if (multiple_qsos) {
					while (cur_record < num_records) {
						field_name = string(spos + 2, (tpos - spos - 2));
						field_val = records[cur_record]->item(field_name, true, true);
						strcpy(dpos, field_val.c_str()); 
						cur_record++;
						if (cur_record < num_records) {
							strcpy(dpos, "<BR>");
							dpos += 4 + field_val.length();
						}
						else {
							dpos += field_val.length();
						}
					}
				} else {
					field_name = string(spos + 1, (tpos - spos - 1));
					field_val = records[cur_record]->item(field_name, true, true);
					strcpy(dpos, field_val.c_str());
					dpos += field_val.length();
				}
				spos = tpos + 1;
			}
			break;
		default:
			// Copy source to destination and step both
			*dpos = *spos;
			spos++;
			dpos++;
			break;
		}
	}
	// copy destination to help_view 
	Fl_Help_View::value(dval);
	show();
}