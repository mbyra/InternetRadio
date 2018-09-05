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

    debug("StationFinder : searchStationService() : beginning");
    const char* buf = LOOKUP_MESSAGE.c_str();
    while (true) {
        debug("StationFinder : searchStationService() : starting lookup send");

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
                debug("StationFinder : searchStationService() : will remove station");


                // If removed station is current, unset currentStation
                if(receiver->currentStation == iter) {
//                    receiver->currentStation;
                    debug("StationFinder : searchStationService() : removed station was current");
                    receiver->stationIsSet = false;
                }


                auto next = iter++;
                assert(next != iter); // I dont remember if the ++ place matters
                receiver->stationList.erase(iter);
                debug("StationFinder : searchStationService() : station removed");
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
    debug("StationFinder : replyParserService() : beginning");

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
        debug("StationFinder : replyParserService() : received reply");


        // Parse this message:
        std::istringstream ss(std::string(buf, length));
        std::string s;
        ss >> s;
        debug("StationFinder : replyParserService() : reply message is %s", s.c_str());
        if (s == REPLY_MESSAGE) {
            debug("Message is same as REPLY_MESSAGE");
        }
        else {
            debug("Message is different from REPLY_MESSAGE. message:_%s_ , "
                  "REPLY_MESSAGE:_%s_", s.c_str(), REPLY_MESSAGE.c_str());
        }

        if (s == REPLY_MESSAGE) {
            debug("Entered loop");
            std::string address, port_str, name;
            ss >> address >> port_str >> name;
            debug("StationFinder : replyParserService() : reply encoding, "
                  "before casting port to int, "
                  "address: %s, port %s, name %s", address.c_str(), port_str.c_str(),
                  name.c_str());
            int port;
            try {
                port = std::stoi(port_str);
            } catch (std::exception &e) {
                debug("catch clause");
                continue; // We must do everything not to crash the receiver
            }



            debug("after try catch");

            // Task description states that name can contain spaces, so we
            // continue do add spaces to 'name' and next parts from ss.
            while(ss >> s)
                name += " " + s;

            debug("StationFinder : replyParserService() : reply encoded, "
                  "address: %s, port %d, name %s", address.c_str(), port,
                  name.c_str());

            // Now 'name' contains name of existing or new station.
            receiver->mut.lock();
            auto iter = receiver->stationList.begin();
            for (;iter != receiver->stationList.end(); iter++) {
                if (iter->stationName == name) {
                    debug("StationFinder : replyParserService() : we know "
                          "this station %s", name.c_str());
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
                debug("StationFinder : replyParserService() : found new "
                      "station, added to stationList");
            }


            // If this is a preferred station or there is nothing playing
            // now, there is a chance that we should start playing now.
            if(PREFERRED_STATION == name || not receiver->isPlayingNow) {
                debug("StationFinder : replyParserService() : reply from "
                      "preferred station, starting playing it");

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
                    // If othing is playing now, start playing.
                    debug("StationFinder : replyParserService() : reply from "
                          "preferred station and is not playing: starting "
                          "startDownloadData in separate thread and marking "
                          "state as STANDARD");
                    receiver->state = STANDARD;
                    receiver->mut.unlock();
                    std::thread t([this]() {receiver->startDownloadingData();});
                    t.detach();

                } else if (name == PREFERRED_STATION and
                                receiver->currentStation->stationName != name) {
                    // If something is playing now, but it is not a PREFERRED
                    // STATION, and we have found PREFERRED STATION, then we
                    // start playing this new station.
                    debug("StationFinder : replyParserService() : reply from "
                          "preferred station, is playing but this is "
                          "preferred: marking receiver state as "
                          "STATION_CHANGED");
                    receiver->state = STATION_CHANGED;
                    receiver->mut.unlock();
//                    std::thread t([this]() {receiver->startDownloadingData();});
//                    t.detach();


                }
            }

            receiver->menu->clientMutex.lock();
            receiver->menu->refreshClientsMenu(); // TODO maybe it should be done periodically
            receiver->menu->clientMutex.unlock();
        }
    }

}

void StationFinder::start() {
    debug("StationFinder : start() : beginning");
    initCtrlSocket();
    debug("StationFinder : start() : after initializing a socket");

    // run replyParser service in separate thread
    std::thread replyParserServiceThread([this]() {replyParserService(); });
    replyParserServiceThread.detach();
    debug("StationFinder : start() : replyParserService started in separate "
          "thread");

    debug("StationFinder : start() : starting searchStationService in this "
          "thread");
    // in current thread run searchStation service
    searchStationService();
    debug("StationFinder : start() : finished searchStationService");

}
