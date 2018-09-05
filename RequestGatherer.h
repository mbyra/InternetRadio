//
// Created by marcin on 05.09.18.
//

#ifndef INTERNETRADIOSIK_REQUESTGATHERER_H
#define INTERNETRADIOSIK_REQUESTGATHERER_H


class Transmitter;

class RequestGatherer {
    friend class Transmitter;

public:
    RequestGatherer(Transmitter* transmitter) : transmitter(transmitter) {};

private:
    Transmitter* transmitter;

    void start();
};



#endif //INTERNETRADIOSIK_REQUESTGATHERER_H
