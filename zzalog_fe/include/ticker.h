#pragma once

#include <cstdint>
#include <vector>




//! This class provides a central timer to control all the real time activity.

//! It is clocked every 100 milliseconds. Any object that wants a regular tick
//! registers a callback and period.
class ticker {

    //! Ticker callback.
    typedef void callback(void* v);

    //! Entry registering an object's request for a tick.
    struct ticker_entry {
        void* object { nullptr };     //!< Pointer to the object - gets returned to the object
        callback* tick { nullptr };   //!< The static function to call
        unsigned int period_ds{ 0 };  //!< Period in units of 100 milliseconds
        bool active{ false };         //!< Ticker active
        bool not_ticked{ true };      //!< Ticker not yet sent - so send it regardless
    };

    public:
    
    //! Constructor
    ticker();
    //! Destructor
    ~ticker();
    //! Set ticker
    
    //! \param object Pointer to the object requesting a tick.
    //! \param cb Method to use as callback.
    //! \param interval Tick period in units of 100 milliseconds.
    //! \param immediate If true, immediately issues a callback to the requesting unit.
    void add_ticker(void* object, callback* cb, unsigned int interval, bool immediate = true);
    //! Remove ticker for \p object.
    void remove_ticker(void* object);
    //! Suspend/restart ticker.
    
    //! \param object Pointer to the requesting object.
    //! \param active If true restart the ticker otherwise suspend it.
    void activate_ticker(void* object, bool active);
    //! Stop all tickers
    void stop_all();
    
    protected:

    //! Callback that gets repeated every 100 milliseconds.
    static void cb_ticker(void* v);

    //! The register of tick requests.
    std::vector<ticker_entry*> tickers_;
    //! Current time (in units of 100 milliseconds since started).
    unsigned int tick_count_;
    
};