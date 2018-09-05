cmake_minimum_required(VERSION 3.12)
project(InternetRadioSIK)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-Wall -O2")

find_package (Threads)
find_library(LIBRT rt)

add_executable(radio_receiver sikradio-receiver.cpp err.cpp err.h parameters.cpp parameters.h MenuAgent.cpp MenuAgent.h Transmitter.h Station.h AudioPackage.h StationFinder.cpp StationFinder.h RetransmissionRequester.cpp RetransmissionRequester.h Receiver.cpp Receiver.h DataDownloader.cpp DataDownloader.h )
add_executable(radio_transmitter sikradio-sender.cpp err.cpp err.h parameters.cpp parameters.h Transmitter.h Sender.cpp Sender.h RequestGatherer.cpp RequestGatherer.h RetransmissionSender.cpp RetransmissionSender.h)

target_link_libraries (radio_receiver ${CMAKE_THREAD_LIBS_INIT} ${LIBRT})
target_link_libraries (radio_transmitter ${CMAKE_THREAD_LIBS_INIT} ${LIBRT})
