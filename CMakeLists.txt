cmake_minimum_required(VERSION 3.28)
project(hearts)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(SOURCES
        src/GameConfig.cpp
        src/ServerNetworkingConfig.cpp
        src/PlayerHandsConfig.cpp
        src/HandConfig.cpp
        src/ServerConfig.cpp
        src/Card.cpp
        src/Seat.cpp
        src/GameType.cpp
        src/Port.cpp
        include/Port.h
        src/Server.cpp
        include/Server.h
)

add_executable(kierki-serwer src/kierki-serwer.cpp ${SOURCES})
add_executable(kierki-klient src/kierki-klient.cpp ${SOURCES})
