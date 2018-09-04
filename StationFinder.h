//
// Created by marcin on 02.09.18.
//

#ifndef INTERNETRADIOSIK_STATIONFINDER_H
#define INTERNETRADIOSIK_STATIONFINDER_H

#include <unistd.h>

#include "Station.h"
#include "Receiver.h"

class Receiver;
class RetransmissionRequester;

class StationFinder {
    friend class Receiver;
    friend class RetransmissionRequester;

public:
    // Main function of class: initializes sockets etc. and starts looking
    // for available stations after every given period of time, adding or
    // deleting them from list if necessary: one thread periodically sends
    // requests for all available stations and second thread parses
    // transmitters' replies in infinite loop (and applies some actions
    // accordingly).
    void start();

private:
    Receiver *receiver;

    int sock;
    struct sockaddr_in receiverAddress;

    long long fetchId = 0;

    StationFinder(Receiver *receiver) : receiver(receiver) {};
    ~StationFinder() {
        close(sock);
    };

    // Creates udp socket used for scanning for transmitters. Sets options and
    // binds the socket.
    // Based on scenario 09 from labs - multi-send.c
    void initCtrlSocket();

    // In infinite loop, does a periodic lookup for available stations and adds
    // them to the list. Removes unavailable stations from list.
    void searchStationService();

    // In ininite loop, parses every reply from transmitters who were asked for
    // available stations in network. Removes unavailable stations from list.
    void replyParserService();

    // Main function of class: initializes socket and runs
    // searchStationService() and replyParser() in separate threads.


};


#endif //INTERNETRADIOSIK_STATIONFINDER_H
