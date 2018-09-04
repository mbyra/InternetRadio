//
// Created by marcin on 02.09.18.
//

#ifndef INTERNETRADIOSIK_STATION_H
#define INTERNETRADIOSIK_STATION_H

#include <string>
#include <arpa/inet.h>
#include <chrono>

// This class contains all information need to comunicate with transmitter.
class Station {


public:
    std::string stationName;
    std::string mcastAddress; // Transmitter's broadcast address.
    struct sockaddr_in transmitterAddress; // Transmitter's streaming address.
    int transmitterPort; // Transmitter's streaming port.
    std::chrono::time_point<std::chrono::system_clock> lastContactTime;

    Station(const std::string stationName,
            const std::string mcastAddress,
            const sockaddr_in &transmitterAddress,
            int transmitterPort)
                        :  stationName(stationName),
                           mcastAddress(mcastAddress),
                           transmitterAddress(transmitterAddress),
                           transmitterPort(transmitterPort),
                           lastContactTime(std::chrono::system_clock::now()) {}
};


#endif //INTERNETRADIOSIK_STATION_H
