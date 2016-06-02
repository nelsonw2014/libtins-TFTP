cmake_minimum_required(VERSION 3.0)
project(tins_TFTP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -ltins")

set(SOURCE_FILES tftp.cpp tftp.h)
add_library(tins_TFTP ${SOURCE_FILES})