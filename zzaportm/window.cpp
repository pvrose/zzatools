#include "window.h"
#include "../zzalib/drawing.h"
#include "../zzalib/serial.h"
#include "../zzalib/callback.h"
#include "../zzalib/utils.h"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Button.H>

using namespace zzaportm;

window::window(int W, int H, const char* label) :
	Fl_Window(W, H, label)
	, port_name_("")
	, baud_rate_()
	, data_("")
	, format_(HEX)
	, rx_timeout_(1000)
	, num_requests_(5)
{
	port_ = new serial;
	create_form();
	show();
}

window::~window() {
}

void window::create_form() {
	begin();
	int curr_x = EDGE;
	int curr_y = EDGE + HTEXT;

	// Port selection
	Fl_Choice* ch_port = new Fl_Choice(curr_x, curr_y, WSMEDIT, HTEXT, "Select Port");
	ch_port->labelfont(FONT);
	ch_port->labelsize(FONT_SIZE);
	ch_port->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	populate_port_choice(ch_port);
	ch_port->callback(cb_choice_text, &port_name_);
	ch_port->textfont(FONT);
	ch_port->textsize(FONT_SIZE);
	ch_port->tooltip("Select port to test");
	curr_x += ch_port->w() + GAP;

	// Baud-rate choice
	Fl_Choice* ch_baud = new Fl_Choice(curr_x, curr_y, WSMEDIT, HTEXT, "Baud-rate");
	ch_baud->labelfont(FONT);
	ch_baud->labelsize(FONT_SIZE);
	ch_baud->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	populate_baud_choice(ch_baud);
	ch_baud->callback(cb_ch_baud, &baud_rate_);
	ch_baud->textfont(FONT);
	ch_baud->textsize(FONT_SIZE);
	ch_baud->tooltip("Select baud rate to use");
	curr_x += ch_baud->w() + GAP;

	// Data pattern input
	Fl_Input* ip_text = new Fl_Input(curr_x, curr_y, WEDIT, HTEXT, "Text to send");
	ip_text->labelfont(FONT);
	ip_text->labelsize(FONT_SIZE);
	ip_text->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_text->value("");
	ip_text->callback(cb_value<Fl_Input, string>, &data_);
	ip_text->textfont(FONT);
	ip_text->textsize(FONT_SIZE);
	ip_text->tooltip("Type data to send");
	curr_x += ip_text->w() + GAP;

	// Hex radio button
	Fl_Radio_Round_Button* bn_hex = new Fl_Radio_Round_Button(curr_x, curr_y, WRADIO, HRADIO, "Hex");
	bn_hex->labelfont(FONT);
	bn_hex->labelsize(FONT_SIZE);
	bn_hex->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn_hex->value(format_ == HEX ? 1 : 0);
	parameters_[(int)HEX] = { HEX, (int*)&format_ };
	bn_hex->callback(cb_radio, &parameters_[(int)HEX]);
	bn_hex->tooltip("Data in hexadecimal format");
	curr_x += bn_hex->w() + GAP;

	// ASCII radio button
	Fl_Radio_Round_Button* bn_ascii = new Fl_Radio_Round_Button(curr_x, curr_y, WRADIO, HRADIO, "ASCII");
	bn_ascii->labelfont(FONT);
	bn_ascii->labelsize(FONT_SIZE);
	bn_ascii->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn_ascii->value(format_ == ASCII ? 1 : 0);
	parameters_[(int)ASCII] = { ASCII, (int*)& format_ };
	bn_ascii->callback(cb_radio, &parameters_[(int)ASCII]);
	bn_ascii->tooltip("Data in hexadecimal format");
	curr_x += bn_ascii->w() + GAP;

	// RX Timeout counter
	Fl_Counter* ct_timeout = new Fl_Counter(curr_x, curr_y, WSMEDIT, HTEXT, "Timeout");
	ct_timeout->type(FL_NORMAL_COUNTER);
	ct_timeout->labelfont(FONT);
	ct_timeout->labelsize(FONT_SIZE);
	ct_timeout->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ct_timeout->textfont(FONT);
	ct_timeout->textsize(FONT_SIZE);
	ct_timeout->value(DEF_TIMEOUT);
	ct_timeout->bounds(MIN_TIMEOUT, MAX_TIMEOUT);
	ct_timeout->step(STEPS_TIMEOUT, STEPL_TIEMOUT);
	ct_timeout->callback(cb_sp_timeout, &rx_timeout_);
	ct_timeout->tooltip("Select the receive timeout");
	curr_x += ct_timeout->w() + GAP;

	// RX Timeout spinner
	Fl_Counter* ct_requests = new Fl_Counter(curr_x, curr_y, WSMEDIT, HTEXT, "Requests");
	ct_requests->labelfont(FONT);
	ct_requests->type(FL_SIMPLE_COUNTER);
	ct_requests->labelsize(FONT_SIZE);
	ct_requests->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ct_requests->textfont(FONT);
	ct_requests->textsize(FONT_SIZE);
	ct_requests->value(DEF_REQUESTS);
	ct_requests->bounds(MIN_REQUESTS, MAX_REQUESTS);
	ct_requests->step(STEP_REQUESTS);
	ct_requests->callback(cb_sp_number, &num_requests_);
	ct_requests->tooltip("Select the number of requests to send");
	curr_x += ct_requests->w() + GAP;

	// GO Button
	Fl_Button* bn_go = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "GO!");
	bn_go->labelfont(FONT);
	bn_go->labelsize(FONT_SIZE);
	bn_go->align(FL_ALIGN_INSIDE);
	bn_go->color(FL_GREEN, fl_lighter(FL_RED));
	bn_go->callback(cb_bn_go);
	bn_go->tooltip("Select to start transfers");


	// This last widget defines the overall width of the window
	const unsigned int w_win = curr_x + bn_go->w() + EDGE;
	const unsigned int w_text = w_win - EDGE - EDGE;

	// Draw the txt display
	curr_y += GAP + HTEXT;
	curr_x = EDGE;
	// First create a buffer
	buffer_ = new Fl_Text_Buffer;
	style_buffer_ = new Fl_Text_Buffer;
	// Now create the display
	display_ = new Fl_Text_Display(curr_x, curr_y, w_text, 200, "Comms traffic");
	display_->labelfont(FONT);
	display_->labelsize(FONT_SIZE);
	display_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	display_->textfont(FL_COURIER);
	display_->textsize(12);
	display_->buffer(buffer_);
	display_->highlight_data(style_buffer_, STYLES, sizeof(STYLES) / sizeof(Fl_Text_Display::Style_Table_Entry), 'F', nullptr, nullptr);
	
	// Now resize the window
	const unsigned int h_win = curr_y + display_->h() + EDGE;
	size(w_win, h_win);
	end();
	show();

}

// Make serial request
void window::send_request(string data) {
	display_data(TX, data);
	if (!port_->write_buffer(data)) {
		display_data(TX_ERROR, string("Write failed"));
	}
	string response;
	if (!port_->read_buffer(response)) {
		display_data(RX_ERROR, string("Read failed"));
	}
	else {
		display_data(RX, response);
	}
}

// Display the transmitted/received data
void window::display_data(direction direction, string data) {
	string time = now(true, "%H:%M:%S");
	string line;
	string disp_data = (format_ == HEX) ? to_hex(data) : data;
	char* style_line = nullptr;
	switch (direction) {
	case TX:
		line.resize(13 + disp_data.length());
		style_line = new char[line.length() + 1];
		line = "TX " + time + ' ' + disp_data + '\n';
		memset(style_line, 0, line.length() + 1);
		memset(style_line, 'A', 12);
		memset(style_line + 12, 'B', disp_data.length() + 1);
		break;
	case RX:
		line.resize(13 + disp_data.length());
		style_line = new char[line.length() + 1];
		line = "RX " + time + ' ' + disp_data + '\n';
		memset(style_line, 0, line.length() + 1);
		memset(style_line, 'A', 12);
		memset(style_line + 12, 'C', disp_data.length() + 1);
		break;
	case TX_ERROR:
		line.resize(13 + data.length());
		style_line = new char[line.length() + 1];
		line = "TX " + time + ' ' + data + '\n';
		memset(style_line, 0, line.length() + 1);
		memset(style_line, 'A', 12);
		memset(style_line + 12, 'D', data.length() + 1);
		break;
	case RX_ERROR:
		line.resize(13 + data.length());
		style_line = new char[line.length() + 1];
		line = "RX " + time + ' ' + data + '\n';
		memset(style_line, 0, line.length() + 1);
		memset(style_line, 'A', 12);
		memset(style_line + 12, 'D', data.length() + 1);
		break;
	case OPEN_ERROR:
		line.resize(13 + data.length());
		style_line = new char[line.length() + 1];
		line = "   " + time + ' ' + data + '\n';
		memset(style_line, 0, line.length() + 1);
		memset(style_line, 'A', 12);
		memset(style_line + 12, 'D', data.length() + 1);
		break;
	}
	buffer_->append(line.c_str());
	style_buffer_->append(style_line);
	redraw();
}

// Populate the port choice
void window::populate_port_choice(Fl_Choice* ch) {
	ch->clear();
	string* ports = new string[1];
	int num_ports;
	if (!port_->available_ports(1, ports, true, num_ports)) {
		delete[] ports;
		ports = new string[num_ports];
		port_->available_ports(num_ports, ports, true, num_ports);
	}
	for (int i = 0; i < num_ports; i++) {
		ch->add(ports[i].c_str());
	}
	ch->value(0);
	port_name_ = ports[0];
}

// Populate the baud choice
void window::populate_baud_choice(Fl_Choice* ch) {
	ch->clear();
	// Default baud-rates
	const int baud_rates[] = { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800 };
	int num_rates = sizeof(baud_rates) / sizeof(int);
	int index = 0;
	// For all possible rates
	for (int i = 0; i < num_rates; i++) {
		// capabilities overridden or within the range supported by capabilities
		ch->add(to_string(baud_rates[i]).c_str());
	}
	ch->value(0);
	baud_rate_ = baud_rates[0];
}

// Callbacks

// Baud choice - v will point to the int value
void window::cb_ch_baud(Fl_Widget* w, void* v) {
	string text;
	cb_choice_text(w, &text);
	*(int*)v = stoi(text);
}

// RX timeout counter - v points to int value
void window::cb_sp_timeout(Fl_Widget* w, void* v) {
	double value;
	cb_value<Fl_Counter, double>(w, &value);
	*(int*)v = (int)(value * 1000);
}

// Number tries spinner
void window::cb_sp_number(Fl_Widget* w, void* v) {
	double value;
	cb_value<Fl_Counter, double>(w, &value);
	*(int*)v = (int)value;
}

// Go button
void window::cb_bn_go(Fl_Widget* w, void* v) {
	window* that = ancestor_view<window>(w);
	if (that->port_->open_port(that->port_name_, that->baud_rate_, false, that->rx_timeout_)) {
		string data = (that->format_ == HEX) ? to_ascii(that->data_) : that->data_;
		for (unsigned int i = 0; i < that->num_requests_; i++) {
			that->send_request(data);
		}
		that->port_->close_port();
	}
	else {
		that->display_data(OPEN_ERROR, "Cannot open port " + that->port_name_);
	}
}

