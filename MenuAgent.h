//
// Created by marcin on 03.09.18.
//

#ifndef INTERNETRADIOSIK_MENU_H
#define INTERNETRADIOSIK_MENU_H

#include <vector>
#include <string>
#include <map>
#include <poll.h>
#include "Transmitter.h"
#include "Receiver.h"
#include "parameters.h"

// Menu agent is based on polling example from 7th scenario from laboratories
// (poll-server.c):
// single process managing ('polling') all open sockets.
// Some fragments (regarding initialization and closing of sockets and other
// formal, repetitive stuff are taken from cited example laboratory program.

class Receiver;

class MenuAgent {
    friend class Receiver;
    friend class StationFinder;

private:
    Receiver* receiver;

    pollfd client[MAX_OPEN_SOCKETS];
    std::mutex clientMutex;

    // Change client state (during 'reading' his command) according to character
    // Set of enum, vector and method is equivalent to simple state automaton.
    enum State {
        START, // when deflects from START-HALFWAY-ESCAPED-UP/DOWN or after all sequence
        HALFWAY, // after reading '\033'
        ESCAPED, // after reading '['
        UP, // after reading 'A'
        DOWN, // after reading 'B"
    };
    std::vector<State> clientState = std::vector<State>(MAX_OPEN_SOCKETS);
    void executeClientCommand(int no, char ch);

    explicit MenuAgent(Receiver *receiver);
    ~MenuAgent();

    // Main function of class: starts initialization and runs UI service.
    void start();

    // Initialize, set options, addresses and bind.
    void initializeMenuSocket();

    // Also set current station to first available if not defined.
    void changeCurrentStation(State cmd);

    // Functions below assume you have clientMutex locked outside them:

    // Write's to all clients' telnets refreshed menu list.
    void refreshClientsMenu();
    // Write to client's telnet clear screen command.
    void telnetSendClearScreen(int sock);

    // Initial telnet commands sent to clients: set linemode, echo etc.
    void telnetSendInitialSettings(int sock);

    // Display menu to client's telnet (identified by socket).
    void telnetSendMenu(int sock);

    // If stationList is not empty, sets first element of stationList as
    // currentStation and returns true.
    // Otherwise, returns false.
    bool setFirstStationAsCurrent();
};




#endif //INTERNETRADIOSIK_MENU_H
