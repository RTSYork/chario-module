cmake_minimum_required(VERSION 3.0)
project(libchario C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wstrict-prototypes -Wmissing-prototypes -Wall -Wextra")
set(CMAKE_C_COMPILER "gcc")


# Enable librt linking on Linux but not OS X
if(UNIX)
    if(APPLE)
        set(LIBRT "")
    else(APPLE)
        set(LIBRT "rt")
    endif(APPLE)
endif(UNIX)


# Fix include directories
#include_directories(${CMAKE_SOURCE_DIR}/include/)
#include_directories(${CMAKE_SOURCE_DIR}/include/uapi/)
#include_directories(${CMAKE_SOURCE_DIR}/arch/arm/include/)
#include_directories(${CMAKE_SOURCE_DIR}/arch/arm/include/uapi/)
#set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/)
#set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/uapi/)
#set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/arch/arm/include/)
#set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/arch/arm/include/uapi/)

add_library(libchario libchario.c)

add_executable(libchario-test libchario-test.c)
target_link_libraries(libchario-test libchario)
