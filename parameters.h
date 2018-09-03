#ifndef INTERNETRADIOSIK_PARAMETERS_H
#define INTERNETRADIOSIK_PARAMETERS_H

#include <string>
#include <netinet/in.h>

// Global variables used accross the program: *************
// Default values come from task description.
extern std::string DISCOVER_ADDR;
extern in_port_t CTRL_PORT;
extern in_port_t UI_PORT;
extern unsigned BSIZE;
extern unsigned RTIME;
extern std::string NAME;

const int myIndex = 347056; // used to generate default values of flags
const int MAX_STATION_NAME = 64;
const int TTL_VALUE = 60;
const int LOOKUP_INTERVAL = 5; // time between subsequent station lookups



#endif //INTERNETRADIOSIK_PARAMETERS_H
