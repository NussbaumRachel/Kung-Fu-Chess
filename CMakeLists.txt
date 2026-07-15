cmake_minimum_required(VERSION 3.15)

project(KungFuChess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(kungfuchess
    src/text_io/main.cpp
    src/model/Piece.cpp
    src/model/King.cpp
    src/model/Queen.cpp
    src/model/Rook.cpp
    src/model/Bishop.cpp
    src/model/Knight.cpp
    src/model/Pawn.cpp
    src/model/Board.cpp
    src/movement/Move.cpp
    src/movement/PieceFactory.cpp
    src/rule_engine/RuleEngine.cpp
    src/arbiter/RealTimeArbiter.cpp
    src/game_engine/GameStateMachine.cpp
    src/game_engine/BoardController.cpp
    src/game_engine/MoveCompletionService.cpp
    src/game_engine/GameEngine.cpp
    src/controllerClick/GameController.cpp
    src/text_io/BoardParser.cpp
    src/text_io/TextTestRunner.cpp

)

target_include_directories(kungfuchess PRIVATE src)

add_subdirectory(external/googletest-1.17.0)

add_executable(chess_tests
    tests/PieceMovementTests.cpp
    tests/GameTests.cpp
    tests/GameOverTests.cpp
    src/model/Piece.cpp
    src/model/King.cpp
    src/model/Queen.cpp
    src/model/Rook.cpp
    src/model/Bishop.cpp
    src/model/Knight.cpp
    src/model/Pawn.cpp
    src/model/Board.cpp
    src/movement/Move.cpp
    src/movement/PieceFactory.cpp
    src/rule_engine/RuleEngine.cpp
    src/arbiter/RealTimeArbiter.cpp
    src/game_engine/GameStateMachine.cpp
    src/game_engine/BoardController.cpp
    src/game_engine/MoveCompletionService.cpp
    src/game_engine/GameEngine.cpp
    src/controllerClick/GameController.cpp
    src/text_io/BoardParser.cpp
    src/text_io/TextTestRunner.cpp
)

target_include_directories(chess_tests PRIVATE src)

target_link_libraries(chess_tests
    GTest::gtest_main
)
message(STATUS "===== CHESS TESTS SECTION =====")
include(GoogleTest)
gtest_discover_tests(chess_tests)
add_test(NAME chess_tests COMMAND chess_tests)
