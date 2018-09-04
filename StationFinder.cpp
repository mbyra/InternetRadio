//
// Created by marcin on 02.09.18.
//

#include "StationFinder.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>

#include "err.h"
#include "parameters.h"
#include <fcntl.h>
#include <cstring>
#include <cassert>
#include <sstream>


// Creates udp socket used for scanning for transmitters. Sets options and
// binds the socket.
// Based on scenario 09 from labs - multi-send.c
void StationFinder::initCtrlSocket() {

    /* zmienne i struktury opisujące gniazda */
    int optval;

    /* otworzenie gniazda */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        syserr("socket");

    /* uaktywnienie rozgłaszania (ang. broadcast) */
    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval,
                   sizeof optval) < 0)
        syserr("setsockopt broadcast");

    /* ustawienie TTL dla datagramów rozsyłanych do grupy */
    optval = TTL_VALUE;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval,
                   sizeof optval) < 0)
        syserr("setsockopt multicast ttl");

//    /* zablokowanie rozsyłania grupowego do siebie */
//    optval = 0;
//    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval,
//                   sizeof optval) < 0)
//        syserr("setsockopt loop");

    /* podpięcie się pod lokalny adres i port */
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(CTRL_PORT);
    if (inet_aton(DISCOVER_ADDR.c_str(), &receiverAddress.sin_addr) == 0)
        syserr("inet_aton");

}

void StationFinder::searchStationService() {

    const char* buf = LOOKUP_MESSAGE.c_str();
    while (true) {
        // Laboratory 05: echo-client.c
        auto snd_len = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)
                &receiverAddress, sizeof(receiverAddress));
        if (snd_len != (ssize_t) strlen(buf)) {
            syserr("partial / failed write");
        }

        receiver->mut.lock();

        auto iter = receiver->stationList.begin();
        while (iter != receiver->stationList.end()) {
            // If last contact was over 20s ago, remove station from list.
            if (std::chrono::duration_cast<std::chrono::seconds>
                    (iter->lastContactTime - std::chrono::system_clock::now())
                    >= std::chrono::seconds(STATION_DELETION_TIME)) {

                // If removed station is current, unset currentStation
                if(receiver->currentStation == iter) {
//                    receiver->currentStation;
                    receiver->stationIsSet = false;
                }


                auto next = iter++;
                assert(next != iter); // I dont remember if the ++ place matters
                receiver->stationList.erase(iter);
            }
            else {
                iter++;
            }
        }

        receiver->mut.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(LOOKUP_INTERVAL));
    }

}

void StationFinder::replyParserService() {
    while(true) {
        char buf[500];
        struct sockaddr_in currentTransmitter;
        auto rcva_len = (socklen_t) sizeof(currentTransmitter);

        // Receive message from first transmitter trying to contact:
        ssize_t length;
        if ((length = recvfrom(sock, buf, 500, 0, (struct sockaddr *)
                &currentTransmitter,
                     &rcva_len)) < 0)
            syserr("Error while recvfrom");

        // Parse this message:
        std::istringstream ss(std::string(buf, rcva_len));
        std::string s;
        ss >> s;
        if (s == REPLY_MESSAGE) {
            std::string address, port_str, name;
            ss >> address >> port_str >> name;
            int port;
            try {
                port = std::stoi(port_str);
            } catch (std::exception &e) {
                continue; // We must do everything not to crash the receiver
            }

            // Task description states that name can contain spaces, so we
            // continue do add spaces to 'name' and next parts from ss.
            while(ss >> s)
                name += " " + s;

            // Now 'name' contains name of existing or new station.
            receiver->mut.lock();
            auto iter = receiver->stationList.begin();
            for (;iter != receiver->stationList.end(); iter++) {
                if (iter->stationName == name) {
                    // We have this station so lets just update lastContactTime
                    iter->lastContactTime = std::chrono::system_clock::now();
                    break;
                }
            }

            if (iter == receiver->stationList.end()) {
                // Name represents new station. Lets add it to list.
                currentTransmitter.sin_port = htons(CTRL_PORT);
                Station station(name, address, currentTransmitter, port);
                receiver->stationList.push_back(station);
            }

            // If preferred station was given as argument, receiver starts to
            // play it whenever discovered.
            if(PREFERRED_STATION == name) {
                auto iter = receiver->stationList.begin();
                for(; iter != receiver->stationList.end(); iter++) {
                    if(iter->stationName == name) {
                        receiver->currentStation = iter;
                        receiver->stationIsSet = true;
                        break;
                    }
                }
                // The station MUST be now on list and if clause entered
                assert(iter != receiver->stationList.end());


                if (!receiver->isPlayingNow) {
                    std::thread t([this]() {receiver->startDownloadingData();});
                    t.detach();
                }
            }
            receiver->mut.unlock();

            receiver->menu->clientMutex.lock();
            receiver->menu->refreshClientsMenu(); // TODO maybe it should be done periodically
            receiver->menu->clientMutex.unlock();
        }
    }

}

void StationFinder::start() {
    initCtrlSocket();

    // run replyParser service in separate thread
    std::thread replyParserServiceThread([this]() {replyParserService(); });
    replyParserServiceThread.detach();

    // in current thread run searchStation service
    searchStationService();
}
