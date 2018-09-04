//
// Created by marcin on 04.09.18.
//

#include <thread>
#include <cmath>

#include "RetransmissionRequester.h"
#include "parameters.h"

void RetransmissionRequester::start() {

    auto RTIME_ms = std::chrono::milliseconds(RTIME);
    using std::chrono::system_clock;

    std::chrono::ceil<std::chrono::milliseconds>(system_clock::now());

    while(true) {
        mut.lock();
        if(not requests.empty()) {
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

            // lock dataDownloader mutex
            // if (



        }

    }


}

void RetransmissionRequester::clearRequests() {

}
