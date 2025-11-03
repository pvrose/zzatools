#pragma once


#include <string>
#include <set>
#include <vector>
#include <map>

#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>



class record;
class Fl_Output;
typedef size_t qso_num_t;

//! This class displays information about previous QSOs with this contact
class qso_details :
    public Fl_Group
{
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qso_details(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor
    ~qso_details();
    //! Override Fl_Group::handle() taing focus to allow keyboard F1 to open userguide.
    virtual int handle(int event);

    //! Instantiate component widgets.
    void create_form();
    //! Configure component widgets after data change.
    void enable_widgets();

    //! Set the call from \p qso to display whose details.
    void set_qso(record* qso_);

protected:
    //! This class provides a table to show collected details about the contacted station.
    class table_d : public Fl_Table
    {
    public:
        // Constructor.

        //! \param X horizontal position within host window
        //! \param Y vertical position with hosr window
        //! \param W width 
        //! \param H height
        //! \param L label
        table_d(int X, int Y, int W, int H, const char* L = nullptr);
        //! Destructor.
        ~table_d();
        // public methods
        //! inherited from Fl_Table_Row::draw_cell that draws the data in each cell.
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);
        //! Concatenate \p names, \p qths, \p locators and \p states into a single std::list.
        
        //! This is referenced by draw_cell when drawing the table.
        void set_data(std::set<std::string> names, std::set<std::string> qths, std::set<std::string> locators, std::set<std::string> states);
        //! Callback from table that copies the selexted item into the current QSO record.
        static void cb_table(Fl_Widget* w, void* v);


    protected:

        //! Identifiers for the items that are shown in each row.
        enum item_type {
            NAME,        //!< Field "NAME".
            QTH,         //!< Field "QTH".
            LOCATOR,     //!< Field "GRIDSQUARE".
            STATE        //!< Field "STATE".
        };

        //! The contents of each row.
        struct item_details {
            item_type type;    //!< Field identifier.
            std::string value;      //!< Foeld value from previous QSO.
        };

        //! The items to display.
        std::vector<item_details> items_;

        //! The row headers.
        struct item_names {
            const char* heading;    //!< Text displayed.   
            const char* field;      //!< Field name.
        };

        //! Maps the field indentifier to row header details.
        const std::map<item_type, item_names> name_map_ = {
            {NAME, {"NAME ", "NAME"}},
            {QTH, {"QTH ", "QTH"}},
            {LOCATOR, {"GRID ", "GRIDSQUARE" }},
            {STATE, {"STATE", "STATE"}}
        };

    };

    //! THis classs provides a table to display the previous QSOs with this contact
    class table_q : Fl_Table
    {
    public:
        //! Constructor.

        //! \param X horizontal position within host window
        //! \param Y vertical position with hosr window
        //! \param W width 
        //! \param H height
        //! \param L label
        table_q(int X, int Y, int W, int H, const char* L = nullptr);
        //! Destructor.
        ~table_q();
        // public methods
        //! Inherited from Fl_Table_Row::draw_cell draws the contents of the cells.
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);
        //! Callback from clicking on the table, changes the selected QSO to that clicked. 
        static void cb_table(Fl_Widget* w, void* v);
        //! Copy previous QSOs.
        
        //! \param items QSOs that match band, mode an station callsign.
        //! \param possibles QSOs that mismatch on one or more.
        void set_data(std::set<qso_num_t> items, std::set<qso_num_t> possibles);
            
    protected:
        //! The previous QSOs: exact matches are listed first followed by possible matches.
        std::vector<qso_num_t> items_;

    };

protected:
    //! Search the book for the previous qSOs
    void get_qsos();
    //! Current QSO
    record* qso_;

    //widgets
    Fl_Output* op_call_;        //! Output showing callsign.
    table_d* table_details_;    //! Table showing callsign's details from previous QSOs.
    table_q* table_qsos_;       //! Table showing std::list of previous QSOs with callsign.


};

