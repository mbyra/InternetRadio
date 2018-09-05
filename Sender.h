//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_SENDER_H
#define INTERNETRADIOSIK_SENDER_H

class Transmitter;

class Sender {
    friend class Transmitter;

public:
    Sender(Transmitter* transmitter) : transmitter(transmitter) {};

private:
    Transmitter* transmitter;

    void start();
};


#endif //INTERNETRADIOSIK_SENDER_H
