#include "sark_handler.h"

#include <cstdio>
#include <cstring>

#include <fcntl.h> // Contains file sark_controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal sark_control definitions
#include <unistd.h> // write(), read(), close()

#include "FL/Fl.H"

sark_handler::sark_handler(const char* device) {
    device_ = device;
    // Open the device
    serial_port_ = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    // Check it opened
    if (serial_port_ < 0) {
        printf("ERROR: Failed to open %s: %d(%s)\n",
            device, errno, strerror(errno));
        return;
    } else {
        printf("Opened %s\n", device_.c_str());
        // termios struct
        struct termios tty;
        // Read in existing settings, and handle any error
        // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
        // must have been initialized with a call to tcgetattr() overwise behaviour
        // is undefined
        if(tcgetattr(serial_port_, &tty) != 0) {
            printf("ERROR: Reading existing attributes: %d(%s)\n", errno, strerror(errno));
            return;
           }
        // Set to 8-N-1
        tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
        tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
        tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
        tty.c_cflag |= CS8; // 8 bits per byte (most common)
        tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow sark_control (most common)
 //       tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
        tty.c_cflag |= CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
        // tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO; // Disable echo
        tty.c_lflag &= ~ECHOE; // Disable erasure
        tty.c_lflag &= ~ECHONL; // Disable new-line echo
        // tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
        // tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl       
        tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
//        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
        tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;
 
        cfsetspeed(&tty, B57600); // Set baudrate to 57600
        // Save tty settings, also checking for error
        if (tcsetattr(serial_port_, TCSANOW, &tty) != 0) {
            printf("ERROR: Fail to set new attributes: %d(%s)\n", errno, strerror(errno));
            return;
        }
    }
}

sark_handler::~sark_handler() {
    // If we've opened the link close it
    if (serial_port_ != -1) {
        close(serial_port_);
    }
}

// size_t sark_handler::read_line() {
//     memset(buffer_, 0, sizeof(buffer_));
//     char* buff = buffer_;
//     size_t bytes = 0;
//     bool line_done = false;
//     while (bytes == 0) {
//         bytes = 0;
//         buff = buffer_;
//         line_done = false;
//         while (!line_done) {
//             ssize_t resp = read(serial_port_, buff, 1);
//             if (resp <= 0) {
//                 printf("Error %d reading port (%s)\n", errno, strerror(errno));
//                 return resp;
//             } else {
//             switch(*buff) {
//                     case '\r':
//                         break;
//                     case '\n':
//                         *buff = '\0';
//                         buff += resp;
//                         line_done = true;
//                         break;
//                     default:
//                         buff += resp;
//                         bytes += resp;
//                         break;
//                 }
//             }
//         }
//     }

//     return bytes;
// }

// Get the reading
bool sark_handler::read_data(sark_data* data, bool raw, bool add_sign) {
    char command[128];
    memset(command, 0, sizeof(command));
    data_ = data;
//    bool ok = process_response(RESP_FLUSH);
//    if (!ok) return false;
    
    sark_data::scan_params params = data->get_params();

    if (raw) printf("Started reading raw data from SARK-100 ");
    else printf("Started reading computed data from SARK-100 ");
    fflush(stdout);

    memset(command, 0, sizeof(command));
    if (raw) {
        snprintf(command, sizeof(command), cmd_scanr, params.start, params.end + 1, params.step);
    } else {
        snprintf(command, sizeof(command), cmd_scan, params.start, params.end + 1, params.step);
    }
    write_cmd(command);
    process_response(RESP_START);
    for (size_t ix = 0; ix < data->size(); ix++) {
        if (raw) {
            if (add_sign) { 
                process_response(RESP_RDATA);
            } else {
                process_response(RESP_RDATA_ABS);
            }
        } else {
            process_response(RESP_IDATA);
        }
    } 
    process_response(RESP_END);
}

// Get the reading
bool sark_handler::read_datai(sark_data* data, bool raw, bool add_sign) {
    char command[128];
    memset(command, 0, sizeof(command));
    data_ = data;
    // bool ok = process_response(RESP_FLUSH);
    // if (!ok) return false;
    
    sark_data::scan_params params = data->get_params();

    memset(command, 0, sizeof(command));
    if (raw) printf("Started reading individual raw data from SARK-100 ");
    else printf("Started reading individual computed data from SARK-100 ");
    fflush(stdout);

    memset(command, 0, sizeof(command));
    for (int64_t f = params.start; f <= params.end; f+=params.step) {
        snprintf(command, sizeof(command), cmd_freq, f);
        write_cmd(command);
        process_response(RESP_OK_NG);
        write_cmd(cmd_on);
        process_response(RESP_OK_NG);
        // check_response();
        if (raw) {
            write_cmd(cmd_raw);
            if (add_sign) {
                process_response(RESP_RDATA);
            } else {
                process_response(RESP_RDATA_ABS);
            }
        } else {
            write_cmd(cmd_imp);
            process_response(RESP_IDATA);
        }

        printf(".");
        fflush(stdout);
    }
    printf(" DONE!\n");

 //   check_prompt();
 }


void sark_handler::write_cmd(const char* cmd) {
    write(serial_port_, cmd, strlen(cmd));
}

bool sark_handler::process_response(sark_handler::resp_state state) {

    memset(buffer_, 0, sizeof(buffer_));
    bool found = false;
    while (!found) {
        memset(buffer_, 0, sizeof(buffer_));
        ssize_t r = read(serial_port_, buffer_, sizeof(buffer_));
        if (r < 0) {
            if (errno != EAGAIN) {
                printf("ERROR %d (%s)\n", errno, strerror(errno));
                found = true;
            }
        }
        if (r > 0) {
            for (char* buff = buffer_; !found && *buff; buff++) {
                switch (*buff) {
                    case '\r':
                    case '\n': break;
                    case '>': break;   // ignore thse
                    default: {
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    switch (state) {
        case RESP_FLUSH: {
            // Ignore everythin except '>>' at end
            if (strncmp(buffer_, ">>", 2) == 0) {
                return true;
            }
            break;
        }
        case RESP_OK_NG: {
            // Expect OK/Error:
            if (strncmp(buffer_, "OK", 2) == 0) {
                return true;
            } else if (strncmp(buffer_, "Error", 5) == 0) {
                printf("Received: %s\n", buffer_);
            } else {
                printf("Unexpected: %s\n", buffer_);
            }
            break;
        }
        case RESP_START: {
            // Expect "Start"
            if (strncmp(buffer_, "Start", 2) == 0) {
                return true;
            }
            break;
        }
        case RESP_IDATA: {
            // Expect SWR R X Z values
            // This is a convoluted way of doing this but directly reading struc
            // fields results in gibberish
            float swr;
            int r, x, z;
            int n = sscanf(buffer_, "%g,%d,%d,%d", &swr, &r, &x, &z);
            sark_data::comp_data ip = { swr, (double)r, (double)z, (double)x};
            data_->add_reading(ip);
            return true;
        }
        case RESP_RDATA: {
            // Expect raw data (voltages)
            sark_data::raw_data ip = {0, 0, 0, 0};
            sscanf(buffer_, "%d,%d,%d,%d", &ip.Vf, &ip.Vr, &ip.Vz, &ip.Va);
            data_->add_reading(ip, true);
            return true;
        }
            case RESP_RDATA_ABS: {
            // Expect raw data (voltages)
            sark_data::raw_data ip = {0, 0, 0, 0};
            sscanf(buffer_, "%d,%d,%d,%d", &ip.Vf, &ip.Vr, &ip.Vz, &ip.Va);
            data_->add_reading(ip, false);
            return true;
        }
        case RESP_END: {
            // Expect "End"
            if (strncmp(buffer_, "End", 2) == 0) {
                return true;
            }
            break;
        }
        case RESP_PROMPT: {
            // Expect ">>"
            if (strcmp(buffer_, ">>") == 0) {
                return true;
            }
        }
        break;
    }
    printf("ERROR: Unexpected state in receive data\n");
}