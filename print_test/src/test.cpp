#include <FL/Fl_Printer.H>
#include <FL/Fl_Group.H>

int main(int argc, char** argv) {
    Fl_Printer* prn = new Fl_Printer();
    Fl_Group* grp = new Fl_Group(0, 0, 100, 100, "Group");
    grp->color(FL_GRAY);
    grp->show();
    char label[100];

    prn->begin_job();
    for(int i = 0; i < 3; i++) {
        prn->begin_page();
        prn->print_widget(grp);
        prn->end_page();
    }
    prn->end_job();

    delete prn;
    delete grp;
}