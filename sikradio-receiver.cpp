#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include <pthread.h>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <ctime>
#include <mutex>

#include "err.h"
#include "parameters.h"
#include "Receiver.h"


// ***** Global variables *********************
MenuAgent menu(); // shared accross clients

// ********************************************


// ***** Parsing and diagnostic functions *****

// Prints diagnostic data to stdout.
void diagnose() {
    std::cout << "All important constants after parse:" << std::endl <<
              "DISCOVER_ADDR: " << DISCOVER_ADDR << std::endl <<
              "CTRL_PORT: " << CTRL_PORT << std::endl <<
              "UI_PORT: " << UI_PORT << std::endl <<
              "BSIZE: " << BSIZE << std::endl <<
              "RTIME: " << RTIME << std::endl <<
              "NAME : " << NAME << std::endl;
}

// Checks if string contains unsigned integer. If so, returns it.
// Otherwise, prints error message and exits with code 1.
unsigned parse_unsigned(const char* s) {
    if(!std::regex_match(optarg, std::regex("[0-9]+")))
        fatal("Not a positive number in argument");

    long number = strtol(optarg, nullptr, 10);

    return static_cast<unsigned int>(number);
}

// Checks if string contains unsigned 16 bit integer. If so, returns it.
// Otherwise, prints error message and exits with code 1.
unsigned parse_unsigned_16_bit(const char* s) {
    unsigned number = parse_unsigned(s);

    if (number >> 16)
        fatal("Not a unsigned 16 bit in argument");

    return number;
}


// Parses all arguments from cli and saves them into global variables. Some
// variables have default value.
void parse(int argc, char **argv) {
    int c;

    if (argc < 2) {
        printf("Usage %s [optional arguments]\n"
               "List of all possible flags [default value]):\n"
               "\t-d discover address [255.255.255.255]\n"
               "\t-C control port [37056]\n"
               "\t-U user interface port [17056]\n"
               "\t-b buffer size in bytes [65536]\n"
               "\t-R retransmission time in ms [250]\n"
               "\t-n default transmitter (station) name [random]\n"
                , argv[0]);
    }

    while((c = getopt(argc, argv, "d:C:U:b:R:n:")) != -1) {
        switch (c) {
            case 'd':
                // Discover address used by receiver to find transmitters.
                struct sockaddr_in sa;
                if (inet_pton(AF_INET, optarg, &(sa.sin_addr)) != 1)
                    fatal("Invalid discover address. It should be proper ipv4 "
                        "address.");
                DISCOVER_ADDR = std::string{optarg};
                break;
            case 'C':
                // UDP port used to transmit control packets.
                CTRL_PORT = static_cast<in_port_t>(parse_unsigned_16_bit(optarg));
                break;
            case 'U':
                // TCP port with user interface of radio receiver.
                UI_PORT = static_cast<in_port_t>(parse_unsigned_16_bit(optarg));
                break;
            case 'b':
                // Size (in bytes) of buffer.
                BSIZE = parse_unsigned(optarg);
                break;
            case 'R':
                // Time (ms) of retransmission of raports about missing packets.
                RTIME = parse_unsigned(optarg);
                break;
            case 'n':
                // Name of transmitter.
                if (strlen(optarg) > MAX_STATION_NAME) {
                    char message [50];
                    sprintf(message, "Max station length is %d.\n",
                            MAX_STATION_NAME);
                    fatal(message);
                }
                NAME = optarg;
                break;
            default:
                assert(true);
        }
    }

    diagnose();
}

// ********************************************


// ***** Receiver functions *******************

// Creates udp socket used for scanning for transmitters. Sets options and
// binds the socket.
// Based on scenario 09 from labs - multi-send.c
int init_ctrl_socket() {

    /* zmienne i struktury opisujące gniazda */
    int sock, optval;
    struct sockaddr_in local_address;

    /* otworzenie gniazda */
    sock = socket(AF_INET, SOCK_DGRAM, 0); //TODO trzeba potem zamknąć!
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

    /* zablokowanie rozsyłania grupowego do siebie */
    optval = 0;
    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval,
            sizeof optval) < 0)
        syserr("setsockopt loop");

    /* podpięcie się pod lokalny adres i port */
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(0);
    if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0)
        syserr("bind");

    return sock;
}

void* ui_thread_func(void* useless) {

    return nullptr;
}

void* buff_write_thread_func(void* useless) {

    return nullptr;
}

void* buff_read_thread_func(void* useless) {

    return nullptr;
}

// Function triggered when timer expires: request rescan of available stations
void rescan(union sigval arg) {

}

void set_process_timer(int &ctrl_socket) {
    timer_t timer;

    // Create object which specifiec timer behavior (not event it triggers):
    struct itimerspec itimerspec;
    itimerspec.it_value.tv_sec = 0;
    itimerspec.it_value.tv_nsec = 200 * 1000000; // wait 0.2s for everything to be set up
    itimerspec.it_interval.tv_sec = RTIME/1000; // RTIME is in ms
    itimerspec.it_interval.tv_nsec = (RTIME % 1000) * 1000000; // 1ms * 10^6 = 1ns

    // Create event object which specifies what should be done when timer expires:
    // Based on: http://man7.org/tlpi/code/online/diff/timers/ptmr_sigev_thread.c.html
    struct sigevent sev;
    sev.sigev_notify = SIGEV_THREAD; // invoke sigev_notify_function when timer expires
    sev.sigev_notify_function = rescan;
    sev.sigev_notify_attributes = nullptr;
    // sigev_value is passed as argument to sigev_notify_function
    sev.sigev_value.sival_int = 0;
    sev.sigev_value.sival_ptr = (void*)&ctrl_socket;

    // Create and set the timer itself:
    if (timer_create(CLOCK_REALTIME, &sev, &timer) != 0)
        syserr("Error when creating timer.");
    if (timer_settime(timer, 0, &itimerspec, nullptr) != 0)
        syserr("Error when setting timer specification");

}

// Main function of radio receiver: plays currently choosen (in menu) radio
// station.
void play() {

}

// ********************************************




int main(int argc, char **argv) {
    parse(argc, argv);

    Receiver receiver;
    receiver.start();



//    Buffer buffer; // IO buffer handling reading and writing data from socket
//    MenuAgent menu; // "MenuAgent" displayed in ui, knowing list of stations (and current)
//
//    // Create socket for scanning for transmitters, set options and bind:
//    int ctrl_socket = init_ctrl_socket();
//
//    // Create three services:
//    pthread_t threads[3];
//    // 1. Takes care of user interface:
//    pthread_create(&threads[0], nullptr, ui_thread_func, nullptr);
//    // 2. Writes to buffer:
//    pthread_create(&threads[1], nullptr, buff_write_thread_func, nullptr);
//    // 3. Reads from buffer:
//    pthread_create(&threads[2], nullptr, buff_read_thread_func, nullptr);
//
//    // Create inter process' interval timer used e.g. for retransmission or ui
//    // refresh and set appropiate events and timer specification:
//    set_process_timer(ctrl_socket);
//
//
//    // Start playing radio according to currently choosen station.
//    play();
//
//
//    close (ctrl_socket);

    return 0;
}



