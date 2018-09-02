cmake_minimum_required(VERSION 3.12)
project(InternetRadioSIK)

set(CMAKE_CXX_STANDARD 17)

add_executable(radio_receiver receiver.cpp err.cpp err.h)
add_executable(radio_transmitter transmitter.cpp err.cpp err.h)