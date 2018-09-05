#include "parameters.h"

// Default values come from task description.

// Global flags for receiver:
std::string DISCOVER_ADDR{"255.255.255.255"};
in_port_t CTRL_PORT{30000 + (myIndex % 10000)};
in_port_t UI_PORT{10000 + (myIndex % 10000)};
unsigned BSIZE{65536};
unsigned RTIME{250};
std::string PREFERRED_STATION{""};


// Global flags for sender:
std::string MCAST_ADDR = "";
in_port_t DATA_PORT{20000 + (myIndex % 10000)};
in_port_t CTRL_PORT_SENDER{30000 + (myIndex % 10000)};
unsigned PSIZE{512}; // in bytes
unsigned FSIZE{131072}; // in bytes, = 128kB
unsigned RTIME_SENDER{250}; // in ms
std::string STATION_NAME = "Nienazwany Nadajnik";


std::string LOOKUP_MESSAGE{"ZERO_SEVEN_COME_IN"};
std::string REPLY_MESSAGE{"BOREWICZ_HERE"};
std::string REXMIT_MESSAGE{"LOUDER_PLEASE"};