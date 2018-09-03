//
// Created by marcin on 02.09.18.
//

#ifndef INTERNETRADIOSIK_STATION_H
#define INTERNETRADIOSIK_STATION_H

#include <string>
#include <arpa/inet.h>

// This class contains all information need to comunicate with transmitter.
class Station {


public:
    std::string station_name;
    std::string miracast_address; // Transmitter's broadcast address.
    struct sockaddr_in transmitter_address; // Transmitter's streaming address.
    int transmitter_port; // Transmitter's streaming port.
    long long creation_time; // For timer to request retransmission.

    Station(const std::string station_name,
            const std::string miracast_address,
            const sockaddr_in &transmitter_address,
            int transmitter_port, long long int creation_time)
                                    :  station_name(station_name),
                                       miracast_address(miracast_address),
                                       transmitter_address(transmitter_address),
                                       transmitter_port(transmitter_port),
                                       creation_time(creation_time) {}
};


#endif //INTERNETRADIOSIK_STATION_H
