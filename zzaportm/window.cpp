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
#include <FL/Fl_Preferences.H>

using namespace zzaportm;

extern Fl_Preferences* settings_;

window::window(int W, int H, const char* label) :
	Fl_Window(W, H, label)
	, port_name_("")
	, baud_rate_()
	, data_("")
	, format_(HEX)
	, rx_timeout_(1000)
	, num_requests_(5)
{
	// Get parameter settings
	Fl_Preferences params_settings(settings_, "Parameters");
	char* temp;
	params_settings.get("Port Name", temp, "");
	port_name_ = temp;
	free(temp);
	params_settings.get("Baud Rate", (signed int&)baud_rate_, 9600);
	params_settings.get("Test Data", temp, "");
	data_ = temp;
	free(temp);
	params_settings.get("Read Timeout", (signed int&) rx_timeout_, 1000);
	params_settings.get("Requests", (signed int&)num_requests_, DEF_REQUESTS);
	params_settings.get("Format", (int&)format_, (int)ASCII);
	params_settings.get("Mode", (int&)mode_, (int)TX_RX); 
	// Create the port - but don't try and connect yet
	port_ = new serial;
	// Add all the widgets
	create_form();
	show();
}

window::~window() {
	// Save parameter settings
	Fl_Preferences params_settings(settings_, "Parameters");
	params_settings.set("Port Name", port_name_.c_str());
	params_settings.set("Baud Rate", (signed int)baud_rate_);
	params_settings.set("Test Data", data_.c_str());
	params_settings.set("Read Timeout", (signed int)rx_timeout_);
	params_settings.set("Requests", (signed int)num_requests_);
	params_settings.set("Format", (int)format_);
	params_settings.set("Mode", (int)mode_);
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
	ctr_requests_ = new Fl_Counter(curr_x, curr_y, WSMEDIT, HTEXT, "Requests");
	ctr_requests_->labelfont(FONT);
	ctr_requests_->type(FL_SIMPLE_COUNTER);
	ctr_requests_->labelsize(FONT_SIZE);
	ctr_requests_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ctr_requests_->textfont(FONT);
	ctr_requests_->textsize(FONT_SIZE);
	ctr_requests_->value(num_requests_);
	ctr_requests_->bounds(MIN_REQUESTS, MAX_REQUESTS);
	ctr_requests_->step(STEP_REQUESTS);
	ctr_requests_->callback(cb_sp_number, &num_requests_);
	ctr_requests_->tooltip("Select the number of requests to send (0 means until Stop pressed)");
	if (mode_ == RX_ONLY) {
		ctr_requests_->deactivate();
	}
	curr_x += ctr_requests_->w() + GAP;

	// GO Button
	Fl_Button* bn_go = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "GO!");
	bn_go->labelfont(FONT);
	bn_go->labelsize(FONT_SIZE);
	bn_go->align(FL_ALIGN_INSIDE);
	bn_go->color(FL_GREEN, fl_lighter(FL_RED));
	bn_go->callback(cb_bn_go);
	bn_go->tooltip("Select to start transfers");
	curr_x += bn_go->w() + GAP;

	// Stop button
	bn_stop_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Stop");
	bn_stop_->labelfont(FONT);
	bn_stop_->labelsize(FONT_SIZE);
	bn_stop_->align(FL_ALIGN_CENTER);
	bn_stop_->color(fl_lighter(FL_RED));
	bn_stop_->callback(cb_bn_stop, &stopped_);
	bn_stop_->tooltip("Select to stop transfers");
	bn_stop_->deactivate();

	int w_win = curr_x + bn_stop_->w() + EDGE;

	curr_x = EDGE;
	curr_y += bn_stop_->h() + HTEXT;

	// Hex radio button
	Fl_Button* bn_format = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON);
	bn_format->type(FL_TOGGLE_BUTTON);
	bn_format->labelfont(FONT);
	bn_format->labelsize(FONT_SIZE);
	bn_format->align(FL_ALIGN_CENTER);
	bn_format->down_box(bn_format->box());
	bn_format->value(format_ == ASCII ? 1 : 0);
	if (bn_format->value()) {
		bn_format->label("ASCII");
	}
	else {
		bn_format->label("Hex");
	}
	bn_format->callback(cb_bn_format, (void*)&format_);
	bn_format->tooltip("Format for the data (Hex or ASCII)");
	curr_x += bn_format->w() + GAP;

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

	// Mode button
	Fl_Button* bn_mode = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON);
	bn_mode->type(FL_TOGGLE_BUTTON);
	bn_mode->labelfont(FONT);
	bn_mode->labelsize(FONT_SIZE);
	bn_mode->align(FL_ALIGN_CENTER);
	bn_mode->down_box(bn_mode->box());
	bn_mode->value(mode_ == RX_ONLY ? 1 : 0);
	if (bn_mode->value()) {
		bn_mode->label("Monitor");
	}
	else {
		bn_mode->label("TX/RX");
	}
	bn_mode->callback(cb_bn_mode, (void*)&mode_);
	bn_mode->tooltip("Mode for the test (TX/RX or RX only)");
	curr_x  = max (curr_x + bn_mode->w() + GAP, bn_go->x());

	// Clear button
	bn_clear_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Clear");
	bn_clear_->labelfont(FONT);
	bn_clear_->labelsize(FONT_SIZE);
	bn_clear_->align(FL_ALIGN_CENTER);
	bn_clear_->color(fl_lighter(FL_RED));
	bn_clear_->callback(cb_bn_clear);
	bn_clear_->tooltip("Select to clear text display");
	bn_clear_->deactivate();

	// This last widget defines the overall width of the window
	w_win = max(w_win, curr_x + bn_clear_->w() + EDGE);
	const unsigned int w_text = w_win - EDGE - EDGE;

	// Draw the txt display
	curr_y += HTEXT + bn_clear_->h();
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
	if (mode_ == TX_RX) {
		display_data(TX, data);
		if (!port_->write_buffer(data)) {
			display_data(TX_ERROR, string("Write failed"));
		}
		redraw();
		Fl::wait();
	}
	string response;
	if (!port_->read_buffer(response)) {
		display_data(RX_ERROR, string("Read failed"));
	}
	else if (response.length()) {
		display_data(RX, response);
	}
	redraw();
	Fl::wait();
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
	bn_clear_->activate();
	redraw();
}

// Populate the port choice
void window::populate_port_choice(Fl_Choice* ch) {
	ch->clear();
	string* ports = new string[1];
	int num_ports;
	ch->value(0);
	if (!port_->available_ports(1, ports, true, num_ports)) {
		delete[] ports;
		ports = new string[num_ports];
		port_->available_ports(num_ports, ports, true, num_ports);
	}
	for (int i = 0; i < num_ports; i++) {
		ch->add(ports[i].c_str());
		if (port_name_ == ports[i]) {
			ch->value(i);
		}
	}
	if (ch->value() == 0) {
		port_name_ = ports[0];
	}
}

// Populate the baud choice
void window::populate_baud_choice(Fl_Choice* ch) {
	ch->clear();
	// Default baud-rates
	const int baud_rates[] = { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800 };
	ch->value(0);
	int num_rates = sizeof(baud_rates) / sizeof(int);
	// For all possible rates
	for (int i = 0; i < num_rates; i++) {
		// Convert to text and add it. Setting value if it matches desired
		char value[10];
		sprintf(value, "%d", baud_rates[i]);
		ch->add(value);
		if (baud_rate_ == baud_rates[i]) {
			ch->value(i);
		}
	}
	if (ch->value() == 0) {
		baud_rate_ = baud_rates[0];
	}
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
	bool waiting_stop = false;
	that->stopped_ = false;
	if (that->mode_ == RX_ONLY || that->num_requests_ == 0) {
		that->bn_stop_->activate();
		that->stopped_ = false;
		waiting_stop = true;
	}
	if (that->port_->open_port(that->port_name_, that->baud_rate_, (that->mode_ == RX_ONLY), that->rx_timeout_)) {
		string data = (that->format_ == HEX) ? to_ascii(that->data_) : that->data_;
		unsigned int i = 0;
		while (waiting_stop ? !that->stopped_ : i < that->num_requests_) {
			that->send_request(data);
			i++;
		}
		that->port_->close_port();
	}
	else {
		that->display_data(OPEN_ERROR, "Cannot open port " + that->port_name_);
	}

	that->bn_stop_->deactivate();
}

// Stop button - v is 
void window::cb_bn_stop(Fl_Widget* w, void* v) {
	*(bool*)v = true;
}

// Format button
void window::cb_bn_format(Fl_Widget* w, void* v) {
	cb_value<Fl_Button, int>(w, v);
	Fl_Button* bn = (Fl_Button*)w;
	if (bn->value()) {
		bn->label("ASCII");
	}
	else {
		bn->label("Hex");
	}
}

// Mode button
void window::cb_bn_mode(Fl_Widget* w, void* v) {
	window* that = ancestor_view<window>(w);
	cb_value<Fl_Button, int>(w, v);
	Fl_Button* bn = (Fl_Button*)w;
	if (bn->value()) {
		bn->label("Monitor");
		that->ctr_requests_->deactivate();
	}
	else {
		bn->label("TX/RX");
		that->ctr_requests_->activate();
	}
}

// Clear button
void window::cb_bn_clear(Fl_Widget* w, void* v) {
	window* that = ancestor_view<window>(w);
	that->buffer_->remove(0, that->buffer_->length());
	that->style_buffer_->remove(0, that->style_buffer_->length());
	w->deactivate();
	that->redraw();
	Fl::wait();
}
