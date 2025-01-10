#include "paddle_sm.h"
#include "key_handler.h"
#include "key_out.h"

#include <FL/Fl.H>
#include <FL/fl_ask.H>

// Time keeping is in ms. Define a minute as seconds
const double MINUTE = 60.0;
// Number of dit times in the word "PARIS" - ".__. ._ ._. .. ...   "
const int DITS_PER_WORD = 50;

paddle_sm::paddle_sm() :
    dash_(false),
    dit_(false),
    dash_pending_(false),
    dit_pending_(false),
    type_(SQUEEZE_A),
    dit_paddle_(key_state::LEFT),
    the_state_(IDLE),
    wpm_(12.0),
    weighting_(3.0),
    key_out_(nullptr),
    enabled_(false)
{

}

paddle_sm::~paddle_sm() {

}

// Set type and which paddle is used for dit
void paddle_sm::set_mode(paddle_t t) {
    type_ = t;
    switch (t) {
    case NONE:
        dit_paddle_ = NEITHER;
        break;
    case STRAIGHT_LEFT:
    case SQUEEZE_A:
    case SQUEEZE_B:
    case FULL_BUG:
    case SEMI_BUG:
        dit_paddle_ = LEFT;
        break;
    case STRAIGHT_RIGHT:
    case SQUEEZE_A_REV:
    case SQUEEZE_B_REV:
    case FULL_BUG_REV:
    case SEMI_BUG_REV:
        dit_paddle_ = RIGHT;
        break;
    case STRAIGHT_EITHER:
        dit_paddle_ = BOTH;
        break;
    }
}

// Set enable
void paddle_sm::enable(bool b) {
    enabled_ = b;
}

// Set the speed parameters
void paddle_sm::set_speed(
	double wpm,        // Words per minute
	double weighting   // Dash to dot ratio
) {
	wpm_ = wpm;
	weighting_ = weighting;
	dit_time_ = MINUTE / wpm / DITS_PER_WORD;
	dash_time_ = dit_time_ * weighting_;
    guard_time_ = dit_time_ * 0.5;
    space_time_ = 1.5 * dit_time_;
}

// Access key out
void paddle_sm::set_key_out(key_out* k) {
	key_out_ = k;
}

// Call back to receive paddle state change
void paddle_sm::cb_paddle(void* v, key_state state) {
	paddle_sm* that = (paddle_sm*)v;
	that->update_dit_dash(state);
}

// Timer callback
void paddle_sm::cb_timer(void* v) {
	paddle_sm* that = (paddle_sm*)v;
	that->the_state_ = that->next_state();
}

// Either on or off
paddle_sm::state_t paddle_sm::ns_straight() {
    state_t next;
    switch (the_state_) {
    case IDLE:
    case PASS_SPACE:
        if (dit_) {
            next = PASS_DIT;
        }
        break;
    case PASS_DIT:
        if (!dit_) {
            next = PASS_SPACE;
        }
        break;
    default:
        // Should not get here
        fl_message("Invalid state not supported by squeeze keyer");
        next = IDLE;
        break;
    }
    send_key(next, 0.0);

    return next;
}

paddle_sm::state_t paddle_sm::ns_squeeze() {
    state_t next;
    double duration;
    switch (the_state_) {
    case IDLE:
    case SPACE_UP: {
        // Go to what the paddle says - dit takes priority
        if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            next = DASH_DOWN;
            duration = dash_time_;
        }
        else {
            next = IDLE;
            duration = 0.0;
        }
        break;
    }
    case GUARD_UP: {
        // Go to what the paddle says - dit takes priority
        if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            next = DASH_DOWN;
            duration = dash_time_;
        }
        else {
            next = SPACE_UP;
            duration = space_time_;
        }
        break;
    }
    case DIT_DOWN: {
        // Complete the sign by adding a space
        next = DIT_UP;
        duration = dit_time_;
        break;
    }
    case DIT_UP: {
        // If we now need to send a dash do it
        if (dash_ || dash_pending_) {
            next = DASH_DOWN;
            duration = dash_time_;
        }
        // Keep sending dits until we stop
        else if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else {
            next = GUARD_UP;
            duration = guard_time_;
        }
        break;
    }
    case DASH_DOWN: {
        // Complete the sign by adding a space
        next = DASH_UP;
        duration = dit_time_;
        break;
    }
    case DASH_UP: {
        // If we now need to send a dit do it
        if (dit_ || dit_pending_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        // Carry on sending dashes
        else if (dash_) {
            next = DASH_DOWN;
            duration = dash_time_;
        }
        else {
            next = GUARD_UP;
            duration = guard_time_;
        }
        break;
    }
    default: {
        // Should not get here
        fl_message("Invalid state not supported by squeeze keyer");
        next = IDLE;
        break;
    }
    }
    if (duration > 0.0) {
        send_key(next, duration);
    }
    return next;

}
paddle_sm::state_t paddle_sm::ns_full_bug() {
    // TODO Change timings for fully-auto timings
    state_t next;
    double duration;
    switch (the_state_) {
    case IDLE:
    case SPACE_UP: {
        // Go to what the paddle says - dit takes priority
        if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            next = DASH_DOWN;
            duration = dash_time_;
        }
        else {
            next = IDLE;
            duration = 0.0;
        }
        break;
    }
    case GUARD_UP: {
        // Go to what the paddle says - dit takes priority
        if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            next = DASH_DOWN;
            duration = dash_time_;
        }
        else {
            next = SPACE_UP;
            duration = space_time_;
        }
        break;
    }
    case DIT_DOWN: {
        // Complete the sign by adding a space
        next = DIT_UP;
        duration = dit_time_;
        break;
    }
    case DIT_UP: {
        if (dit_) {
            // Keep sending dits
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            // Go to sending dashes
            next = DASH_DOWN;
            duration = dash_time_;
        }
        else {
            next = GUARD_UP;
            duration = guard_time_;
        }
        break;
    }
    case DASH_DOWN: {
        // Complete the sign by adding a space
        next = DASH_UP;
        duration = dit_time_;
        break;
    }
    case DASH_UP: {
        // Keep sending dashes
        if (dash_) {
            next = DASH_DOWN;
            duration = dash_time_;
        }
        else if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else {
            next = GUARD_UP;
            duration = guard_time_;
        }
        break;
    }
    default: {
        // Should not get here
        fl_message("Invalid state not supported by squeeze keyer");
        next = IDLE;
        break;
    }
    }
    if (duration > 0.0) {
        send_key(next, duration);
    }
    return next;
}
paddle_sm::state_t paddle_sm::ns_semi_bug() {
    // TODO Change semi-auto bug timings
    state_t next;
    double duration;
    switch (the_state_) {
    case IDLE:
    case SPACE_UP:
    case PASS_SPACE: {
        // Go to what the paddle says - dit takes priority
        if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            next = PASS_DASH;
            duration = 0.0;
        }
        else {
            next = IDLE;
            duration = 0.0;
        }
        break;
    }
    case GUARD_UP: {
        // Go to what the paddle says - dit takes priority
        if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            next = PASS_DASH;
            duration = 0.0;
        }
        else {
            next = PASS_SPACE;
            duration = 0.0;
        }
        break;
    }
    case DIT_DOWN: {
        // Complete the sign by adding a space
        next = DIT_UP;
        duration = dit_time_;
        break;
    }
    case DIT_UP: {
        if (dit_) {
            next = DIT_DOWN;
            duration = dit_time_;
        }
        else if (dash_) {
            next = PASS_DASH;
            duration = 0.0;
        } else {
            next = PASS_SPACE;
            duration = 0.0;
        }
        break;
    }
    default: {
        // Should not get here
        fl_message("Invalid state not supported by squeeze keyer");
        next = IDLE;
        break;
    }
    }
    send_key(next, duration);
    return next;
}

// Check next state - called on timer or paddle call backs
paddle_sm::state_t paddle_sm::next_state() {
    if (!enabled_) return IDLE;
    // Straight key - does not really use the state machine
    switch (type_) {
    case STRAIGHT_EITHER:
    case STRAIGHT_LEFT:
    case STRAIGHT_RIGHT:
        return ns_straight();
    case SQUEEZE_A:
    case SQUEEZE_A_REV:
    case SQUEEZE_B:
    case SQUEEZE_B_REV:
        return ns_squeeze();
    case FULL_BUG:
    case FULL_BUG_REV:
        return ns_full_bug();
    case SEMI_BUG:
    case SEMI_BUG_REV:
        return ns_semi_bug();
    default:
        return IDLE;
    }
}

// Update dit and dash values
void paddle_sm::update_dit_dash(key_state ps) {
    switch (dit_paddle_) {
    case NEITHER:
        dit_ = false;
        dash_ = false;
        break;
    case LEFT:
        switch (ps) {
        case NEITHER:
            dit_ = false;
            dash_ = false;
            break;
        case LEFT:
            dit_ = true;
            dash_ = false;
            break;
        case RIGHT:
            dit_ = false;
            dash_ = true;
            break;
        case BOTH:
            dit_ = true;
            dash_ = true;
            break;
        }
    case RIGHT:
        switch (ps) {
        case NEITHER:
            dit_ = false;
            dash_ = false;
            break;
        case LEFT:
            dit_ = false;
            dash_ = true;
            break;
        case RIGHT:
            dit_ = true;
            dash_ = false;
            break;
        case BOTH:
            dit_ = true;
            dash_ = true;
            break;
        }
    case BOTH:
        dit_ = true;
        dash_ = false;
    }
    switch (type_) {
    case STRAIGHT_EITHER:
    case STRAIGHT_LEFT:
    case STRAIGHT_RIGHT:
    case FULL_BUG:
    case FULL_BUG_REV:
    case SEMI_BUG:
    case SEMI_BUG_REV:
        dit_pending_ = false;
        dash_pending_ = false;
        break;
    case SQUEEZE_A:
    case SQUEEZE_A_REV:
        udd_squeeze(true);
        break;
    case SQUEEZE_B:
    case SQUEEZE_B_REV:
        udd_squeeze(false);
        break;
    }
    // If we are not active then kick into life
    if (dit_ || dash_) {
        if (the_state_ == IDLE) {
            the_state_ = next_state();
        }
    }
}

// Squeeze mode - next dit/dash to send
void paddle_sm::udd_squeeze(bool mode_a) {
    switch (the_state_) {
    case IDLE:
    case SPACE_UP:
    case GUARD_UP: {
        // Clear pending memories
        dit_pending_ = false;
        dash_pending_ = false;
        break;
    }
    case DIT_DOWN: {
        // Sending a dit, remember dash paddle in mode B
        dit_pending_ = false;
        dash_pending_ = !mode_a ? dash_ || dash_pending_ : false;
        break;
    }
    case DASH_DOWN: {
        // Sensing a dash, remember dit paddle in mode B
        dit_pending_ = !mode_a ? dit_ || dit_pending_ : false;
        dash_pending_ = false;
        break;
    }
    case DIT_UP: {
        // Sending the space after a dit, remeber dash pddle in both modes
        dit_pending_ = false;
        dash_pending_ = dash_ || dash_pending_;
        break;
    }
    case DASH_UP: {
        // Sending the space after a dash, remeber dit pddle in both modes
        dit_pending_ = dit_ || dit_pending_;
        dash_pending_ = false;
        break;
    }
    }
 }

// Send key
void paddle_sm::send_key(state_t state, double duration) {
    switch (state) {
    case IDLE:
    case DIT_UP:
    case DASH_UP:
    case SPACE_UP:
    case GUARD_UP:
        key_out_->set_key(false);
        break;
    case DIT_DOWN:
    case DASH_DOWN:
        key_out_->set_key(true);
        break;
    case PASS_DIT:
        key_out_->set_key(true);
        break;
    case PASS_DASH:
        key_out_->set_key(true);
        break;
    case PASS_SPACE:
        key_out_->set_key(false);
        break;
    }
    // This will re-use the existing tmeout or create a new one
    if (duration > 0.0) {
        Fl::repeat_timeout(duration, cb_timer, this);
    }
}


