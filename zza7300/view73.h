#ifndef __VIEW73__
#define __VIEW73__

#include <FL/Fl_Table_Row.H>

namespace zza7300 {
	enum view_type {
		VT_MEMORIES,
		VT_PANADAPTER_BANDS,
		VT_USER_BANDS,
		VT_CW_MESSAGES,
		VT_RTTY_MESSAGES
	};

	class view73 : public Fl_Table_Row
	{
	public:
		view73(int X, int Y, int W, int H, const char* label);
		~view73();
		
	public:
		// Get/Set type
		void type(view_type t);
		view_type type();

	protected:
		struct memory {
			char number[4];
			char split;
			char group;
			char rx_freq[10];
			char rx_mode[4];
			char rx_data;
			char rx_tone;
			char rx_rptr_tone[6];
			char rx_rptr_sqlch[6];
			char tx_freq[10];
			char tx_mode[4];
			char tx_data;
			char tx_tone;
			char tx_rptr_tone[6];
			char tx_rptr_sqlch[6];
			char name[10];
		};
		struct pan_band {
			char lower[6];
			char upper[6];
		};
		struct user_band {
			char number[2];
			char lower[10];
			char upper[10];
		};
		struct message {
			char number[2];
			char message[70];
		};
		view_type type_;
		// table rows
		int rows_;
		// table columns
		int columns_;
		// Items
		void* items_;
	};

}

#endif

