cmake_minimum_required(VERSION 3.28)
project(hearts)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(SOURCES
        src/DealConfig.cpp
        src/ServerNetworkingConfig.cpp
        src/PlayerHandsConfig.cpp
        src/HandConfig.cpp
        src/ServerConfig.cpp
        src/Card.cpp
        src/Seat.cpp
        src/DealType.cpp
        src/Port.cpp
        src/Server.cpp
        src/PlayersManager.cpp
        src/Logger.cpp
        src/PlayerData.cpp
        src/Game.cpp
        src/TrickHistory.cpp
        include/TrickHistory.h
        src/Error.cpp
        include/Error.h
        src/Trick.cpp
        include/Trick.h
)

add_executable(kierki-serwer src/kierki-serwer.cpp ${SOURCES})
add_executable(kierki-klient src/kierki-klient.cpp ${SOURCES})
