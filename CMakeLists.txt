cmake_minimum_required(VERSION 3.26)

project(tortoise LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TORTOISE_EXCEPTIONS "Enable C++ exceptions" OFF)
option(TORTOISE_TESTS "Enable or disable Tortoise tests" OFF)

include(CheckIPOSupported)

check_ipo_supported(
    RESULT IPO_SUPPORTED
    OUTPUT IPO_ERROR
)

if(IPO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    message(STATUS "LTO enabled")
else()
    message(WARNING "LTO unavailable: ${IPO_ERROR}")
endif()

if(MSVC)

    add_compile_options(
        $<$<BOOL:${TORTOISE_EXCEPTIONS}>:/EHsc>
        $<$<NOT:$<BOOL:${TORTOISE_EXCEPTIONS}>>:/EHs-c->
        /W4
        /permissive-
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/arch:AVX2>
    )

    add_compile_definitions(
        $<$<NOT:$<BOOL:${TORTOISE_EXCEPTIONS}>>:_HAS_EXCEPTIONS=0>
    )

    add_link_options(
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
    )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")

    add_compile_options(
        -Wall
        -Wextra
        -pedantic
        -Wconversion
        -Wsign-conversion
        -Wimplicit-fallthrough
        -Werror=format-security
        -Wformat

        $<$<NOT:$<BOOL:${TORTOISE_EXCEPTIONS}>>:-fno-exceptions>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    )
    if(NOT APPLE)
        add_compile_options(-march=native)
    endif()

endif()

add_subdirectory(rules)
add_subdirectory(playground)