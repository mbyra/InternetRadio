//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_SENDER_H
#define INTERNETRADIOSIK_SENDER_H

#include <list>
#include <map>
#include <mutex>

class Transmitter;

// Class which main task is to read data from input and send it as UDP
// datagrams. Data format is specified as structure of AudioPackage.
class Sender {
    friend class Transmitter;

public:
    explicit Sender(Transmitter* transmitter) : transmitter(transmitter) {};

private:
    Transmitter* transmitter;

    int dataSock = -1;
    uint64_t nextFirstByte = 0;
    std::map<uint64_t, std::string> data; // FIFO for fast retransmission
    std::mutex mut;

    // Creates udp socket used for sending AudioPackages. Sets options and
    // connects the socket. Sends UDP datagrams to DATA_PORT (of MCAST_ADDR).
    // Based on scenario 05 from labs, paragraph 'alternate control flow'
    void initializeDataSocket();

    void start();

    void startRetransmissionSender();
};


#endif //INTERNETRADIOSIK_SENDER_H
