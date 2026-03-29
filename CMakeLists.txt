cmake_minimum_required(VERSION 3.23)

project(tuibox LANGUAGES C)

option(TUIBOX_EXAMPLES "Build tuibox example programs" ON)

add_library(tuibox INTERFACE)
add_library(tuibox::tuibox ALIAS tuibox)

target_compile_features(tuibox INTERFACE c_std_99)

target_include_directories(tuibox
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_sources(tuibox
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/tuibox.h"
)

if(TUIBOX_EXAMPLES)
    add_subdirectory(examples)
endif()
