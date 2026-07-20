cmake_minimum_required(VERSION 3.15)

project(KungFuChess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# ── קבצי מקור ──
set(SOURCE_ENTRY_POINT src/text_io/main.cpp)

set(SOURCE_MODEL
    src/model/Piece.cpp
    src/model/King.cpp
    src/model/Queen.cpp
    src/model/Rook.cpp
    src/model/Bishop.cpp
    src/model/Knight.cpp
    src/model/Pawn.cpp
    src/model/Board.cpp
)
set(SOURCE_CONFIG
    src/config/PieceSpeedConfig.cpp
    src/config/PieceConfigReader.cpp
)
set(SOURCE_MOVEMENT
    src/movement/Move.cpp
    src/movement/PieceFactory.cpp
)

set(SOURCE_RULE_ENGINE
    src/rule_engine/RuleEngine.cpp
)

set(SOURCE_ARBITER
    src/arbiter/RealTimeArbiter.cpp
)

set(SOURCE_GAME_ENGINE
    src/game_engine/GameStateMachine.cpp
    src/game_engine/BoardController.cpp
    src/game_engine/MoveCompletionService.cpp
    src/game_engine/GameEngine.cpp
    src/game_engine/ClickPreparationService.cpp
)

set(SOURCE_CONTROLLER
    src/controllerClick/GameController.cpp
)

set(SOURCE_CONFIG
    src/config/PieceSpeedConfig.cpp
    src/config/PieceConfigReader.cpp
)

set(SOURCE_TEXT_IO
    src/text_io/BoardParser.cpp
    src/text_io/TextTestRunner.cpp
)

# ── איחוד כל קבצי המקור (חוץ מהקובץ הראשי) ──
set(ALL_SOURCES
    ${SOURCE_MODEL}
    ${SOURCE_MOVEMENT}
    ${SOURCE_RULE_ENGINE}
    ${SOURCE_ARBITER}
    ${SOURCE_GAME_ENGINE}
    ${SOURCE_CONTROLLER}
    ${SOURCE_TEXT_IO}
    ${SOURCE_CONFIG}
)

# ── קובץ הבדיקות ──
set(TEST_SOURCES
    tests/PieceMovementTests.cpp
    tests/GameTests.cpp
    tests/GameOverTests.cpp
)

# ── תיקיית ה-include המשותפת ──
set(COMMON_INCLUDE_DIR src)

# ══════════════════════════════════════════
#  Binary: kungfuchess
# ══════════════════════════════════════════
add_executable(kungfuchess
    ${SOURCE_ENTRY_POINT}
    ${ALL_SOURCES}
)

target_include_directories(kungfuchess PRIVATE ${COMMON_INCLUDE_DIR})

# ══════════════════════════════════════════
#  Googletest (תלות חיצונית)
# ══════════════════════════════════════════
add_subdirectory(external/googletest-1.17.0)

# ══════════════════════════════════════════
#  Binary: chess_tests
# ══════════════════════════════════════════
add_executable(chess_tests
    ${TEST_SOURCES}
    ${ALL_SOURCES}
)

target_include_directories(chess_tests PRIVATE ${COMMON_INCLUDE_DIR})

target_link_libraries(chess_tests
    GTest::gtest_main
)

# ══════════════════════════════════════════
#  גילוי אוטומטי של בדיקות
# ══════════════════════════════════════════
message(STATUS "===== CHESS TESTS SECTION =====")

include(GoogleTest)
gtest_discover_tests(chess_tests)
add_test(NAME chess_tests COMMAND chess_tests)

# ══════════════════════════════════════════
#  תיקיית DEMO (אם נדרש)
# ══════════════════════════════════════════
add_subdirectory(DEMO)