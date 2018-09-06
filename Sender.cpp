//
// Created by marcin on 05.09.18.
//

#include "Sender.h"
#include "err.h"
#include <arpa/inet.h>
#include <zconf.h>
#include <sstream>
#include "parameters.h"
#include "Transmitter.h"
#include <string>
#include <thread>
#include <iostream>
#include <cinttypes>
#include <byteswap.h>
#include <cstring>
#include "err.h"



void Sender::start() {


    debug("Sender : start() : beginning");
    initializeDataSocket();
    debug("Sender : start() : after socket initialization");

    char buf[PSIZE];
    int bufEndIndex = 0;
    ssize_t readSize = 0;

    while(true) {
        // read(source descriptor, dest. first byte, length of buffer)
        const int STDIN = 0;
        readSize = read(STDIN, buf + bufEndIndex, sizeof(buf)-bufEndIndex);
        if (readSize <= 0) {
            // Error while reading (<) or end of input (=). Either way, break.
            debug("Sender : start() : error while reading from STDIN");
            exit(0); // TODO sprawdzić czy na pewno tak
            break;
        }
        bufEndIndex += readSize;

        if (bufEndIndex == sizeof(buf)) {
            // buffer is full, sending package:
            bufEndIndex = 0;
            readSize = 0;

            // Create a string to send: (field audio_data of Audio Package)
            char bufWithVars[2*sizeof(uint64_t) + PSIZE];

            // Add sessionID in network order to buffer.
            uint64_t sessionID_network = bswap_64(transmitter->sessionID);
            memcpy(bufWithVars, &sessionID_network, sizeof(uint64_t));

            // After this, add firstByte in network order to buffer.
            uint64_t firstByte_network = bswap_64(nextFirstByte);
            memcpy(bufWithVars + sizeof(uint64_t), &firstByte_network, sizeof(uint64_t));

            // Now add raw data (PISZE bytes):
            memcpy(bufWithVars + 2*sizeof(uint64_t), buf, PSIZE);

            // Send bufWithVars:
            if (write(dataSock, bufWithVars, 2*sizeof(uint64_t) + PSIZE)
                                != (long long) 2*sizeof(uint64_t) + PSIZE) {
                // Try to live with this fact and work on.
                std::cerr << "Error in write, continuing." << std::endl;
                debug("Sender : start() : error writing to dataSock");
            }


            mut.lock();
            std::string toSend(buf, PSIZE);
            data.emplace(std::make_pair(nextFirstByte, toSend)); // Add
            // audioData to FIFO
            if (data.size() > FSIZE/PSIZE) // If fifo is full
                data.erase(data.begin()); // Remove oldest audioData
            // Prepare firstByte for next package:
            nextFirstByte += PSIZE;
            mut.unlock();
        } else {
            debug("Sender : start() : buffer is not full");

        }
    }
}


// Laboratory 05 - UDP sockets, paragraph alternative control flow
void Sender::initializeDataSocket() {

    /* zmienne i struktury opisujące gniazda */
    int optval;

    /* otworzenie gniazda */
    dataSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (dataSock < 0)
        syserr("socket");

    /* uaktywnienie rozgłaszania (ang. broadcast) */
    optval = 1;
    if (setsockopt(dataSock, SOL_SOCKET, SO_BROADCAST, (void*)&optval,
                   sizeof optval) < 0)
        syserr("setsockopt broadcast");

    /* ustawienie TTL dla datagramów rozsyłanych do grupy */
    optval = TTL_VALUE;
    if (setsockopt(dataSock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval,
                   sizeof optval) < 0)
        syserr("setsockopt multicast ttl");

//    /* zablokowanie rozsyłania grupowego do siebie */
//    optval = 0;
//    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval,
//                   sizeof optval) < 0)
//        syserr("setsockopt loop");

    /* podpięcie się pod lokalny adres i port */
    struct sockaddr_in receiverAddress;
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(DATA_PORT);
    if (inet_aton(MCAST_ADDR.c_str(), &receiverAddress.sin_addr) == 0)
        syserr("inet_aton");

    // We use connect to easily switch connections
    if (connect(dataSock, (struct sockaddr *)&receiverAddress,
            sizeof receiverAddress) < 0)
        syserr("connect");
}



void Sender::startRetransmissionSender() {

    debug("RetransmissionSender : start() : beginning");
    while(true) {
        transmitter->mut.lock();

        while(not transmitter->requests.empty()) {
            uint64_t firstByte = transmitter->requests.front();
            transmitter->requests.pop();

            transmitter->sender->mut.lock();
            auto iter = transmitter->sender->data.find(firstByte);
            // TODO it's a map, can I just data[firstByte] instead?
            if (iter != transmitter->sender->data.end()) {
                debug("RetransmissionSender : start() : sending "
                      "retransmission");

//                std::cerr << "sending retransmission" << std::endl;

                // Create a string to send: (field audio_data of Audio Package)
                char bufWithVars[2*sizeof(uint64_t) + PSIZE];

                // Add sessionID in network order to buffer.
                uint64_t sessionID_network = bswap_64(transmitter->sessionID);
                memcpy(bufWithVars, &sessionID_network, sizeof(uint64_t));

                // After this, add firstByte in network order to buffer.
                uint64_t firstByte_network = bswap_64(firstByte);
                memcpy(bufWithVars + sizeof(uint64_t), &firstByte_network, sizeof(uint64_t));

                // Now add raw data (PISZE bytes):
                memcpy(bufWithVars + 2*sizeof(uint64_t),
                       (iter->second).c_str(), PSIZE);

                // Send bufWithVars: // TODO this write is from zconf.h, should it?
                if (write(transmitter->sender->dataSock, bufWithVars, 2*sizeof(uint64_t) + PSIZE)
                    != (long long) 2*sizeof(uint64_t) + PSIZE) {
                    // Try to live with this fact and work on.
//                    std::cerr << "Error in write, continuing." << std::endl;
                    debug("RetransmissionSender : start() : error while "
                          "writing retransmission to dataSock");
                }

            }
            transmitter->sender->mut.unlock();
        }

        transmitter->mut.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(RTIME));
    }
}