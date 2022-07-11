#ifndef __QSL_HTML_VIEW__
#define __QSL_HTML_VIEW__

#include "record.h"

#include <FL/Fl_Help_View.H>

namespace zzalog {

	class qsl_html_view : public Fl_Help_View
	{
	public:
		qsl_html_view(int X, int Y, int W, int H, const char* l = nullptr);
		void value(const char* val, record** records, int num_records);

	};

}

#endif