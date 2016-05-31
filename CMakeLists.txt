cmake_minimum_required(VERSION 3.0)
project(charfs-module C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wstrict-prototypes -Wmissing-prototypes -Wall -Wextra")
set(CMAKE_C_COMPILER "gcc")


# Enable librt linking on Linux but not OS X
IF(UNIX)
    IF(APPLE)
        SET(LIBRT "")
    ELSE(APPLE)
        SET(LIBRT "rt")
    ENDIF(APPLE)
ENDIF(UNIX)


set(SOURCES charfs.c nvme-core.c nvme-scsi.c)


# Fix include directories
include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/uapi/)
include_directories(${CMAKE_SOURCE_DIR}/arch/arm/include/)
include_directories(${CMAKE_SOURCE_DIR}/arch/arm/include/uapi/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/uapi/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/arch/arm/include/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/arch/arm/include/uapi/)
add_definitions(-D__CLION__)
add_executable(charfs ${SOURCES})
add_executable(chardevtest chardevtest.c)
add_executable(readtest readtest.c)


# Target to SCP to and make on Zynq
add_custom_target(
        zynq-mitx ALL
        SOURCES ${SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND scp *.c *.h Makefile zynq-mitx:/root/charfs
        COMMAND ssh zynq-mitx 'cd /root/charfs && make'
)
