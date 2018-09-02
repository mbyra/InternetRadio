//
// Created by marcin on 02.09.18.
//

#include "err.h"

void err(const char *message) {
    std::cout << message << std::endl;
    exit(1);
}