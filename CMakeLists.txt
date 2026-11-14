cmake_minimum_required(VERSION 3.20)
project(Tetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# SFML configuration
if(NOT ANDROID)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
endif()
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Do not build SFML examples" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "Do not build SFML documentation" FORCE)
set(SFML_INSTALL_PKGCONFIG_FILES OFF CACHE BOOL "Do not install pkgconfig" FORCE)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/sfml-src/CMakeLists.txt")
    FetchContent_Declare(
        SFML
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/sfml-src"
    )
else()
    FetchContent_Declare(
        SFML
        URL https://github.com/SFML/SFML/archive/refs/tags/2.6.2.zip
    )
endif()
FetchContent_MakeAvailable(SFML)

set(TETRIS_SOURCES
    src/main.cpp
    src/Game.cpp
    src/Board.cpp
    src/Tetromino.cpp
    src/PieceBag.cpp
    src/Particles.cpp
    src/SoundManager.cpp
)

if(ANDROID)
    add_library(Tetris SHARED ${TETRIS_SOURCES})
    target_link_libraries(Tetris PRIVATE 
        -Wl,--whole-archive sfml-main -Wl,--no-whole-archive
        sfml-graphics 
        sfml-audio 
        sfml-window 
        sfml-system
        log
        android
        EGL
        GLESv1_CM
    )
else()
    add_executable(Tetris ${TETRIS_SOURCES})
    target_link_libraries(Tetris PRIVATE 
        sfml-graphics 
        sfml-audio 
        sfml-window 
        sfml-system
    )
    if(WIN32)
        target_link_libraries(Tetris PRIVATE opengl32 winmm gdi32)
        if(EXISTS "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
            add_custom_command(TARGET Tetris POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
                "$<TARGET_FILE_DIR:Tetris>"
            )
        endif()
    endif()
endif()

target_include_directories(Tetris PRIVATE include)
