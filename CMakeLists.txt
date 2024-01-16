cmake_minimum_required(VERSION 3.18)
project(rasberry_client)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wno-write-strings)

file(GLOB_RECURSE SOURCES "extern_var/src/*.*" "camera/src/*.*" "net/src/*.*" "process/src/*.*" "network/src/*.*" "log/src/*.*")

include_directories(extern_var/inc camera/inc net/inc process/inc network/inc log/inc)

add_executable(
        rasberry_client
        main.cpp
        ${SOURCES}
)

add_executable(
        test_camera
        test/src/test_camera.cpp
        ${SOURCES}
)

add_executable(
        test_net
        test/src/test_net.cpp
        ${SOURCES}
)

#target_link_libraries(Heart_Detection_Server "/usr/lib/x86_64-linux-gnu/libraylib.a")
