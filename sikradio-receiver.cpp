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
#include "parsers.h"



int main(int argc, char **argv) {
    parseReceiver(argc, argv);

    Receiver receiver;
    receiver.start();

    return 0;
}



