//
// Created by marcin on 03.09.18.
//

#ifndef INTERNETRADIOSIK_DATADOWNLOADER_H
#define INTERNETRADIOSIK_DATADOWNLOADER_H

#include "Receiver.h"
#include "AudioPackage.h"
#include "RetransmissionRequester.h"
#include <string>
#include <mutex>


class Receiver;
class RetransmissionRequester;

class DataDownloader {
    friend class Receiver;
    friend class RetransmissionRequester;

private:
    Receiver *receiver;

    // Variables below tell the downloader from where to download packets.
    // They are usually the same as in the receiver's current station. However,
    // when e.g. current station is changed or not set, these variables here
    // and in receiver are different. When DataDownloader detects it, it
    // triggers resetAll() function.
    std::string currMcastAddress = "";
    int currSock = -1; // -1 means unset
    int currTransmitterPort = -1; // -1 means unset
    std::mutex transmitterParamsMutex;

    // Buffer implemented as map to have easy access to Packages by their
    // firstByteNum
    std::map<uint64_t, AudioPackage> buffer;
    uint64_t currByteZero = 0;
    uint64_t currSesionId = 0; // The same during transmission from receiver
    bool initializedTransmission = false;
    std::mutex bufferMutex;

    uint64_t playbackID = 0; // incremented every time something goes wrong
    bool isPlaybackValid = false; // Flag checked before continuing to play

    explicit DataDownloader(Receiver *receiver) : receiver(receiver) {};

    // Unlike socket initialize methods in other main classes of program,
    // this can be triggered many times, because transmitter address or port
    // can change during lifetime of program.
    // Method restarts buffers here and in RetransmissionRequester and sets
    // isPlayingNow in Receiver to true. If socket changed, it is reinitialized,
    // currTransmitterPort and currMcastAddress are set to match those in
    // receiver->currentStation.
    void restartDownloader();


    // Main method of class. Runs fetching data in infinite loop. Plays data
    // from currently choosen station. Changes station and reinitializes if
    // needed. Delegates retransmission tasks jobs to RetransmissionRequester.
    // Method is stopped only if there is no longer currentStation in
    // receiver set. For example, the signal is lost. (However, I run some
    // automation and new station is automatically set, but not here).
    void start();

    // Returns firstByte of first missing package. Package is missing if
    // firstByte of last package in buffer minus PSIZE is not equal to
    // firstByt of previous package in buffer.
    // The method also checks if missing byte is not too old(would not fit
    // into buffer), then returns firstByte of oldest possible missing
    // package instead.
    uint64_t oldestPossibleMissingFirstByte(AudioPackage lastPackage);

    // Let name speak for itself.
    void addRetransmissionRequestOfMissingPackages(AudioPackage newest);

    // Erase from buffer packages older than last [BSIZE/PSIZE],
    // that is, make the buffer keep as many packages as
    // specification says, not implementation.
    void eraseOldPackagesFromBuffer(AudioPackage current,
            ssize_t audioDataLength);

    // Starts 'playing' to stdout until encounters first missing byte.
    void play(uint64_t firstByteToPlay, uint64_t pbID);

    // Conversion (bytes in network order, uint64_t in host order)
    uint64_t bytesToUint64(std::string);
public:

};


#endif //INTERNETRADIOSIK_DATADOWNLOADER_H
