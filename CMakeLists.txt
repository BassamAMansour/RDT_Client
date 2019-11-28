cmake_minimum_required(VERSION 3.15)
project(RDT_Client)

set(CMAKE_CXX_STANDARD 17)

add_executable(RDT_Client main.cpp PacketUtils.cpp PacketUtils.h Packets.h)