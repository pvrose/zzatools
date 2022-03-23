#ifndef __ALARM_DIAL__
#define __ALARM_DIAL__

#include <FL/Fl_Line_Dial.H>

namespace zzalog {

    // This widget extends Fl_Line_Dial to add 1 or two further dial
    // hands to trigger a callback (FL_WHEN_CHANGED) if the value
    // crosses either of these hands

    class alarm_dial :
        public Fl_Line_Dial
    {
    public:
        alarm_dial(int X, int Y, int W, int H, const char* l);
        ~alarm_dial();

        // Inheritance
        // from Fl_Dial::draw(X. Y. W. H)
        virtual void draw(int X, int Y, int W, int H);
        virtual void draw();

        // from Fl_Dial::handle(e, X, Y, W, H)
        virtual int handle(int event, int X, int Y, int W, int H);
        virtual int handle(int event);
        // From Fl_valuator::value(d)
        virtual int value(double v);

        // widget specific
        void alarms(double alarm1, double alarm2);

        double alarm1();
        double alarm2();

        void alarm_color(Fl_Color c);
        Fl_Color alarm_color();

        void lock();
        void unlock();


    protected:

        double alarm1_;
        double alarm2_;
        Fl_Color alarm_color_;
        bool alarm_locked_;
        double* grabbed_alarm_;

        double angle_of(double v);
        void draw_alarm(double v);
        void draw_ticks();
        double value_of(double angle);
    };

}
#endif