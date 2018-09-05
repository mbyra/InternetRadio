//
// Created by marcin on 05.09.18.
//

#include <byteswap.h>
#include <cstring>
#include "RetransmissionSender.h"
#include "Transmitter.h"
#include "parameters.h"
#include <zconf.h>

void RetransmissionSender::start() {

    while(true) {
        transmitter->mut.lock();

        while(not transmitter->requests.empty()) {
            uint64_t firstByte = transmitter->requests.front();
            transmitter->requests.pop();

            transmitter->sender->mut.lock();
            auto iter = transmitter->sender->data.find(firstByte);
            // TODO it's a map, can I just data[firstByte] instead?
            if (iter != transmitter->sender->data.end()) {
                std::cerr << "sending retransmission" << std::endl;

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
                    std::cerr << "Error in write, continuing." << std::endl;
                }

            }
            transmitter->sender->mut.unlock();
        }

        transmitter->mut.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(RTIME));
    }
}