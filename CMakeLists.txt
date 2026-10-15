cmake_minimum_required(VERSION 3.10)
project(ex1_deduplication)

set(CMAKE_CXX_STANDARD 20)


add_executable(ex1_deduplication
        src/main.cpp
        src/Point.cpp
        src/Greedy.cpp
        include/SpatialHashReduction.h
        src/SpatialHashReduction.cpp
        include/ConvexHull.h
        src/ConvexHull.cpp
        include/GrahamHull.h
        src/GrahamHull.cpp
)

target_include_directories(ex1_deduplication PRIVATE include)


add_executable(zad2_sensors
        src/main_zad2.cpp
)

target_include_directories(zad2_sensors PRIVATE include)