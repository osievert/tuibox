find_library(MATH_LIBRARY m)

function(tuibox_add_demo name)
    add_executable(${name} "${name}.c")
    target_compile_features(${name} PRIVATE c_std_99)
    target_link_libraries(${name} PRIVATE tuibox::tuibox)

    if(MATH_LIBRARY)
        target_link_libraries(${name} PRIVATE "${MATH_LIBRARY}")
    endif()
endfunction()

tuibox_add_demo(tuibox_basic)
tuibox_add_demo(tuibox_bounce)
tuibox_add_demo(tuibox_bounce_inline)
tuibox_add_demo(tuibox_drag)
