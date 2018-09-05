//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_REQUESTGATHERER_H
#define INTERNETRADIOSIK_REQUESTGATHERER_H

#include <sstream>

class Transmitter;

class RequestGatherer {
    friend class Transmitter;

public:
    RequestGatherer(Transmitter* transmitter) : transmitter(transmitter) {};

private:
    Transmitter* transmitter;

    int sock;

    // According to specification, transmitter should listen on UDP on CTRL_PORT
    // Based on scenario 09 from labs example, multi-recv.c
    void initializeSocket();

    // Unpacks list of firstBytes in stringstream and adds them to
    // retransmission requests contaner (in transmitter).
    // NO NEED TO ACQUIRE TRANSMITTER MUTEX, the method does it internally.
    void sendRexmitMessage(std::istringstream &iss);

    // Creates reply for lookup message and sends it to sender_addr.
    void sendReplyMessage(struct sockaddr_in &sender_addr);

    void start();
};



#endif //INTERNETRADIOSIK_REQUESTGATHERER_H
