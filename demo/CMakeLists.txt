cmake_minimum_required(VERSION 3.15)

# ── Demo target: GUI demo for Kung-Fu-Chess ──
# Depends on OpenCV 4.5.1 (from KamaTech repo)
#
# Setup:
#   1. Download OpenCV_451 folder from Google Drive (see repo README)
#   2. Place it at <project-root>/OpenCV_451
#   3. Or set OPENCV_DIR when running CMake:
#        cmake -DOPENCV_DIR="C:/path/to/OpenCV_451" ...

# OpenCV paths — adjust these to your local OpenCV installation
set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/OpenCV_451" CACHE PATH "Path to OpenCV 4.5.1 installation")

set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
set(OPENCV_LIB_DIR "${OPENCV_DIR}/bin")

# Check if OpenCV exists at the expected location
if(NOT EXISTS "${OPENCV_INCLUDE_DIR}/opencv2/core.hpp")
    message(WARNING "==============================================")
    message(WARNING "  OpenCV 4.5.1 NOT FOUND at: ${OPENCV_DIR}")
    message(WARNING "  Download from the Google Drive link in the repo README,")
    message(WARNING "  or run: cmake -DOPENCV_DIR=\"C:/path/to/OpenCV_451\" ...")
    message(WARNING "  The demo target will be SKIPPED.")
    message(WARNING "==============================================")
    return()
endif()

# Find OpenCV (config mode may fail if not installed system-wide;
# we provide the paths manually as fallback)
find_package(OpenCV QUIET)

if(NOT OpenCV_FOUND)
    message(STATUS "OpenCV not found via find_package, using manual paths")
    set(OpenCV_INCLUDE_DIRS "${OPENCV_INCLUDE_DIR}")
    set(OpenCV_LIBS
        optimized "${OPENCV_LIB_DIR}/opencv_world451.lib"
        debug "${OPENCV_LIB_DIR}/opencv_world451d.lib"
    )
endif()

# Demo sources
set(DEMO_SOURCES
    main.cpp
    img.cpp
    AnimatedSprite.cpp
    AssetManager.cpp
    AnimationManager.cpp
    ChessRenderer.cpp
    BoardRenderer.cpp
    PieceRenderer.cpp
    MouseHandler.cpp
    DemoBoardState.cpp
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
    DemoBoardState.hpp
)

add_executable(kungfuchess_demo ${DEMO_SOURCES} ${DEMO_HEADERS})

target_include_directories(kungfuchess_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src          # for GameSnapshot.hpp, types.hpp, etc.
    ${OpenCV_INCLUDE_DIRS}
)

target_link_directories(kungfuchess_demo PRIVATE ${OPENCV_LIB_DIR})

target_link_libraries(kungfuchess_demo ${OpenCV_LIBS})

# Copy OpenCV DLLs to output directory on Windows
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
