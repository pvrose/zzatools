#include "alarm_dial.h"

#include <cmath>

#include <algorithm>

#include <FL/fl_draw.H>
#include <FL/math.h>

using namespace zzalog;

alarm_dial::alarm_dial(int X, int Y, int W, int H, const char* l) :
    Fl_Line_Dial(X, Y, W, H, l)
    , alarm1_(nan(""))
    , alarm2_(nan(""))
    , alarm_color_(FL_RED)
    , alarm_locked_(false)
    , grabbed_alarm_(nullptr)
{ }

alarm_dial::~alarm_dial() {}

// from Fl_Dial::draw(X. Y. W. H)
void alarm_dial::draw(int X, int Y, int W, int H) {
    // Draw the main dial and value hand
    Fl_Line_Dial::draw(X, Y, W, H);
    // Now draw the alarm hands - algorith copied from Fl_Dial
    X += Fl::box_dx(box());
    Y += Fl::box_dy(box());
    W -= Fl::box_dw(box());
    H -= Fl::box_dh(box());
    double phi_min = angle1();
    double phi_max = angle2();

    double phi_value = angle_of(Fl_Line_Dial::value());
    double phi_1 = isnan(alarm1_) ? nan("") : angle_of(alarm1_);
    double phi_2 = isnan(alarm2_) ? nan("") : angle_of(alarm2_);

    // Prep the drawing according to size of area
    fl_push_matrix();
    fl_translate(X + W / 2 - .5, Y + H / 2 - .5);
    fl_scale(W - 1, H - 1);

    draw_ticks();

    if (!isnan(phi_1)) draw_alarm(phi_1);
    if (!isnan(phi_2)) draw_alarm(phi_2);

    fl_pop_matrix();

    draw_numbers(X, Y, W, H);

}

void alarm_dial::draw() {
    draw(x(), y(), w(), h());
}

// from Fl_Dial::handle(e, X, Y, W, H)
int alarm_dial::handle(int event, int X, int Y, int W, int H) {
    if (alarm_locked_) {
        return Fl_Line_Dial::handle(event, X, Y, W, H);
    }
    switch (event) {
    case FL_PUSH: {
        handle_push();
        // Get mouse coords
        int mx = (Fl::event_x() - X - W / 2) * H;
        int my = (Fl::event_y() - Y - H / 2) * W;
        // Clicked the centre
        if (!mx && !my) return 1;

        // Now calculate the angle
        double phi_click = 270 - atan2((float)-my, (float)mx) * 180 / M_PI;
        while (phi_click < 0.0) phi_click += 360.0;
        while (phi_click > 360.0) phi_click -= 360.0;
        double phi_value = angle_of(Fl_Line_Dial::value());
        double phi_1 = angle_of(alarm1_);
        double phi_2 = angle_of(alarm2_);

        double new_alarm;
        bool changed_alarm = false;

        new_alarm = value_of(phi_click);

        // is it closer to phi1 than either phi2 or the value
        if (abs(phi_1 - phi_click) <= abs(phi_value - phi_click) &&
            (isnan(phi_2) || (abs(phi_1 - phi_click) <= abs(phi_2 - phi_click)))) {
            grabbed_alarm_ = &alarm1_;
            alarm1_ = new_alarm;
            changed_alarm = true;
        }
        // or closer to phi2 than either phi1 or the value
        // TODO: I can't convince myself I don't need to check between phi1 and phi2 again
        else if (abs(phi_2 - phi_click) <= abs(phi_value - phi_click) &&
            (isnan(phi_1) || (abs(phi_2 - phi_click) <= abs(phi_1 - phi_click)))) {
            grabbed_alarm_ = &alarm2_;
            alarm2_ = new_alarm;
            changed_alarm = true;
        }
        else {
            grabbed_alarm_ = nullptr;
        }
        // If we have changed an alarm value do the callback
        if (changed_alarm) {
            redraw();
            if (when() & FL_WHEN_CHANGED) do_callback();
        }
        else {
            return Fl_Line_Dial::handle(event, X, Y, W, H);
        }
        return 1;
    }
    case FL_DRAG: {
        if (grabbed_alarm_ == nullptr) {
            return Fl_Line_Dial::handle(event, X, Y, W, H);
        }
        // Get mouse coords
        int mx = (Fl::event_x() - X - W / 2) * H;
        int my = (Fl::event_y() - Y - H / 2) * W;
        // Clicked the centre
        if (!mx && !my) return 1;

        // Now calculate the angle
        double phi_click = 270 - atan2((float)-my, (float)mx) * 180 / M_PI;
        while (phi_click < 0.0) phi_click += 360.0;
        while (phi_click > 360.0) phi_click -= 360.0;
        double phi_value = angle_of(Fl_Line_Dial::value());
        double phi_1 = angle_of(alarm1_);
        double phi_2 = angle_of(alarm2_);

        double new_alarm;
        bool changed_alarm = false;

        new_alarm = value_of(phi_click);

         if (grabbed_alarm_) {
            *grabbed_alarm_ = new_alarm;
            return 1;
        }
        else {
            return Fl_Line_Dial::handle(event, X, Y, W, H);
        }
    }
    case FL_RELEASE:
        if (grabbed_alarm_) {
            return 1;
        }
        else {
            return Fl_Line_Dial::handle(event, X, Y, W, H);
        }
        break;
    default:
        return Fl_Line_Dial::handle(event, X, Y, W, H);
    }
}

// Convert angle to value
double alarm_dial::value_of(double angle) {
    double new_alarm;
    // Is it outwith minimum and maximum
    if ((angle1() < angle2() && angle < angle1()) ||
        (angle1() >= angle2() && angle > angle2())) {
        new_alarm = minimum();
    }
    else if ((angle1() < angle2() && angle < angle1()) ||
        (angle1() >= angle2() && angle > angle2())) {
        new_alarm = maximum();
    }
    // else interpolate new value
    else {
        new_alarm = minimum() +
            ((maximum() - minimum()) *
                (angle - angle1()) /
                (angle2() - angle1()));
        if (new_alarm > maximum() - step()) {
            new_alarm = maximum() - step();
        }
        else if (new_alarm < minimum() + step()) {
            new_alarm = minimum() + step();
        }
        else if (grabbed_alarm_ == &alarm1_ && new_alarm >= alarm2_) {
            new_alarm = alarm2_ - step();
        }
        else  if (grabbed_alarm_ == &alarm2_ && new_alarm <= alarm1_) {
            new_alarm = alarm1_ + step();
        }
        new_alarm = round(new_alarm);
    }
    return new_alarm;
}

// Inheritable method
int alarm_dial::handle(int event) {
    return handle(event, x(), y(), w(), h());
}

// From Fl_valuator::value(d) - call changed callback if it has changed
int alarm_dial::value(double v) {
    int result = Fl_Line_Dial::value(v);
    if (changed() && (when() & FL_WHEN_CHANGED)) do_callback();
    return result;
}

// widget specific
void alarm_dial::alarms(double alarm1, double alarm2) {
    alarm1_ = alarm1;
    alarm2_ = alarm2;
}

double alarm_dial::alarm1() { return alarm1_; }

double alarm_dial::alarm2() { return alarm2_; }

void alarm_dial::draw_alarm(double phi) {
    fl_push_matrix();
    // Orientate according to specific hand
    fl_rotate(45 - phi);
    if (active_r()) fl_color(alarm_color());
    else fl_color(fl_inactive(alarm_color()));
    fl_begin_loop();
    {
        fl_vertex(-0.00, 0.0);
        fl_vertex(-0.35, 0.35);
        fl_vertex(0.0, 0.00);
    }
    fl_end_loop();
    fl_pop_matrix();
}

void alarm_dial::draw_ticks() {
    if (active_r()) fl_color(FL_BLACK);
    else fl_color(fl_inactive(FL_BLACK));
    for (double d = minimum(); d <= maximum(); d += step() * 10.0) {
        fl_push_matrix();
        fl_rotate(std::min(angle1(), angle2()) - angle_of(d));
        fl_begin_loop();
        {
            fl_vertex(-0.3, 0.3); // arbitrary just inside the below
            fl_vertex(-0.35, 0.35); // radius(0.5) * sqrt(0.5)
        }
        fl_end_loop();
        fl_pop_matrix();
    }
}

void alarm_dial::draw_numbers(int X, int Y, int W, int H) {
    double ox = X + W / 2 - 0.5;
    double oy = Y + H / 2 - 0.5;
    if (active_r()) fl_color(FL_BLACK);
    else fl_color(fl_inactive(FL_BLACK));
    for (double d = minimum(); d <= maximum(); d += step() * 10.0) {
        double angle = angle_of(d);
        double rangle = angle * M_PI / 180;
        char text[5];
        snprintf(text, 5, "%g", d);
        int tx;
        int ty;
        fl_measure(text, tx, ty);
        double dx = (-sin(rangle) * (double)(W * 0.3 + tx /2));
        double dy = (cos(rangle) * (double)(H * 0.3 + ty/2));
        
        fl_draw((int)(90-angle), text, (int)(ox + dx), (int)(oy + dy));
    }
}

void alarm_dial::alarm_color(Fl_Color color) {
    alarm_color_ = color;
}

Fl_Color alarm_dial::alarm_color() { return alarm_color_; }

double alarm_dial::angle_of(double v) {
    return ((angle2() - angle1())* 
        (v - minimum()) / 
        (maximum() - minimum())) + angle1();
}

void alarm_dial::lock() { alarm_locked_ = true; }

void alarm_dial::unlock() { alarm_locked_ = false; }