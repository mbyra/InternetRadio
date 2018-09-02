#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include "err.h"

// Global variables used accross the program: *************
// Default values come from task description.
const int myIndex = 347056;
std::string DISCOVER_ADDR{"255.255.255.255"};
in_port_t CTRL_PORT{30000 + (myIndex % 10000)};
in_port_t UI_PORT{10000 + (myIndex % 10000)};
unsigned BSIZE = 65536;
unsigned RTIME = 250;
std::string NAME{"Nienazwany Nadajnik"};

// Parses all arguments from cli and saves them into global variables. Some
// variables have default value.
void parse(int argc, char **argv);

int main(int argc, char **argv) {
    parse(argc, argv);

    std::cout << "All important constants after parse:" << std::endl <<
    "DISCOVER_ADDR: " << DISCOVER_ADDR << std::endl <<
    "CTRL_PORT: " << CTRL_PORT << std::endl <<
    "UI_PORT: " << UI_PORT << std::endl <<
    "BSIZE: " << BSIZE << std::endl <<
    "RTIME: " << RTIME << std::endl <<
    "NAME : " << NAME << std::endl;

    // run receiver server and do everything described in pdf
    return 0;
}


unsigned parse_unsigned(const char* s) {
    if(!std::regex_match(optarg, std::regex("[0-9]+")))
        err("Not a positive number in argument");

    long number = strtol(optarg, nullptr, 10);

    return static_cast<unsigned int>(number);
}

unsigned parse_unsigned_16_bit(const char* s) {
    auto number = parse_unsigned(s);

    if (number >> 16)
        err("Not a unsigned 16 bit in argument");

    return number;
}

void parse(int argc, char **argv) {
    int c;
    long number;

    while((c = getopt(argc, argv, "d:C:U:b:R:n:")) != -1) {
        switch (c) {
            case 'd':
                // Discover address used by receiver to find transmitters.
                struct sockaddr_in sa;
                if (inet_pton(AF_INET, optarg, &(sa.sin_addr)) != 1)
                    err("Invalid discover address. It should be proper ipv4 "
                        "address.");
                DISCOVER_ADDR = std::string{optarg};
                break;
            case 'C':
                // UDP port used to transmit control packets.
                CTRL_PORT = static_cast<in_port_t>(parse_unsigned_16_bit(optarg));
                break;
            case 'U':
                // TCP port with user interface of radio receiver.
                UI_PORT = static_cast<in_port_t>(parse_unsigned_16_bit(optarg));
                break;
            case 'b':
                // Size (in bytes) of buffer.
                BSIZE = parse_unsigned(optarg);
                break;
            case 'R':
                // Time (ms) of retransmission of raports about missing packets.
                RTIME = parse_unsigned(optarg);
                break;
            case 'n':
                // Name of transmitter.
                NAME = optarg;
                break;
        }
    }
}