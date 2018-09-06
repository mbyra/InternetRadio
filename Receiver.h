//
// Created by marcin on 02.09.18.
//

#ifndef INTERNETRADIOSIK_RECEIVER_H
#define INTERNETRADIOSIK_RECEIVER_H

#include <vector>
#include <mutex>
#include "Station.h"
#include "MenuAgent.h"
#include "StationFinder.h"
#include "DataDownloader.h"
#include "RetransmissionRequester.h"
#include "parameters.h" // DISCOVER_ADDR, CTRL_PORT, UI_PORT etc.

// All classes of receiver services defined in separate files:
class MenuAgent;
class RetransmissionRequester;
class StationFinder;
class DataDownloader;


// DataDownloader must know in manny places what to do: play on, reset etc.
enum ReceiverState {
    STANDARD,
    STATION_CHANGED,
    STATION_NOT_SET,
};

class Receiver {
    friend class MenuAgent;
    friend class RetransmissionRequester;
    friend class StationFinder;
    friend class DataDownloader;

public:
    // Main function of the receiver: initializes all four services.
    void start();
    ~Receiver();


private:
    // Here we declare all necessary variables and create class objects
    // responsible for fetching station list, data to buffer, displaying user
    // interface and asking for retransmission. All objects will run in their
    // own threads.

    // Vector in which objects representing stations (transmitters) are stored:
    std::list<Station> stationList;
    std::mutex stationListMutex;

    // Pointer to current
    std::list<Station, std::allocator<Station>>::iterator currentStation;
    bool stationIsSet = false;
    bool isPlayingNow = false; // toggle play on/off
    std::mutex controlMutex; // guards stationList, currentStation, isPlayingNow //

    ReceiverState state = STATION_NOT_SET;


    // Object providing menu displaying and controlling:
    MenuAgent *menu;

    // Object prividing requesting for available stations in network:
    StationFinder *finder;

    // Object providing requesting for retransmission:
    RetransmissionRequester *requester;

    // Object downloading data from transmitter:
    DataDownloader *downloader;

    // Starts DataDownloader service.
    void startDownloadingData();

    // Creates services objects.
    void initializeServices();


};

#endif //INTERNETRADIOSIK_RECEIVER_H
