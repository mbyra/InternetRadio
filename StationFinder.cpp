#include "StationFinder.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>

#include "err.h"
#include "parameters.h"
#include <fcntl.h>

StationFinder::~StationFinder() {
    close(sock);
}

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
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_port = htons(CTRL_PORT);
    if (inet_aton(DISCOVER_ADDR.c_str(), &remoteAddress.sin_addr) == 0)
        syserr("inet_aton");

//    if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0)
//        syserr("bind");

//    return sock;

}


//
void StationFinder::searchStationService() {


}

void StationFinder::replyParserService() {

}

void StationFinder::start() {
    initCtrlSocket();

    // run searchStation service in separate thread
    std::thread searchStationServiceThread([this]() {searchStationService(); });
    searchStationServiceThread.detach();

    // in current thread run replyParser service
    replyParserService();
}
