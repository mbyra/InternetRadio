#ifndef INTERNETRADIOSIK_RETRANSMISSIONREQUESTER_H
#define INTERNETRADIOSIK_RETRANSMISSIONREQUESTER_H

#include "Receiver.h"

class Receiver;

class RetransmissionRequester {

public:
    friend class Receiver;
    // Main method of class: starts service sending retransmission requests
    // every RTIME miliseconds.
    void start();

private:
    Receiver* receiver;

    RetransmissionRequester(Receiver *receiver) : receiver(receiver) {};
    ~RetransmissionRequester();

};



#endif //INTERNETRADIOSIK_RETRANSMISSIONREQUESTER_H
