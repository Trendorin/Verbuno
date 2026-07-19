function(translunix_enable_sanitizers target)
    if(NOT TRANSLUNIX_ENABLE_SANITIZERS)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -fno-omit-frame-pointer
            -fsanitize=address,undefined
        )
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    else()
        message(WARNING "Sanitizers are not configured for this compiler")
    endif()
endfunction()
