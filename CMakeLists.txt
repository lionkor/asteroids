cmake_minimum_required(VERSION 3.15)

include_directories(include)
link_directories(lib)

project(asteroids)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra")

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
    sfml-window-s
    sfml-graphics-s
    sfml-system-s
    sfml-audio-s
    udev
    X11
    dl
    Xrandr
)

