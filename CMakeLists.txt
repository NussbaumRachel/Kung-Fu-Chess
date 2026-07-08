cmake_minimum_required(VERSION 3.15)

project(KungFuChess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(kungfuchess
    src/main.cpp
    src/Piece.cpp
    src/King.cpp
    src/Queen.cpp
    src/Rook.cpp
    src/Bishop.cpp
    src/Knight.cpp
    src/Pawn.cpp
    src/Board.cpp
    src/BoardParser.cpp
    src/Game.cpp
)

target_include_directories(kungfuchess PRIVATE src)

add_subdirectory(external/googletest-1.17.0)

add_executable(chess_tests
    tests/PieceMovementTests.cpp
    src/Piece.cpp
    src/King.cpp
    src/Queen.cpp
    src/Rook.cpp
    src/Bishop.cpp
    src/Knight.cpp
    src/Pawn.cpp
    src/Board.cpp
    src/BoardParser.cpp
    src/Game.cpp
)

target_include_directories(chess_tests PRIVATE src)

target_link_libraries(chess_tests
    GTest::gtest_main
)
message(STATUS "===== CHESS TESTS SECTION =====")
include(GoogleTest)
gtest_discover_tests(chess_tests)
add_test(NAME chess_tests COMMAND chess_tests)