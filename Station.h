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
    std::string station_name;
    std::string miracast_address; // Transmitter's broadcast address.
    struct sockaddr_in transmitter_address; // Transmitter's streaming address.
    int transmitter_port; // Transmitter's streaming port.
    std::chrono::time_point<std::chrono::system_clock> lastContactTime;

    Station(const std::string station_name,
            const std::string miracast_address,
            const sockaddr_in &transmitter_address,
            int transmitter_port)
                        :  station_name(station_name),
                           miracast_address(miracast_address),
                           transmitter_address(transmitter_address),
                           transmitter_port(transmitter_port),
                           lastContactTime(std::chrono::system_clock::now()) {}
};


#endif //INTERNETRADIOSIK_STATION_H
