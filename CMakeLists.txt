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


set(SOURCES chario.c nvme-core.c)


# Fix include directories
include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/uapi/)
include_directories(${CMAKE_SOURCE_DIR}/arch/arm64/include/)
include_directories(${CMAKE_SOURCE_DIR}/arch/arm64/include/uapi/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/uapi/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/arch/arm64/include/)
set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/arch/arm64/include/uapi/)
add_definitions(-D__CLION__)
add_executable(chario ${SOURCES})
add_executable(chardevtest chardevtest.c)
add_executable(readtest readtest.c)
add_executable(readtest2 readtest2.c)
add_executable(readtest3 readtest3.c)
add_executable(writetest writetest.c)
add_executable(ocmreadtest ocmreadtest.c)
add_executable(ocmwritetest ocmwritetest.c)
add_executable(bramreadtest bramreadtest.c)
add_executable(bramwritetest bramwritetest.c)
add_executable(bramtest bramtest.c)
add_executable(ddrreadtest ddrreadtest.c)
add_executable(ddrwritetest ddrwritetest.c)
add_executable(ddrtest ddrtest.c)
add_executable(acptest acptest.c)


# Target to SCP to and make on Zynq MITX
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

# Target to SCP to and make on ZCU102
add_custom_target(
        zcu102 ALL
        SOURCES ${SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND rsync -uv *.c *.h Makefile zcu102:/home/russell/chario
        COMMAND ssh zcu102 'cd /home/russell/chario && make'
)

# Target to SCP to and make libchario on Zynq MITX
add_custom_target(
        libchario-zynq ALL
        SOURCES libchario/libchario.c libchario/libchario-test.c
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libchario
        COMMAND rsync -ruv CMakeLists.txt *.c *.h zynq-mitx:/root/chario/libchario
        COMMAND ssh zynq-mitx 'cd /root/chario/libchario && cmake . && make'
)

# Target to SCP to and make libchario on ZCU102
add_custom_target(
        libchario-zcu102 ALL
        SOURCES libchario/libchario.c libchario/libchario-test.c
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libchario
        COMMAND rsync -ruv CMakeLists.txt *.c *.h zcu102:/home/russell/chario/libchario
        COMMAND ssh zcu102 'cd /home/russell/chario/libchario && cmake . && make'
)
