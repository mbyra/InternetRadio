//
// Created by marcin on 05.09.18.
//



#include <getopt.h>
#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <cassert>
#include <regex>
#include <iostream>
#include "parameters.h"
#include "err.h"

// Prints diagnostic data to stdout.
void diagnose() {
    std::cout << "All important constants after parse:" << std::endl <<
              "MCAST_ADDR: " << MCAST_ADDR << std::endl <<
              "DATA_PORT: " << DATA_PORT << std::endl <<
              "CTRL_PORT_SENDER: " << CTRL_PORT_SENDER << std::endl <<
              "PSIZE: " << PSIZE << std::endl <<
              "FSIZE: " << FSIZE << std::endl <<
              "RTIME_SENDER: " << RTIME_SENDER << std::endl <<
              "STATION_NAME : " << STATION_NAME << std::endl;
}

// Checks if string contains unsigned integer. If so, returns it.
// Otherwise, prints error message and exits with code 1.
unsigned parse_unsigned(const char* s) {
    if(!std::regex_match(optarg, std::regex("[0-9]+")))
        fatal("Not a positive number in argument");

    long number = strtol(optarg, nullptr, 10);

    return static_cast<unsigned int>(number);
}

// Checks if string contains unsigned 16 bit integer. If so, returns it.
// Otherwise, prints error message and exits with code 1.
unsigned parse_unsigned_16_bit(const char* s) {
    unsigned number = parse_unsigned(s);

    if (number >> 16)
        fatal("Not a unsigned 16 bit in argument");

    return number;
}

// Parses all arguments from cli and saves them into global variables. Some
// variables have default value.
void parse(int argc, char **argv) {
    int c;

    while((c = getopt(argc, argv, "a:P:C:p:f:R:n:")) != -1) {
        switch (c) {
            case 'a':
                // Broadcast address, this argument is obligatory.
                struct sockaddr_in sa;
                if (inet_pton(AF_INET, optarg, &(sa.sin_addr)) != 1)
                    fatal("Invalid discover address. It should be proper ipv4 "
                          "address.");
                MCAST_ADDR = std::string{optarg};
                break;
            case 'P':
                // Data port used to transmit data packets (audio packets)
                DATA_PORT = static_cast<in_port_t>
                        (parse_unsigned_16_bit(optarg));
                break;
            case 'C':
                // Port for controlling packets
                CTRL_PORT_SENDER = static_cast<in_port_t>
                        (parse_unsigned_16_bit(optarg));
                break;
            case 'p':
                // Size of single audio packet (in bytes)
                PSIZE = parse_unsigned(optarg);
                break;
            case 'f':
                // Size of fifo container (in bytes)
                FSIZE = parse_unsigned(optarg);
                break;
            case 'R':
                // Time between subsequent retransmissions of packets.
                RTIME_SENDER = parse_unsigned(optarg);
                break;
            case 'n':
                // Name of this.
                if (strlen(optarg) > MAX_STATION_NAME) {
                    char message [50];
                    sprintf(message, "Max station length is %d.\n",
                            MAX_STATION_NAME);
                    fatal(message);
                }
                STATION_NAME = optarg;
                break;
            default:
                assert(true);
        }
    }

    if (MCAST_ADDR.empty()){
            printf("Usage %s [optional arguments]\n"
                   "List of all possible flags [default value]):\n"
                   "\t-a broadcast address OBLIGATORY\n"
                   "\t-P data port [27056]\n"
                   "\t-C control port [37056]\n"
                   "\t-p packet size in bytes [512]\n"
                   "\t-f fifo size in bytes [131072 (=128kB)]\n"
                   "\t-R retransmission time in ms [250]\n"
                   "\t-n station name [\"Nienazwany Nadajnik\"]\n"
                    , argv[0]);
        }

    diagnose();
}






int main(int argc, char **argv) {
    parse(argc, argv);

    return 0;
}
