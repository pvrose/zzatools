#pragma once

#include <cstdint>
#include <vector>

using namespace std;


// This class provides a central timer to control all the real time activity
class ticker {

    typedef void callback(void* v);

    struct ticker_entry {
        void* object { nullptr };       // Pointer to the object - gets returned to the object
        callback* tick { nullptr };     // The static function to call
        unsigned int period_ds{ 0 };         // Period in deciseconds
        bool active{ false };                    // Ticker active
        bool not_ticked{ true };        // Ticker not yet sent - so send it regardless
    };

    public:
    
    // Constructor
    ticker();
    // Destructor
    ~ticker();
    // Set ticker
    void add_ticker(void* object, callback* cb, unsigned int interval);
    // Remove ticker
    void remove_ticker(void* object);
    // Suspend/restart ticker
    void activate_ticker(void* object, bool active);
    // Close all tickers
    void stop_all();
    
    protected:

    // Decisecond ticker
    static void cb_ticker(void* v);

    // The list of objecst tp call
    vector<ticker_entry*> tickers_;
    // Current time (in ds since started)
    unsigned int tick_count_;
    
};