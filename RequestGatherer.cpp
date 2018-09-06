//
// Created by marcin on 05.09.18.
//

#include <arpa/inet.h>
#include <bits/socket.h>
#include <iostream>
#include "RequestGatherer.h"
#include "err.h"
#include "parameters.h"
#include "Transmitter.h"


void RequestGatherer::start() {
//    debug("RequestGatherer : start() : beginning");
    initializeSocket();
//    debug("RequestGatherer : start() : after socket initialization");

    char buf[800000];

    while(true) {
        // Wait for any package on CTRL_PORT
        struct sockaddr_in sender_addr;
        socklen_t sender_addr_len = sizeof(sender_addr);

        ssize_t recv_len = recvfrom(sock, buf, sizeof(buf), 0,
                (struct sockaddr *) &sender_addr, &sender_addr_len);

        if (recv_len < 1) {
            // Just ignore if some error occurs, he will send another packages
//            debug("RequestGatherer : start() : recv_len < 1");
        }
        else {
            // There is a message in buffer, lets unpack it:
            // TODO check if rcvlen can be "good message" by checking size?

            std::istringstream iss(std::string(buf, recv_len));
            std::string tmp;
            if (iss >> tmp) {
                // tmp should contain now name of message:
                if (tmp == LOOKUP_MESSAGE) {
                    // Its cyclic lookup message.
//                    debug("RequestGatherer : start() : before sending reply "
//                          "message");
                    sendReplyMessage(sender_addr);
//                    debug("RequestGatherer : start() : after sending reply "
//                          "message");
                }

                else if (tmp == REXMIT_MESSAGE) {
                    // Message is retransmission request in form
                    // e.g LOUDER_PLEASE 512,1024,1536,5632,3584
//                    debug("RequestGatherer : start() : before sending "
//                          "rexmit message");
                    sendRexmitMessage(iss);
//                    debug("RequestGatherer : start() : after sending "
//                          "rexmit message");
                }
            }
        }

    }
}

// As in other classes, based on laboratory 09 ('UDP with many receivers')
// example, multi-recv.c
void RequestGatherer::initializeSocket() {
    /* otworzenie gniazda */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        syserr("socket");

    /* podpięcie się do grupy rozsyłania (ang. multicast) */
    struct ip_mreq ip_mreq;
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(MCAST_ADDR.c_str(), &ip_mreq.imr_multiaddr) == 0)
        syserr("inet_aton");
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof ip_mreq) < 0)
        syserr("setsockopt");

    // https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        syserr("setsockopt(SO_REUSEADDR) failed");


    /* podpięcie się pod lokalny adres i port */
    struct sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(CTRL_PORT_SENDER);
    if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0)
        syserr("bind");
}

void RequestGatherer::sendRexmitMessage(std::istringstream &iss) {
    std::string firstByte;
    while(std::getline(iss, firstByte, ',')) {
        try {
            transmitter->mut.lock();
            transmitter->requests.push(std::stoull(firstByte)); //TODO strtoull?
            transmitter->mut.unlock();
        } catch(...) {
            // Do nothing, receiver will send another request if necessary.
            return;
        }

    }
}

void RequestGatherer::sendReplyMessage(struct sockaddr_in &sender_addr) {
    // reply should be in the form:
    // BOREWICZ_HERE [MCAST_ADDR] [DATA_PORT] [nazwa stacji]
    std::string reply = REPLY_MESSAGE;
    reply += " ";
    reply += MCAST_ADDR;
    reply += " ";
    reply += std::to_string(DATA_PORT);
    reply += " ";
    reply += STATION_NAME;
    reply += "\n"; // TODO czy na pewno?

//    debug("RequestGatherer : sendReplyMessage() : messageToSend:_%s_", reply.c_str());

    socklen_t sender_addr_len = sizeof(sender_addr);
    auto snd_len = sendto(sock, reply.c_str(), reply.length(), 0,
                          (struct sockaddr *) &sender_addr, sender_addr_len);
    if (snd_len != (ssize_t) reply.length()) {
//        debug("RequestGatherer : sendReplyMessage() : partial/failed send");

        // syserr("partial / failed write");

        // Ignoring, he will send again and we will try again.
    }
}
