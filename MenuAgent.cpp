#include <sstream>
#include <cstring>
#include <cstdio>

#include "MenuAgent.h"

std::string MenuAgent::text() const {
    std::stringstream ss; // I will print all chunks of text into this ss.

    // heading:
    ss <<
    "------------------------------------------------------------------------\n"
    "  SIK Radio\n"
    "------------------------------------------------------------------------\n";

    // stations with indicator:
    bool first = true;
    for (const auto &station : station_list) {
        if (first) {
            first = false;
            if (current_station.empty())
                // There is no current station so I place indicator next to the
                // first item on list.
                ss << "  > " << station.first << "\n";
            else // There is a current station choosen already.
                ss << "    " << station.first << "\n";
            continue;
        }

        if (station.first == current_station)
            ss << "  > " << station.first << "\n";
        else
            ss << "    " << station.first << "\n";
    }

    // bottom:
    ss << "------------------------------------------------------------------------\n";

    return ss.str();
}

void MenuAgent::start() {

}
