#include "ticker.h"

#include <FL/Fl.H>

extern bool DEBUG_QUICK;

const double TICK = 0.1;

// Constructor
ticker::ticker() {
    tick_count_ = 0;
    Fl::add_timeout(TICK, cb_ticker, this);
}

// Destructor
ticker::~ticker() {
    Fl::remove_timeout(cb_ticker);
    for( auto it = tickers_.begin(); it != tickers_.end(); it++) {
        delete *it;
    };
}

// Add ticker
void ticker::add_ticker(void* object, callback* cb, unsigned int interval) {
    ticker_entry* entry = new ticker_entry;
    entry->object = object;
    entry->tick = cb;
    entry->period_ds = DEBUG_QUICK ? max(interval, 3000U) : interval;
    tickers_.push_back(entry);
}

// Remove ticker
void ticker::remove_ticker(void* object) {
    bool go_on = true;
    for (auto it = tickers_.begin(); it != tickers_.end() && go_on; it++) {
        if ((*it)->object == object) {
            // We are removing this. it should be safe as we clear go_on
            tickers_.erase(it);
            go_on = false;
        }
    }
}

// Call back - check tickers
void ticker::cb_ticker(void * v) {
    ticker* that = (ticker*)v;
    that->tick_count_++;
    for (auto it = that->tickers_.begin(); it != that->tickers_.end(); it++) {
        // Send ticks to all who need it at this time
        if (that->tick_count_ % (*it)->period_ds == 0) {
            (*it)->tick((*it)->object);
        }
    }
    // Now repeat timer
    Fl::repeat_timeout(TICK, cb_ticker, v);
}