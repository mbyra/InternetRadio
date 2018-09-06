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
#include "err.h"

class Sender;
class RequestGatherer;

class Transmitter {
    friend class Sender;
    friend class RequestGatherer;

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
//        delete retransmissionSender;
    }

    // Main function of the class. Starts all services in infinite loops in
    // separate threads.
    void start() {
        sender = new Sender(this);

        std::thread senderServiceThread([this]() { sender->start(); });
        senderServiceThread.detach();

        std::thread retransmissionSenderThread
                ([this]() { sender->startRetransmissionSender() ;});
        retransmissionSenderThread.detach();


        requestGatherer = new RequestGatherer(this);
        // Start requestGatherer in this thread:
        requestGatherer->start();
//        std::thread gathererServiceThread([this]() { requestGatherer->start(); });
//        gathererServiceThread.detach();

    }

};

#endif //INTERNETRADIOSIK_TRANSMITTER_H
