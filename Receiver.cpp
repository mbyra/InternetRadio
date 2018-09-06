//
// Created by marcin on 02.09.18.
//

#include <thread>
#include "Receiver.h"
#include "err.h"


void Receiver::start() {
    debug("Receiver : start() : beginning");
    initializeServices();
    debug("Receiver : start() : after socket initialization");

    // Runs all services in separate threads:
//    std::thread menuAgentThread = std::thread([this]() {menu->start(); });
//    menuAgentThread.detach();


    std::thread retransmissionRequesterThread = std::thread([this]()
            {requester->start(); });
    retransmissionRequesterThread.detach();
    debug("Receiver : start() : retransmissionRequester detached");

//    std::thread dataDownloaderThread = std::thread([this]()
//            {downloader.start(); });
//    dataDownloaderThread.detach();

    std::thread stationFinderThread = std::thread([this]() {finder->start(); });
    stationFinderThread.detach();
    debug("Receiver : start() : stationFinder detached");

    // StationFinder is run in this thread:
    debug("Receiver : start() : starting menu in this thread");
    menu->start();
    debug("Receiver : start() : after exiting menu service");
}

void Receiver::startDownloadingData() {
    debug("Receiver : start() : starting dataDownlaoder");
    std::thread dataDownloaderThread = std::thread([this]()
            {downloader->start(); });
    dataDownloaderThread.detach();
    debug("Receiver : start() : dataDownloader detached");

}

void Receiver::initializeServices() {
    menu = new MenuAgent(this);
    requester = new RetransmissionRequester(this);
    finder = new StationFinder(this);
    downloader = new DataDownloader(this);
}

Receiver::~Receiver() {
    delete menu;
    delete requester;
    delete finder;
    delete downloader;
}
