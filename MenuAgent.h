#ifndef INTERNETRADIOSIK_MENU_H
#define INTERNETRADIOSIK_MENU_H

#include <vector>
#include <string>
#include <map>
#include "Transmitter.h"


class MenuAgent {
    std::string current_station = "";
    std::map<std::string, Transmitter> station_list;

public:
//    MenuAgent(std::string current_station, std::map<std::string, Transmitter>
//            station_list) : current_station(current_station), station_list
//            (station_list) {};

    std::string text() const;

    // Main function of class: initializes menu agent and runs it in infinite
    // loop.
    void start();
};




#endif //INTERNETRADIOSIK_MENU_H
