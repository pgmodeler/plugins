#Author Piotr Wygocki

set(PAAL ${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 2.8)

SET(CMAKE_COLOR_MAKEFILE ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-tautological-constant-out-of-range-compare")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu")
    ADD_DEFINITIONS("-stdlib=libc++")
    set(LIBS c++)
else()
    if(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    endif()
endif()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pthread -Wall -fno-strict-aliasing")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe -g")
else()
    add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4521 /wd4522" )
    # TODO fix paths to glpk on windows
    include_directories ("C:/Program Files (x86)/GnuWin32/include")
    link_directories ("C:/Program Files (x86)/GnuWin32/lib")
endif()

file(GLOB BOOST_SUBS "${PAAL}/boost/*")

foreach(dir ${BOOST_SUBS})
    include_directories("${dir}/include/")
endforeach()
include_directories("${BOOST_SUBS}/numeric_bindings/")

include_directories (${PAAL}/include)

add_subdirectory(bin)

if(${WITH_TESTS})
    add_subdirectory(test)
endif()

if(${WITH_EXAMPLES})
    add_subdirectory(examples)
endif()

if(NOT CMAKE_BUILD_TYPE)
    message("BUILD TYPE = NORMAL -  NEITHER RELEASE OR DEBUG IS SET")
else()
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWERCASE)
    if (BUILD_TYPE_LOWERCASE MATCHES release)
        message("BUILD TYPE = RELEASE")
    elseif(BUILD_TYPE_LOWERCASE MATCHES debug)
        message("BUILD TYPE = DEBUG")
    else()
        message(WARNING "BUILD TYPE = UNKNOWN")
    endif()
endif()
