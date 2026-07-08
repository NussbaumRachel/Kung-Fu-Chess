cmake_minimum_required(VERSION 3.15)
project(KungFuChess VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# קבצי מקור
set(SOURCES
    src/main.cpp
    src/types.cpp
    src/Board.cpp
    src/BoardParser.cpp
    src/Game.cpp
)

set(HEADERS
    src/types.hpp
    src/Board.hpp
    src/BoardParser.hpp
    src/Game.hpp
)

add_executable(kungfuchess ${SOURCES} ${HEADERS})

# תיקיית include
target_include_directories(kungfuchess PRIVATE ${CMAKE_SOURCE_DIR}/src)
