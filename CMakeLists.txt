cmake_minimum_required(VERSION 3.0)
project(chario-module C)

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


set(SOURCES chario.c nvme-core.c nvme-scsi.c)


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
add_executable(chario ${SOURCES})
add_executable(chardevtest chardevtest.c)
add_executable(readtest readtest.c)
add_executable(readtest2 readtest2.c)
add_executable(writetest writetest.c)
add_executable(ocmreadtest ocmreadtest.c)
add_executable(ocmwritetest ocmwritetest.c)
add_executable(bramreadtest bramreadtest.c)
add_executable(bramwritetest bramwritetest.c)
add_executable(bramtest bramtest.c)


# Target to SCP to and make on Zynq
add_custom_target(
        zynq-mitx ALL
        SOURCES ${SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND rsync -uv *.c *.h Makefile zynq-mitx:/root/chario
        COMMAND ssh zynq-mitx 'cd /root/chario && make'
)

# Target to SCP to and make on Dave
add_custom_target(
        dave ALL
        SOURCES ${SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND rsync -uv *.c *.h Makefile dave:/home/russell/chario
        COMMAND ssh dave 'cd /home/russell/chario && make'
)

# Target to SCP to and make libchario on Zynq
add_custom_target(
        libchario-zynq ALL
        SOURCES libchario/libchario.c libchario/libchario-test.c
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libchario
        COMMAND rsync -ruv CMakeLists.txt *.c *.h zynq-mitx:/root/chario/libchario
        COMMAND ssh zynq-mitx 'cd /root/chario/libchario && make'
)
