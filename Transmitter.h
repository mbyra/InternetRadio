//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_TRANSMITTER_H
#define INTERNETRADIOSIK_TRANSMITTER_H

#include <queue>
#include <cstdint>
#include <mutex>
#include <chrono>
#include <thread>
#include <iostream>
#include "Sender.h"
#include "RequestGatherer.h"
#include "RetransmissionSender.h"

class Sender;
class RequestGatherer;
class RetransmissionSender;

class Transmitter {
    friend class Sender;
    friend class RequestGatherer;
    friend class RetransmissionSender;

private:
    // FIFO containing requests for retransmission (represented as firstByte)
    std::queue<uint64_t> requests;
    std::mutex mut;

    uint64_t sessionID;

    // According to specification, transmitter should have three threads:
    // 1. Reading data from input and sending it in UDP datagrams to DATA
    // port  of MCAST ADDRESS
    Sender* sender;

    // 2. Gathering retransmission requests in periods of RTIME ms
    RequestGatherer* requestGatherer;

    // 3. Retransmitting requested missing audio packets.
    RetransmissionSender* retransmissionSender;

    // TODO 2 and 3 in one or separate class?

public:
    Transmitter() {
        // Count returns number of ticks. After casting to seconds, this is
        // number of seconds since the beginning of epoch.
        sessionID = (uint64_t) std::chrono::duration_cast<std::chrono::seconds>
                (std::chrono::system_clock::now().time_since_epoch()).count();
    };

    ~Transmitter() {
        delete requestGatherer;
        delete sender;
        delete retransmissionSender;
    }

    // Main function of the class. Starts all services in infinite loops in
    // separate threads.
    void start() {
        sender = new Sender(this);
        std::cerr << "ELO ELOE 320" << std::endl;
        std::thread senderServiceThread([this]() { sender->start(); });
        senderServiceThread.detach();
        std::this_thread::sleep_for(std::chrono::seconds(2));


        requestGatherer = new RequestGatherer(this);
        std::thread gathererServiceThread([this]() { requestGatherer->start(); });
        gathererServiceThread.detach();

        retransmissionSender = new RetransmissionSender(this);
        retransmissionSender->start();
    }

};

#endif //INTERNETRADIOSIK_TRANSMITTER_H
