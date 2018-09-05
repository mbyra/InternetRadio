//
// Created by marcin on 03.09.18.
//

#include "DataDownloader.h"
#include "Receiver.h"
#include "err.h"
#include <sys/time.h>
#include <cassert>
#include <cstring>
#include <sstream>
#include <thread>
#include <iostream>


void DataDownloader::restartDownloader() {

    debug("DataDownloader : restartDownloader() : beginning");
    receiver->mut.lock();
    debug("DataDownloader : restartDownloader() : after locking receiver "
          "mutex");

    bool socketIsNotSet = currSock == -1;
    bool changedMcastAddress = currMcastAddress !=
            receiver->currentStation->mcastAddress;
    bool changedTransmitterPort = currTransmitterPort !=
            receiver->currentStation->transmitterPort;

    debug("DataDownloader : restartDownloader() : socketIsNotSet: %d, "
          "changedMcastAddress: %d, changedTransmitterPort: %d",
          socketIsNotSet, changedMcastAddress, changedTransmitterPort);

    // If something from above occurs, reinitialize socket.
    if(socketIsNotSet || changedMcastAddress || changedTransmitterPort) {
        debug("DataDownloader : restartDownloader() : starting reinitializing"
              " socket");

        assert(receiver->stationIsSet);
        currMcastAddress = receiver->currentStation->mcastAddress;
        currTransmitterPort = receiver->currentStation->transmitterPort;

        struct sockaddr_in local_address;
        struct ip_mreq ip_mreq;

        debug("DataDownloader : restartDownloader() : before locking transmitterParamsMutex");

        transmitterParamsMutex.lock();
        debug("DataDownloader : restartDownloader() : after locking "
              "transmitterParamsMutex");


        // This method can be triggered many times, also with "living' socket.
        // In this case, it is necessary to close the socket.
        if (currSock != -1) {
            debug("DataDownloader : restartDownloader() : closing currSock");
            close(currSock);
        }

        // Now we initialize socket. As in other parts of program, this fragment
        // is based on laboratory 09 example: multi-recv.c

        /* otworzenie gniazda */
        currSock = socket(AF_INET, SOCK_DGRAM, 0);
        if (currSock < 0)
            syserr("socket");

        /* podpięcie się pod lokalny adres i port */
        local_address.sin_family = AF_INET;
        local_address.sin_addr.s_addr = htonl(INADDR_ANY);
        local_address.sin_port = htons(currTransmitterPort);
        if (bind(currSock, (struct sockaddr *)&local_address,
                sizeof local_address) < 0)
            syserr("bind");

        /* podpięcie się do grupy rozsyłania (ang. multicast) */
        ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (inet_aton(currMcastAddress.c_str(), &ip_mreq.imr_multiaddr) == 0)
            syserr("inet_aton");
        if (setsockopt(currSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq,
                       sizeof ip_mreq) < 0)
            syserr("setsockopt");

        // Setting timeout: https://stackoverflow.com/a/28484682/6121465
        struct timeval timeout={2,0}; //set timeout for 2 seconds
        /* set receive UDP message timeout */
        setsockopt(currSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,
                   sizeof(struct timeval));

        transmitterParamsMutex.unlock();
    }
    debug("DataDownloader : restartDownloader() : socket reinitialized");


    receiver->requester->mut.lock();
    receiver->requester->requests.clear(); // TODO should I do it?
    receiver->requester->mut.unlock();
    debug("DataDownloader : restartDownloader() : retransmission requests "
          "cleared");

    bufferMutex.lock();
    buffer.clear();
    bufferMutex.unlock();

    debug("DataDownloader : restartDownloader() : buffer cleared");

    receiver->isPlayingNow = true;

    receiver->mut.unlock();
    debug("DataDownloader : restartDownloader() : closing");

}



void DataDownloader::addRetransmissionRequestOfMissingPackages(AudioPackage ap) {


    bool thisHasGreatest1stByte =
            ap.first_byte_num == buffer.rbegin()->first;
    bool otherPackagesAreInBuffer = buffer.size() > 1;
    if (otherPackagesAreInBuffer and thisHasGreatest1stByte) {
        // ... so lets check if there are any packages missing since
        // previous package
        uint64_t missingByte = oldestPossibleMissingFirstByte(ap);


        std::list<uint64_t> missingBytes;
        if(missingByte < ap.first_byte_num) {
            // There are some missing bytes indeed.
            debug("DataDownloader: there are missing packages : will add to requests container");

            while(missingByte < ap.first_byte_num) {
                missingBytes.push_back(missingByte);
                missingByte += ap.audio_data.size();
            }

            receiver->requester->mut.lock();
            receiver->requester->requests.emplace_back(
                    std::chrono::system_clock::now() +
                    std::chrono::milliseconds(RTIME),
                    missingBytes);
            receiver->requester->mut.unlock();
            debug("DataDownloader: there are missing packages : finished dealing with it");
        }
    }
}

void DataDownloader::eraseOldPackagesFromBuffer(AudioPackage current,
        ssize_t audioDataLength) {
    // Removing too old packages
    if (not buffer.empty()) {
        auto iter = buffer.begin();
        // find end of buffer first package that should not be removed:
        while (iter->first != buffer.end()->first
           and iter->first < current.first_byte_num + audioDataLength - BSIZE) {
            iter++;
        }

        if (buffer.begin() != iter) {
            debug("DataDownloader: erasing too old packages from buffer");
            buffer.erase(buffer.begin(), iter);
        }
    }
}


void DataDownloader::start() {
    debug("DataDownloader: start() : beginning");
    restartDownloader();
    debug("DataDownloader: start() : after restartDownloader() at beginning");

    // Buffer for reading packages
    char buf[1'000'000];

    while (true) {

        // If station was changed or is not set at all, take some actions:
        receiver->mut.lock();
        if(receiver->state != STANDARD) {
            isPlaybackValid = false;
            playbackID++;
            if(receiver->state == STATION_NOT_SET) {
                // close socket, clean up, return
                debug("DataDownloader: start() : station not set ");
                assert(currSock != -1);
                if (close(currSock) < 0)
                    perror("close");
                currSock = -1;
                receiver->isPlayingNow = false;
                return;
            } else if (receiver->stationIsSet == STATION_CHANGED) {
                debug("DataDownloader: start() : station changed : beginning");
                // continue working, but prepare to start playing new station
                receiver->mut.unlock(); // TODO ??
                restartDownloader();
                receiver->mut.lock();
                receiver->state = STANDARD;
                debug("DataDownloader: start() : station changed : restarted");
            }
        }


        // Now state is STANDARD, so we can just continue standard routine,
        // that is: read from socket, parse package, and play or wait for
        // more packets if necessary.

        auto rcv_len = read(currSock, buf, strlen(buf));
        // According to specification, we should keep the receiver running as
        // long as possible, so we ignore failure in read, because it can be
        // requested for retransmission further.
        // Moreover valid audio package should contain at least session_id
        // and first_byte_num
        AudioPackage ap;
        if (rcv_len >= sizeof(ap.session_id) + sizeof(ap.first_byte_num)) {
            debug("DataDownloader: start() : received valid AudioPackage");

            std::string rec{buf, (unsigned long) rcv_len};

            ap.session_id = bytesToUint64(rec.substr(0,8));
            ap.first_byte_num = bytesToUint64(rec.substr(8,8));
            ap.audio_data = rec.substr(16, rcv_len - 16);

            bufferMutex.lock();

            // Check if this is first package received from this transmitter.
            if (!initializedTransmission) {
                debug("DataDownloader: start() : AudioPackage is first package");
                currSesionId = ap.session_id;
                currByteZero = ap.first_byte_num;
                initializedTransmission = true;
            }

            // Since now we don't care if this is first or not first package
            if(ap.session_id < currSesionId) {
                debug("DataDownloader: start() : ap.session_id < currSessionID");
                // Specification says: ignore this
                bufferMutex.unlock();
            }
            else if (ap.session_id > currSesionId) {
                debug("DataDownloader: start() : ap.session_id > currSessionID : beginning");
                // Specification says: start playing this from the beginning.
                isPlaybackValid = false;
                playbackID++;
                bufferMutex.unlock();
                restartDownloader();
                debug("DataDownloader: start() : ap.session_id > currSessionID : restartedDownloader");
            }
            else { // ap.session_id == currSessionId  ,i.e. standard case
                // Add new package to buffer
                buffer.emplace(ap.first_byte_num, ap);

                // Check if we are not missing some packages:
                addRetransmissionRequestOfMissingPackages(ap);

                // Erase from buffer packages older than last [BSIZE/PSIZE],
                // that is, make the buffer keep as many packages as
                // specification says, not implementation.
                eraseOldPackagesFromBuffer(ap,
                        rcv_len-sizeof(ap.first_byte_num)-sizeof(ap.session_id));

                // Specification says: start playing if buffer is filled in 75%
                // (only if !isPlaybackValid, because otherwise music would
                // be playing now)
                if (!isPlaybackValid
                        and ap.first_byte_num >= currByteZero + 3*BSIZE/4) {
                    debug("DataDownloader: start() : was not valid and buffer"
                          " is now 3/4 full: starting playing service");
                    std::thread t([this]() {play(currByteZero, playbackID); });
                    debug("DataDownloader: start() : was not valid and buffer"
                          " is now 3/4 full: playing service stopped");
                }

                bufferMutex.unlock();
            }

        }
    }
}

void DataDownloader::play(uint64_t firstByte, uint64_t pbID) {
    while(isPlaybackValid) {
        auto iter = buffer.find(firstByte);
        if(iter != buffer.end()) {
            // Playing at least
            std::cout << iter->second.audio_data;
            firstByte += iter->second.audio_data.size();
        }
        else {
            isPlaybackValid = false;
            playbackID++;
            bufferMutex.unlock(); // TODO czy aby na pewno
            restartDownloader();
        }
    }
}


uint64_t DataDownloader::oldestPossibleMissingFirstByte(AudioPackage lastPackage) {
    auto previous = buffer.rbegin()++;
    // If there are bytes missing, this will be first:
    auto firstMissingByte =
            previous->first + lastPackage.audio_data.size();
    // If there are no packages missing then
    // firstMissingByte == ap.firstByteNum

    // Odbiornik nie wysyła próśb o retransmisję paczek
    // zawierających bajty wcześniejsze niż BYTE0, ani tak
    // starych, że i tak nie będzie na nie miejsca w buforze.

    // In buffer, there is unused BSIZE % PSIZE bytes (less
    // than one package size)
    auto unused = BSIZE % lastPackage.audio_data.size();
    // So usable size of buffer:
    auto usable = buffer.size() - unused;
    // If buffer was not implemented as char[very much], it
    // would mean that there really can fit only usable/PSIZE
    // packages, in reality it is more.
    // Lets find firstByteNum of oldest possible missing package
    auto oldestPossibleFirstByte = lastPackage.first_byte_num - usable;
    // Now lets set firstMissing byte to oldestPossible, if
    // it is lower than it:
    firstMissingByte =
            std::max(firstMissingByte, oldestPossibleFirstByte);

    return firstMissingByte;
}

// https://stackoverflow.com/questions/42919525/convert-array-of-bytes-to-uint64-t
uint64_t DataDownloader::bytesToUint64(std::string str) {
    assert(str.length() == 8);
    return  (uint64_t)(str[7]) |
            (uint64_t)(str[6]) << 8  |
            (uint64_t)(str[5]) << 16 |
            (uint64_t)(str[4]) << 24 |
            (uint64_t)(str[3]) << 32 |
            (uint64_t)(str[2]) << 40 |
            (uint64_t)(str[1]) << 48 |
            (uint64_t)(str[0]) << 56;
}


