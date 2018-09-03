#include "parameters.h"

// Default values come from task description.
extern std::string DISCOVER_ADDR{"255.255.255.255"};
in_port_t CTRL_PORT{30000 + (myIndex % 10000)};
in_port_t UI_PORT{10000 + (myIndex % 10000)};
unsigned BSIZE{65536};
unsigned RTIME{250};
std::string NAME{""};
