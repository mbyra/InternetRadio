//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_RETRANSMISSIONSENDER_H
#define INTERNETRADIOSIK_RETRANSMISSIONSENDER_H


#include "Sender.h"

class Transmitter;

class RetransmissionSender {
    friend class Transmitter;

public:
    explicit RetransmissionSender(Transmitter* transmitter) : transmitter(transmitter) {};

private:
    Transmitter* transmitter;

    void start();
};



#endif //INTERNETRADIOSIK_RETRANSMISSIONSENDER_H
