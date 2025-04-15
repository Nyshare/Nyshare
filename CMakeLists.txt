cmake_minimum_required(VERSION 3.26)
project(Nyshare)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_subdirectory(util)
add_subdirectory(database)
add_subdirectory(http)
add_subdirectory(entry)