//
// Created by marcin on 03.09.18.
//

#include <sstream>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cassert>

#include "MenuAgent.h"
#include "err.h"

// Slightly modified code from poll-server.c program from laboratories.
void MenuAgent::start() {
    initializeMenuSocket();

//    FOR TESTING:
//
//    sockaddr_in server;
//    server.sin_family = AF_INET;
//    server.sin_addr.s_addr = htonl(INADDR_ANY);
//    server.sin_port = htons(UI_PORT);
//    Station s("Stacja nr 1", "249.000.000.001", server, 17000);
//    receiver->stationList.push_back(s);
//
//    sockaddr_in server2;
//    server2.sin_family = AF_INET;
//    server2.sin_addr.s_addr = htonl(INADDR_ANY);
//    server2.sin_port = htons(UI_PORT);
//    Station s2("Stacja nr 2", "249.000.000.001", server2, 17000);
//    receiver->stationList.push_back(s2);
//
//    sockaddr_in server3;
//    server3.sin_family = AF_INET;
//    server3.sin_addr.s_addr = htonl(INADDR_ANY);
//    server3.sin_port = htons(UI_PORT);
//    Station s3("Stacja nr 3", "249.000.000.001", server3, 17000);
//    receiver->stationList.push_back(s3);

    /* Zapraszamy klientów */
    if (listen(client[0].fd, 5) == -1) {
        perror("Starting to listen");
        exit(EXIT_FAILURE);
    }

    int activeClients = 0;

    do {
        if(not receiver->stationIsSet)
            setFirstStationAsCurrent();

        for (int i = 0; i < MAX_OPEN_SOCKETS; ++i)
            client[i].revents = 0;

        /* Czekamy przez POLL_INTERVAL ms */
        int ret = poll(client, MAX_OPEN_SOCKETS, POLL_INTERVAL);

        if (ret < 0)
            perror("poll");
        else if (ret >= 0) {
            if (client[0].revents & POLLIN) {
                int msgsock = accept(client[0].fd, (struct sockaddr*)0, (socklen_t*)0);
                if (msgsock == -1)
                    perror("accept");
                else {
                    for (int i = 1; i < MAX_OPEN_SOCKETS; ++i) {
                        if (client[i].fd == -1) {
                            // client disconnected
                            clientMutex.lock();
                            client[i].fd = msgsock;
                            clientMutex.unlock();
                            activeClients += 1;
                            clientState[i] = START;
                            telnetSendInitialSettings(msgsock);
                            telnetSendClearScreen(msgsock);
                            telnetSendMenu(msgsock);
                            break;
                        }
                    }
                }
            }
            for (int i = 1; i < MAX_OPEN_SOCKETS; ++i) {
                if (client[i].fd != -1
                    && (client[i].revents & (POLLIN | POLLERR))) {
                    char buf[1];
                    auto rval = read(client[i].fd, buf, 1);
                    if (rval <= 0) { //TODO <0 or <=0 ? -> I want 0
                        // Client disconnected
                        if (close(client[i].fd) < 0)
                            perror("close");
                        clientMutex.lock();
                        client[i].fd = -1;
                        clientMutex.unlock();
                        activeClients -= 1;
                    }
                    else {
                        executeClientCommand(i, buf[0]);
                        if(clientState[i] == UP) {
                            changeCurrentStation(UP);
                            refreshClientsMenu();
                            clientState[i] = START;
                        } else if (clientState[i] == DOWN) {
                            changeCurrentStation(DOWN);
                            refreshClientsMenu();
                            clientState[i] = START;
                        }
                    }
                }
            }
        }
        else
            fprintf(stderr, "Do something else\n");

    } while(true);
}

void MenuAgent::executeClientCommand(int no, char ch) {
    switch(ch) {
        case '\033':
            clientState[no] = HALFWAY;
            break;
        case '[':
            clientState[no] == HALFWAY
                            ? clientState[no] = ESCAPED
                            : clientState[no] = START;
            break;
        case 'A':
            clientState[no] == ESCAPED
                            ? clientState[no] = UP
                            : clientState[no] = START;
            break;
        case 'B':
            clientState[no] == ESCAPED
                            ? clientState[no] = DOWN
                            : clientState[no] = START;
            break;
        default:
            break;
    }
}

MenuAgent::MenuAgent(Receiver *receiver) : receiver(receiver) {
    int i;
    /* Inicjujemy tablicę z gniazdkami klientów, client[0] to gniazdko centrali */
    // -> więc inicjuję od 1, bo centralę ogarniam gdzie indziej.
    for (i = 0; i < MAX_OPEN_SOCKETS; ++i) {
        client[i].fd = -1;
        client[i].events = POLLIN;
        client[i].revents = 0;
    }
//    activeClients = 0;
}

MenuAgent::~MenuAgent() {
    for (int i = 1; i < MAX_OPEN_SOCKETS; ++i) {
        if (client[i].fd != -1)
            close(client[i].fd);
    }
}

void MenuAgent::initializeMenuSocket() {
    // Based on poll-server.c
    struct sockaddr_in server;

    /* Tworzymy gniazdko centrali */
    client[0].fd = socket(PF_INET, SOCK_STREAM, 0);
    if (client[0].fd < 0) {
        perror("Opening stream socket");
        exit(EXIT_FAILURE);
    }

    // https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr
    int enable = 1;
    if (setsockopt(client[0].fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        syserr("setsockopt(SO_REUSEADDR) failed");

    /* Co do adresu nie jesteśmy wybredni */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(UI_PORT);
    if (bind(client[0].fd, (struct sockaddr*)&server,
             (socklen_t)sizeof(server)) < 0) {
        perror("Binding stream socket");
        exit(EXIT_FAILURE);
    }

}

void MenuAgent::telnetSendMenu(int sock) {

    std::stringstream ss; // I will print all chunks of text into this ss.

    // heading:
    ss <<
       "------------------------------------------------------------------------\r\n"
       "  SIK Radio\r\n"
       "------------------------------------------------------------------------\r\n";

    // stations with indicator:
//    debug("MenuAgent : telnetSendMenu : LOCKING receiver->mut");
    receiver->controlMutex.lock();
    receiver->stationListMutex.lock();

    // If there is no currently chosen station, choose the first one.
    if(not receiver->stationIsSet)
        setFirstStationAsCurrent();

    for (auto iter = receiver->stationList.begin();
                                iter != receiver->stationList.end(); iter++) {
        if (iter == receiver->currentStation)
            ss << "  > " << iter->stationName << "\r\n";
        else
            ss << "    " << iter->stationName << "\r\n";
    }
    receiver->controlMutex.unlock();
    receiver->stationListMutex.unlock();
//    debug("MenuAgent : telnetSendMenu : UNLOCKED receiver->mut");

    // bottom:
    ss << "------------------------------------------------------------------------\r\n";

    // Send all string to client's telnet socket:
    std::string text = ss.str();
    if (text.length() != write(sock, text.c_str(), text.length())) {
//        std::cerr << "Could not send menu to client on socket " << sock << std::endl;
    }

}

void MenuAgent::telnetSendClearScreen(int sock) {
    char clear[32] = "\033[2J\033[0;0H"; // clear terminal, move to left upper corner
    if(32 != write(sock, clear, 32)) {
//        std::cerr << "Could not send clear screen to client on socket " << sock <<std::endl;
    }
}

void MenuAgent::telnetSendInitialSettings(int sock) {
    char iacDoLinemode[3] = {(char)255, (char)253, (char)34};
    char iacSbLinemodeMode0IacSe[7] = {(char)255, (char)250, (char)34, (char)
                                       1, (char)0, (char) 255, (char)240};
    char iacWillEcho[3] = {(char)255, (char)251, (char)1};

    if (3 != write(sock, iacDoLinemode, 3)
        or 7 != write(sock, iacSbLinemodeMode0IacSe, 7)
        or 3!= write(sock, iacWillEcho, 3)) {
//        std::cerr << "Could not send initial command to cliend on socket " <<
//                  sock << std::endl;
    }
}

void MenuAgent::refreshClientsMenu() {
    for (int i = 1; i < MAX_OPEN_SOCKETS; i++) {
        if (client[i].fd != -1) {
            telnetSendClearScreen(client[i].fd);
            telnetSendMenu(client[i].fd);
        }
    }
}

void MenuAgent::changeCurrentStation(State cmd) {
    receiver->controlMutex.lock();
    receiver->stationListMutex.lock();

//    debug("MenuAgent : changecurrentStation : locked mutexes");

    // If current station is not selected try to set first station from list
    // as current.
    if(not receiver->stationIsSet) {
//        debug("MenuAgent : changecurrentStation : station was not set, "
//              "setting");
        setFirstStationAsCurrent();
    }

    // It is possible to change the station only if stationList is longer than 1
    if (receiver->stationList.size() > 1) {
        // iterate over stations to find current station
        for (auto iter = receiver->stationList.begin();
                                iter != receiver->stationList.end(); iter++) {
            if (iter == receiver->currentStation) {
//                debug("MenuAgent : changecurrentStation : found current "
//                      "station, it is %s",
//                      receiver->currentStation->stationName.c_str());

                switch (cmd) {
                    case UP:
                        if (iter->stationName != receiver->stationList.begin()->stationName)
                            receiver->currentStation = --iter;
                        break;
                    case DOWN:
                        if (iter->stationName != receiver->stationList.rbegin()->stationName)
                            receiver->currentStation = ++iter;
                        break;
                    default:
                        assert(true);
                }
//                debug("MenuAgent : changecurrentStation : changed current "
//                      "station, now it is %s",
//                      receiver->currentStation->stationName.c_str());
//                receiver->state = STATION_CHANGED;
                break;

            }
        }
    }

    receiver->controlMutex.unlock();
    receiver->stationListMutex.unlock();
//    debug("MenuAgent : changecurrentStation : unlocked mutexes");

}

bool MenuAgent::setFirstStationAsCurrent() {
    if(not receiver->stationList.empty()) {
        receiver->currentStation = receiver->stationList.begin();
        receiver->stationIsSet = true;
        return true;
    }
    return false;
}
