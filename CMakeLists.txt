cmake_minimum_required(VERSION 3.15)

project(KungFuChess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

find_package(Boost CONFIG REQUIRED COMPONENTS system)

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
    pkg_check_modules(HIREDIS REQUIRED IMPORTED_TARGET hiredis)
endif()

enable_testing()

# ── Source files ──
set(SOURCE_ENTRY_POINT
    src/text_io/main.cpp
)

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
    src/arbiter/CollisionResolver.cpp
)

set(SOURCE_GAME_ENGINE
    src/game_engine/GameStateMachine.cpp
    src/game_engine/BoardController.cpp
    src/game_engine/MoveCompletionService.cpp
    src/game_engine/GameEngine.cpp
    src/game_engine/ClickPreparationService.cpp
    src/game_engine/ScoreTracker.cpp
    src/game_engine/MoveRecorder.cpp
    src/game_engine/CooldownService.cpp
    src/game_engine/SnapshotBuilder.cpp
)

set(SOURCE_CONTROLLER
    src/controllerClick/GameController.cpp
)

set(SOURCE_GUI
    demo/img.cpp
    demo/AnimatedSprite.cpp
    demo/AnimationManager.cpp
    demo/AssetManager.cpp
    demo/ChessRenderer.cpp
    demo/BoardRenderer.cpp
    demo/PieceRenderer.cpp
    demo/MouseHandler.cpp
)

set(SOURCE_TEXT_IO
    src/text_io/BoardParser.cpp
    src/text_io/TextTestRunner.cpp
)

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

set(SOURCE_NETWORK
    src/network/JsonProtocol.cpp
    src/network/GameServer.cpp
    src/network/ClientSession.cpp
    src/network/SessionManager.cpp
    src/network/MessageRouter.cpp
    src/network/GameLoop.cpp
    src/network/Room.cpp
    src/network/RoomManager.cpp
)

set(SOURCE_SERVER_INFRASTRUCTURE
    src/infrastructure/memory/InMemoryUserRepository.cpp
    src/infrastructure/memory/InMemoryGameResultRepository.cpp
    src/infrastructure/memory/InMemorySessionStore.cpp
)

if(UNIX)
    list(APPEND SOURCE_SERVER_INFRASTRUCTURE
        src/infrastructure/postgres/PostgresConnectionConfig.cpp
        src/infrastructure/postgres/PostgresUserRepository.cpp
        src/infrastructure/postgres/PostgresGameResultRepository.cpp
        src/infrastructure/redis/RedisConnectionConfig.cpp
        src/infrastructure/redis/RedisSessionStore.cpp
    )
endif()

set(SOURCE_NETWORK_CLIENT
    src/network/NetworkClient.cpp
    src/network/ClientMessageProcessor.cpp
)

set(TEST_SOURCES
    tests/PieceMovementTests.cpp
    tests/GameTests.cpp
    tests/GameOverTests.cpp
)

set(COMMON_INCLUDE_DIR
    src
)

# ── Local header-only nlohmann/json target ──
add_library(nlohmann_json INTERFACE)

target_include_directories(
    nlohmann_json
    INTERFACE
        ${CMAKE_SOURCE_DIR}/external
)

add_library(
    nlohmann_json::nlohmann_json
    ALIAS
    nlohmann_json
)

# ── Text application ──
add_executable(
    kungfuchess
    ${SOURCE_ENTRY_POINT}
    ${ALL_SOURCES}
)

target_compile_features(
    kungfuchess
    PRIVATE
        cxx_std_17
)

target_include_directories(
    kungfuchess
    PRIVATE
        ${COMMON_INCLUDE_DIR}
)

# ── Game server: Linux/Docker only ──
if(UNIX)
    add_executable(
        kungfuchess_server
        src/network/network_main_server.cpp
        ${ALL_SOURCES}
        ${SOURCE_NETWORK}
        ${SOURCE_SERVER_INFRASTRUCTURE}
    )

    target_compile_features(
        kungfuchess_server
        PRIVATE
            cxx_std_17
    )

    target_include_directories(
        kungfuchess_server
        PRIVATE
            ${COMMON_INCLUDE_DIR}
    )

    target_link_libraries(
        kungfuchess_server
        PRIVATE
            Boost::system
            nlohmann_json::nlohmann_json
            PkgConfig::PQXX
            PkgConfig::HIREDIS
    )
endif()

# ── Network client ──
add_executable(
    kungfuchess_client
    src/network/network_main_client.cpp
    ${ALL_SOURCES}
    ${SOURCE_NETWORK}
    ${SOURCE_NETWORK_CLIENT}
    ${SOURCE_GUI}
)

target_compile_features(
    kungfuchess_client
    PRIVATE
        cxx_std_17
)

target_include_directories(
    kungfuchess_client
    PRIVATE
        ${COMMON_INCLUDE_DIR}
)

target_link_libraries(
    kungfuchess_client
    PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
)

# ── OpenCV GUI support for the network client ──
set(
    OPENCV_DIR
    "${CMAKE_SOURCE_DIR}/OpenCV_451"
    CACHE PATH
    "Path to OpenCV 4.5.1 installation"
)

set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
set(OPENCV_LIB_DIR "${OPENCV_DIR}/bin")

if(EXISTS "${OPENCV_INCLUDE_DIR}/opencv2/core.hpp")
    find_package(OpenCV QUIET)

    if(NOT OpenCV_FOUND)
        set(OpenCV_INCLUDE_DIRS "${OPENCV_INCLUDE_DIR}")
        set(OpenCV_LIBS
            optimized "${OPENCV_LIB_DIR}/opencv_world451.lib"
            debug "${OPENCV_LIB_DIR}/opencv_world451d.lib"
        )
    endif()

    target_include_directories(
        kungfuchess_client
        PRIVATE
            ${CMAKE_SOURCE_DIR}/demo
            ${OpenCV_INCLUDE_DIRS}
    )

    target_link_directories(
        kungfuchess_client
        PRIVATE
            ${OPENCV_LIB_DIR}
    )

    target_link_libraries(
        kungfuchess_client
        PRIVATE
            ${OpenCV_LIBS}
    )
endif()

# ── GoogleTest ──
add_subdirectory(external/googletest-1.17.0)

add_executable(
    chess_tests
    ${TEST_SOURCES}
    ${ALL_SOURCES}
)

target_compile_features(
    chess_tests
    PRIVATE
        cxx_std_17
)

target_include_directories(
    chess_tests
    PRIVATE
        ${COMMON_INCLUDE_DIR}
)

target_link_libraries(
    chess_tests
    PRIVATE
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(chess_tests)

# ── Demo ──
add_subdirectory(demo)
