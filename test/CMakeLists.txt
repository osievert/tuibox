set(TARGET tuibox_test)

set(${TARGET}_SOURCES
    main.c
    suite_all.c
    # the files below are #included into suite_all.c to avoid duplicate symbols because tuibox.h is a header-only library
    # suite_boxes.c
    # suite_events.c
    # suite_lifecycle.c
    # suite_render.c
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
