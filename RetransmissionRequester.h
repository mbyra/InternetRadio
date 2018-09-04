//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_RETRANSMISSIONREQUESTER_H
#define INTERNETRADIOSIK_RETRANSMISSIONREQUESTER_H

#include <chrono>
#include <tuple>
#include <list>
#include <mutex>
#include <set>
#include <sstream>

class Receiver;
class DataDownloader;

class RetransmissionRequester {

public:
    friend class Receiver;
    friend class DataDownloader;

private:
    Receiver* receiver;

    explicit RetransmissionRequester(Receiver *receiver) : receiver(receiver) {};

    // This container keeps pairs:
    // * startTime- time when first package with number greater than every
    // package number in the list 'packages' was received. Retransmission
    // request is sent in moments startTime + k*RTIME, when k is natural
    // number and RTIME is program parameter, until success
    // * packages - container keeping packages number which were ommited for
    // the first time at startTime
    using Time = std::chrono::time_point<std::chrono::system_clock>;
    using Request = std::pair<Time, std::list<uint64_t>>;
    std::list<Request> requests;
    std::mutex mut; // Ensures exclusive access to requests container.


    // Main method of class: starts service sending retransmission requests
    // every RTIME miliseconds.
    void start();

    // Method used to transform list of numbers (missing packages' first
    // bytes) in the list to form of comma-separated string, accepted by
    // transmitter. At the beggining of string there is RETRANSMIT_MESSAGE.
    std::string createRequestPackagesMessage(std::list<uint64_t> list);

    // Removes unnecessary packages from request list (for example, too old
    // to be useful or already retrieved.
    std::list<uint64_t> removeUnnecessaryPackages(std::list<uint64_t> list);
};


#endif //INTERNETRADIOSIK_RETRANSMISSIONREQUESTER_H
