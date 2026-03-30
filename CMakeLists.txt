cmake_minimum_required(VERSION 3.23)

project(tuibox LANGUAGES C)

option(TUIBOX_EXAMPLES "Build tuibox example programs" ON)
option(TUIBOX_TEST "Build tuibox test program" ON)

add_library(tuibox src/tuibox.c src/vec.c)
add_library(tuibox::tuibox ALIAS tuibox)

target_compile_features(tuibox PUBLIC c_std_99)

target_include_directories(tuibox
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_sources(tuibox
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/tuibox/tuibox.h"
)

if(TUIBOX_EXAMPLES)
    add_subdirectory(examples)
endif()

if(TUIBOX_TEST)
    add_subdirectory(test)
endif()
