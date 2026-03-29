cmake_minimum_required(VERSION 2.8.0...3.20.1)

set(TARGET tuibox_test)

set(${TARGET}_SOURCES
    common.c
    main.c
    suite_core.c
    )
   
set(${TARGET}_HEADERS
    common.h
    greatest.h
    )

add_executable(${TARGET} ${${TARGET}_SOURCES} ${${TARGET}_HEADERS})

target_include_directories(${TARGET} PUBLIC
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
   )

   target_link_libraries(${TARGET} PUBLIC
    tuibox
    m
    pthread
   )

set_target_properties(${TARGET} PROPERTIES
   FOLDER test
)

install(TARGETS ${TARGET} EXPORT ${TARGET}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )
