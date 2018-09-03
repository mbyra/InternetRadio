//
// Created by marcin on 03.09.18.
//

#include <thread>
#include "Receiver.h"

void Receiver::start() {
    // Runs all services in separate threads:
    std::thread menuAgentThread = std::thread([this]() {menu.start(); });
    menuAgentThread.detach();


    std::thread retransmissionRequesterThread = std::thread([this]()
            {requester.start(); });
    retransmissionRequesterThread.detach();

//    std::thread dataDownloaderThread = std::thread([this]()
//            {downloader.start(); });
//    dataDownloaderThread.detach();

//    std::thread stationFinderThread = std::thread([this]() {finder.start(); });
//    stationFinderThread.detach();
    // StationFinder is run in this thread:
    finder.start();
}