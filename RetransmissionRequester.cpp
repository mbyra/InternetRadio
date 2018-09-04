//
// Created by marcin on 05.09.18.
//

#include <thread>
#include <cmath>

#include "RetransmissionRequester.h"
#include "parameters.h"
#include "Receiver.h"

void RetransmissionRequester::start() {

    auto RTIME_ms = std::chrono::milliseconds(RTIME);
    using std::chrono::system_clock;

    std::chrono::ceil<std::chrono::milliseconds>(system_clock::now());

    while(true) {
        mut.lock();
        if(requests.empty())
            mut.unlock();
        else {
            auto req = requests.front(); // The oldest request.
            requests.pop_front();
            mut.unlock();

            // We have to sleep until req.first + k*RTIME, when k is minimal
            // natural number st. req.first + k*RTIME > now
            // k > (now - req.first)/RTIME
            // so we sleep until now + (now - req.first)/RTIME
            // (now - req.first)/RTIME is rounded up so it represents the
            // smalles k fulfilling inequality above so we just wait until
            // req.first + (k*RTIME casted to milliseconds)
            auto k = std::chrono::ceil<std::chrono::milliseconds>
                    (system_clock::now() - req.first) / RTIME_ms;
            std::this_thread::sleep_until(
                    req.first + std::chrono::milliseconds(k*RTIME_ms));

            // Now it is time for the next retransmission.

            // Firstly lets delete too old packets (older than currByteZero
            // or oldest package in buffer) or packets that have already been
            // retransmitted and are in buffer
            auto reducedList = removeUnnecessaryPackages(req.second);

            if (not reducedList.empty()) {
                // Reduced list is on awaiting request container again
                // because, according to specification, missing packages
                // should be requested every RTIME milliseconds until retrieved.
                mut.lock();
                requests.push_back(std::make_pair
                        (std::chrono::system_clock::now() +
                        std::chrono::milliseconds(RTIME), reducedList));
                mut.unlock();

                auto packagesListStr = createRequestPackagesMessage(reducedList);
                // TODO check if I need mutex because it can be the only place used
                receiver->downloader->transmitterParamsMutex.lock();
                auto snd_len = sendto(receiver->downloader->currSock,
                        packagesListStr.c_str(),
                        packagesListStr.length(), 0,
                        (struct sockaddr *) &receiver->currentStation
                                ->transmitterAddress,
                        sizeof(receiver->currentStation->transmitterAddress));
                if (snd_len) {
                    // Specification says we should keep receiver alive
                    // despite of communication problems, so I ignore send
                    // error. TODO check it
                }
                receiver->downloader->transmitterParamsMutex.unlock();

            }

        }
    }
}

std::string
RetransmissionRequester::createRequestPackagesMessage(std::list<uint64_t>
        list) {
    std::stringstream ss;
    for(auto element : list)
        ss << element << ',';

    std::string s = ss.str();
    // remove last comma:
    std::string withoutComma = s.substr(0, s.size() - 1);
    return std::string(REPLY_MESSAGE) + " " + withoutComma;
}

std::list<uint64_t>
RetransmissionRequester::removeUnnecessaryPackages(std::list<uint64_t> list) {
    std::list<uint64_t> listWithoutUnnecessaryPackets;
    receiver->downloader->bufferMutex.lock();
    for (auto byteNum : list) {
        if (byteNum < receiver->downloader->currByteZero
            or byteNum < receiver->downloader->buffer.begin()->first
            or receiver->downloader->buffer.find(byteNum) !=
               receiver->downloader->buffer.end()) {
            // Package is too old to be useful ever again or was
            // already saved in buffer.
            continue;
        }
        listWithoutUnnecessaryPackets.push_back(byteNum);
    }
    receiver->downloader->bufferMutex.unlock();
    return listWithoutUnnecessaryPackets;
}
