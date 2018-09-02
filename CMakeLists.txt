cmake_minimum_required(VERSION 3.12)
project(InternetRadioSIK)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-Wall -O2")

find_package (Threads)
find_library(LIBRT rt)

add_executable(radio_receiver receiver.cpp err.cpp err.h parameters.cpp parameters.h Buffer.cpp Buffer.h Menu.cpp Menu.h Transmitter.cpp Transmitter.h)
add_executable(radio_transmitter transmitter.cpp err.cpp err.h parameters.cpp parameters.h Transmitter.cpp Transmitter.h)

target_link_libraries (radio_receiver ${CMAKE_THREAD_LIBS_INIT} ${LIBRT})
target_link_libraries (radio_transmitter ${CMAKE_THREAD_LIBS_INIT} ${LIBRT})
