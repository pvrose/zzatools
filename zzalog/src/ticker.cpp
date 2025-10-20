#include "ticker.h"

#include "main.h"

#include <FL/Fl.H>

const double TICK = 0.1;

// Constructor
ticker::ticker() {
    tick_count_ = 0;
    Fl::add_timeout(TICK, cb_ticker, this);
}

// Destructor
void ticker::stop_all() {
    Fl::remove_timeout(cb_ticker);
    for( auto it = tickers_.begin(); it != tickers_.end(); it++) {
        delete *it;
    };
    tickers_.clear();
}

ticker::~ticker() {
    stop_all();
}

// Add ticker
void ticker::add_ticker(void* object, callback* cb, unsigned int interval, bool immediate) {
    ticker_entry* entry = new ticker_entry;
    entry->object = object;
    entry->tick = cb;
    entry->period_ds = DEBUG_QUICK ? min(interval, 3000U) : interval;
    entry->active = true;
    entry->not_ticked = immediate;
    tickers_.push_back(entry);
}

// Remove ticker
void ticker::remove_ticker(void* object) {
    bool go_on = true;
    auto itd = tickers_.begin();
    for (auto it = tickers_.begin(); it != tickers_.end() && go_on; it++) {
        if ((*it)->object == object) {
           go_on = false;
           itd = it;
        }
    }
    if (!go_on) {
        // We are removing this. it should be safe as we clear go_on
        tickers_.erase(itd);
    }
}

// Activate ticker
void ticker::activate_ticker(void* object, bool active) {
    bool go_on = true;
    auto it = tickers_.begin();
    for (; it != tickers_.end() && go_on; it++) {
        if ((*it)->object == object) {
            go_on = false;
            (*it)->active = active;
        }
    }
}

// Call back - check tickers
void ticker::cb_ticker(void * v) {
    ticker* that = (ticker*)v;
    that->tick_count_++;
    for (auto it = that->tickers_.begin(); it != that->tickers_.end(); it++) {
        // Send ticks to all who need it at this time
        if ((that->tick_count_ % (*it)->period_ds == 0) || (*it)->not_ticked) {
            if ((*it)->active) {
                (*it)->tick((*it)->object);
                (*it)->not_ticked = false;
            }
        }
    }
    // Now repeat timer
    Fl::repeat_timeout(TICK, cb_ticker, v);
}