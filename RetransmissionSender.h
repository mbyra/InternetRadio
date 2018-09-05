//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_RETRANSMISSIONSENDER_H
#define INTERNETRADIOSIK_RETRANSMISSIONSENDER_H

// TODO merge this functionality it to Sender class

#include "Sender.h"

class Transmitter;

class RetransmissionSender {
    friend class Transmitter;

public:
    explicit RetransmissionSender(Transmitter* transmitter) : transmitter(transmitter) {};

private:
    Transmitter* transmitter;

    // Main function of class. In infinite loop, sends retransmission of all
    // packets from request container, taking data from FIFO data container,
    // then sleeps for RTIME.
    // NO NEED TO ACQUIRE TRANSMITTER MUTEX, the method does it internally.
    // NO NEED TO ACQUIRE SENDER MUTEX also (for modifying FIFO data container)
    void start();
};



#endif //INTERNETRADIOSIK_RETRANSMISSIONSENDER_H
