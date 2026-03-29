set(TARGET tuibox_test)

set(${TARGET}_SOURCES
    all_suites.c
    main.c
    )
   
set(${TARGET}_HEADERS
    common.h
    greatest.h
    )

add_executable(${TARGET} ${${TARGET}_SOURCES} ${${TARGET}_HEADERS})

target_compile_features(${TARGET} PRIVATE c_std_99)
target_link_libraries(${TARGET} PRIVATE tuibox::tuibox)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(${TARGET} PRIVATE "${MATH_LIBRARY}")
endif()
