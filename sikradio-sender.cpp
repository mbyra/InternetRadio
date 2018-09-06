//
// Created by marcin on 05.09.18.
//


#include <getopt.h>
#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <cassert>
#include <regex>
#include <iostream>
#include "parameters.h"
#include "err.h"
#include "Transmitter.h"
#include "parsers.h"


int main(int argc, char **argv) {
    parseSender(argc, argv);

    Transmitter transmitter;
    transmitter.start();

    return 0;
}
