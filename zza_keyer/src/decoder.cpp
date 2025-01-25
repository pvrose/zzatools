#include "decoder.h"
#include "display.h"
#include "engine.h"
#include "wave_gen.h"

using namespace std;

extern engine* engine_;
extern wave_gen* wave_gen_;
extern display* display_;

// Morse lookup tables - Some characters map onto LCD1602 character set
// Characters have from 1 to 6 signs (dits or dashes). Error code_ has 8 signs
// * is used for unidentified character; \nnn is octal for non ASCII characters
// non-ASCII are limited to U+0080 to U+00FF only.
const char MLU1[] = "ET";
const char MLU2[] = "IANM";
const char MLU3[] = "SURWDKGO";
const char MLU4[] = "HVF\xdcL\xc4PJ" "BXCYZQ\xd6*";
const char MLU5[] = "54*3***2" "&*+****1" "6=/*\xc9*(*" "7***8*90";
const char MLU6[] = "********" "****?_**" "**\"**.**" "**@*****"
                    "********" "**;!*)**" "***,****" ":*******";

const double MAX_DIT = 2.0;          // Maximum length of a dit in dit-times
const double MAX_SIGN_GAP = 2.0;     // Diffreence between a dit/dash gap and a character gap
const double MAX_CHAR_GAP = 4.0;     // Difference between a dit/dash gap and a word gap
const double MAX_WORD_GAP = 10.0;    // Maximum wait in dit gaps before forcing end of word
const double MAX_DASH = 5.0;         // Maximum length of a dash
const double TIMEOUT = 5.0;          // Timeout set to 5 seconds
const double BIAS = 2.0 / 3.0;       // Factor used in calculating biased average bit period
const double MIN_WT = 2.8;           // Minimum weighting
const double MAX_WT = 4.8;           // Maximum weighting
// Time keeping is in ms. Define a minute as seconds
const double MINUTE = 60000.0;
// Number of dit times in the word "PARIS" - ".__. ._ ._. .. ...   "
const int DITS_PER_WORD = 50;

decoder::decoder() 
{
    previous_signal_ = new signal_def({ false, 0 });
    t_decoder_ = nullptr;
    close_ = false;
    wpm_ = 0.0;
    weighting_ = 0.0;
    dit_time_ = 0;
    dash_time_ = 0;
    key_idle_ = false;
    code_ = 0;
    len_code_ = 0;
    char_in_progress_ = false;
    characters_ = "";

}

decoder::~decoder() {
    close_ = true;
    t_decoder_->join();
}

// Set initial speed
void decoder::set_speed(double wpm, double weighting) {
	wpm_ = wpm;
	weighting_ = weighting;
	dit_time_ = MINUTE / wpm / DITS_PER_WORD;
	dash_time_ = dit_time_ * weighting_;
    printf("WPM %g, weighting %g, Tdit %g, Tdash %g\n", wpm_, weighting_, dit_time_, dash_time_);
}

// Get current speed
void decoder::get_speed(double& wpm, double& weighting) {
    wpm = wpm_;
    weighting = weighting_;
}

// Decode the key
decoder::decode_t decoder::decode_monitor(signal_def signal) {
    if (!signal.value) {
        // We were stuck low
        if (key_idle_) {
            return STUCK_LOW;
        }
        // Posedge on key
        else if (signal.durn_ms < dit_time_ / 5) {
            // Key bounce
            return NOISE;
        }
        else if (signal.durn_ms < MAX_SIGN_GAP * dit_time_) {
            // Inter sign gap
            return SIGN;
        }
        else if (signal.durn_ms < MAX_CHAR_GAP * dit_time_) {
            // Inter character gap
            return CHAR;
        }
        else {
            // Inter word gap
            return WORD;
        }
    }
        //else {
        //    // Check stuckk high
        //    if (signal.durn_ms > TIMEOUT) {
        //        return STUCK_HIGH;
        //    }
        //    else {
        //        return NO_CHANGE;
        //    }
        //}
    //}
    else {
        // Negedge on key
        if (signal.durn_ms < dit_time_ / 5) {
            // Key bounce
            return NOISE;
        }
        else if (signal.durn_ms < MAX_DIT * dit_time_) {
            // less tha two dit times treat as a fdit
            return DIT;
        }
        else {
            // Greater than 2 dit times treat as a dash
            return DASH;
        }
        //}
        //else {
        //    // Check a long low
        //    if (signal.durn_ms > MAX_DASH * dit_time_) {
        //        // Traet is as a word space
        //        return STUCK_LOW;
        //    }
        //    else {
        //        return NO_CHANGE;
        //    }
        //}
    }
}

// re-evaluate the speed
void decoder::check_speed(decode_t decode, uint64_t duration_ms) {
    switch (decode) {
    case DIT:
    case SIGN: {
        // Generate biased avaearge between existing dit time and last dit/gap
        dit_time_ = (BIAS * dit_time_) + ((1.- - BIAS) * duration_ms);
        break;
    }
    case DASH: {
        // First adjust dash length
        dash_time_ = ((BIAS * dash_time_) + ((100 - BIAS) * duration_ms)) / 100;
        // Now allocate it to weighting_ and dit_time_ - first
        // calculate what weight would be needed
        weighting_ = dash_time_ / dit_time_;
        // Check within allowable range and if not set weight to either limit
        // and adjust dit time to compensate
        if (weighting_ < MIN_WT) {
            weighting_ = MIN_WT;
            dit_time_ = dash_time_ / weighting_;
        }
        else if (weighting_ > MAX_WT) {
            weighting_ = MAX_WT;
            dit_time_ = dash_time_ / weighting_;
        } // else use the unadjusted weight and leave dit time unaltered
        break;
    }
    default:
        // Ignore the duration
        break;
    }
    wpm_ = MINUTE / DITS_PER_WORD / dit_time_;
}

char decoder::morse_to_letter() {
    switch (len_code_) {
    case 0: return '*';
    case 1: return MLU1[code_];
    case 2: return MLU2[code_];
    case 3: return MLU3[code_];
    case 4: return MLU4[code_];
    case 5: return MLU5[code_];
    case 6: return MLU6[code_];
    case 8: if (code_ == 0) return '\xab'; // <- (backspace) character
    default: return '*';
    }
}

// Analyse the key change
void decoder::do_key_change(decode_t decode) {
    switch (decode) {
    case NO_CHANGE: {
        return;
    }
    case DIT: {
        len_code_ += 1;
        // Shift left and or in a dit
        code_ += code_;
        char_in_progress_ = true;
        key_idle_ = false;
        break;
    }
    case DASH: {
        len_code_ += 1;
        // Shift left and or in a dash
        code_ += code_ + 1;
        char_in_progress_ = true;
        key_idle_ = false;
        break;
    }
    case SIGN: {
        key_idle_ = false;
        break;
    }
    case CHAR: {
        // End of character
        send_char(morse_to_letter());
        code_ = 0;
        len_code_ = 0;
        key_idle_ = false;
        break;
    }
    case WORD: {
        // End of word, - complete the current dit or dash and add a word space
        send_char(morse_to_letter());
        send_char(' ');
        code_ = 0;
        len_code_ = 0;
        key_idle_ = false;
        break;
    }
    case NOISE: {
        return;
    }
    case STUCK_HIGH: {
        if (!key_idle_) {
            send_char(morse_to_letter());
            code_ = 0;
            len_code_ = 0;
            printf("There appears to be a problem - key is stuck high\n");
        }
        key_idle_ = true;
        break;
    }
    case STUCK_LOW: {
        if (!key_idle_) {
            send_char(morse_to_letter());
            send_char(' ');
            code_ = 0;
            len_code_ = 0;
        }
        key_idle_ = true;
        break;
    }
    }

}

// SEnd character
void decoder::send_char(char c) {
    characters_ += c;
    // Get display to update the monitor
    Fl::awake(display::cb_monitor, display_);
}

// Get character
string decoder::get_characters() {
    return characters_;
}

// Start the decoder
void decoder::start() {
    t_decoder_ = new thread(run_thread, this);
}

// Run the thread
void decoder::run_thread(decoder* that) {
    that->run_decoder();
}

// That does this
void decoder::run_decoder() {
    bool timed_out = false;
    while (!close_) {
        signal_def signal;
        signal = wave_gen_->get_signal();
        if (signal.value != previous_signal_->value) {
            decode_t decode = decode_monitor(*previous_signal_);
            do_key_change(decode);
            //printf("Edge detected - was %d for %d ms Decode %s Code = %x (l%d)\n",
            //    previous_signal_->value, previous_signal_->durn_ms, decode_text[decode], code_, len_code_);
            timed_out = false;
        } else if (!timed_out && signal.durn_ms > (dit_time_ * MAX_WORD_GAP)) {
            decode_t decode = decode_monitor(*previous_signal_);
            do_key_change(decode);
            //printf("Timeout detected - was %d for %d ms Decode %s Code = %x (l%d)\n",
            //    previous_signal_->value, previous_signal_->durn_ms, decode_text[decode], code_, len_code_);
            timed_out = true;
        } 
        this_thread::yield();
        *previous_signal_ = signal;
    }
}

