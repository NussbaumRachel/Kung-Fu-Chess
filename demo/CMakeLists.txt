cmake_minimum_required(VERSION 3.15)

# ── Demo target: GUI demo for Kung-Fu-Chess ──
# Depends on OpenCV 4.5.1

set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/OpenCV_451" CACHE PATH "Path to OpenCV 4.5.1 installation")

set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
set(OPENCV_LIB_DIR "${OPENCV_DIR}/bin")

# Check OpenCV
if(NOT EXISTS "${OPENCV_INCLUDE_DIR}/opencv2/core.hpp")
    message(WARNING "==============================================")
    message(WARNING "  OpenCV 4.5.1 NOT FOUND at: ${OPENCV_DIR}")
    message(WARNING "  Demo target will be skipped.")
    message(WARNING "==============================================")
    return()
endif()

# Find OpenCV
find_package(OpenCV QUIET)

if(NOT OpenCV_FOUND)
    message(STATUS "OpenCV not found via find_package, using manual paths")

    set(OpenCV_INCLUDE_DIRS
        "${OPENCV_INCLUDE_DIR}"
    )

    set(OpenCV_LIBS
        optimized "${OPENCV_LIB_DIR}/opencv_world451.lib"
        debug "${OPENCV_LIB_DIR}/opencv_world451d.lib"
    )
endif()


# Demo + Engine sources
set(DEMO_SOURCES

    # Demo layer
    main.cpp
    img.cpp
    AnimatedSprite.cpp
    AssetManager.cpp
    AnimationManager.cpp
    ChessRenderer.cpp
    BoardRenderer.cpp
    PieceRenderer.cpp
    MouseHandler.cpp


    # Model
    ${CMAKE_SOURCE_DIR}/src/model/Piece.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Board.cpp
    ${CMAKE_SOURCE_DIR}/src/model/King.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Queen.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Rook.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Bishop.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Knight.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Pawn.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Position.hpp
    ${CMAKE_SOURCE_DIR}/src/model/types.hpp


    # Movement
    ${CMAKE_SOURCE_DIR}/src/movement/Move.cpp
    ${CMAKE_SOURCE_DIR}/src/movement/PieceFactory.cpp


    # Rules
    ${CMAKE_SOURCE_DIR}/src/rule_engine/RuleEngine.cpp


    # Arbiter
    ${CMAKE_SOURCE_DIR}/src/arbiter/RealTimeArbiter.cpp


    # Game engine
    ${CMAKE_SOURCE_DIR}/src/game_engine/GameStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/game_engine/BoardController.cpp
    ${CMAKE_SOURCE_DIR}/src/game_engine/GameEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/game_engine/MoveCompletionService.cpp
    ${CMAKE_SOURCE_DIR}/src/game_engine/ClickPreparationService.cpp
    ${CMAKE_SOURCE_DIR}/src/game_engine/ClickPreparationService.hpp
    ${CMAKE_SOURCE_DIR}/src/game_engine/ClickContext.hpp
    ${CMAKE_SOURCE_DIR}/src/game_engine/GameSnapshot.hpp


    # Controller
    ${CMAKE_SOURCE_DIR}/src/controllerClick/GameController.cpp


    # IO
    ${CMAKE_SOURCE_DIR}/src/text_io/BoardParser.cpp
)


set(DEMO_HEADERS

    img.hpp
    AnimatedSprite.hpp
    AssetManager.hpp
    AnimationManager.hpp
    ChessRenderer.hpp
    BoardRenderer.hpp
    PieceRenderer.hpp
    MouseHandler.hpp
    DemoConfig.hpp
)


add_executable(kungfuchess_demo
    ${DEMO_SOURCES}
    ${DEMO_HEADERS}
)


target_include_directories(kungfuchess_demo PRIVATE

    # Demo headers
    ${CMAKE_CURRENT_SOURCE_DIR}

    # Source headers
    ${CMAKE_SOURCE_DIR}/src

    # OpenCV
    ${OpenCV_INCLUDE_DIRS}
)


target_link_directories(kungfuchess_demo PRIVATE
    ${OPENCV_LIB_DIR}
)


target_link_libraries(kungfuchess_demo PRIVATE
    ${OpenCV_LIBS}
)


# Copy OpenCV DLLs on Windows
if(WIN32)

    add_custom_command(TARGET kungfuchess_demo POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_if_different

            "$<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.dll>"

            "$<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.dll>"

            "$<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.dll>"

            "$<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.dll>"

            $<TARGET_FILE_DIR:kungfuchess_demo>
    )

endif()


message(STATUS "Demo OpenCV include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Demo OpenCV lib dir: ${OPENCV_LIB_DIR}")
message(STATUS "Demo sources: ${DEMO_SOURCES}")