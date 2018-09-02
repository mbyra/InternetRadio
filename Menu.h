#ifndef INTERNETRADIOSIK_MENU_H
#define INTERNETRADIOSIK_MENU_H

#include <vector>
#include <string>
#include <map>
#include "Transmitter.h"


class Menu {
    std::string current_station = "";
    std::map<std::string, Transmitter> station_list;

public:
//    Menu(std::string current_station, std::map<std::string, Transmitter>
//            station_list) : current_station(current_station), station_list
//            (station_list) {};

    Menu() = default;

    std::string text() const;
};


#endif //INTERNETRADIOSIK_MENU_H
