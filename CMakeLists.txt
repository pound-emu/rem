cmake_minimum_required(VERSION 3.14)

set(PROJECT_SHARED rem)

set(SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src/)
set(LIBRARY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libraries/)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(${PROJECT_SHARED})

add_definitions(-std=c++17)
add_compile_definitions(XBYAK64)

set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE SOURCES "${SOURCE_DIRECTORY}*.cpp")
file(GLOB_RECURSE HEADERS "${SOURCE_DIRECTORY}*.h")

include_directories(${PROJECT_SHARED} ${LIBRARY_DIRECTORY})
include_directories(${PROJECT_SHARED} ${SOURCE_DIRECTORY})

add_library(${PROJECT_SHARED} SHARED ${SOURCES} ${HEADERS})
