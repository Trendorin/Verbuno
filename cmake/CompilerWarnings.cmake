function(translunix_set_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wcast-align
            -Wconversion
            -Wformat=2
            -Wnull-dereference
            -Woverloaded-virtual
            -Wshadow
            -Wsign-conversion
        )
    endif()
endfunction()
