cmake_minimum_required(VERSION 3.14)

project(rem)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_definitions(XBYAK64)
add_definitions(-Wno-c++11-narrowing)

set(SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src)
set(LIBRARY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libraries)

file(GLOB_RECURSE SOURCES "${SOURCE_DIRECTORY}/*.cpp")
file(GLOB_RECURSE HEADERS "${SOURCE_DIRECTORY}/*.h")

include_directories(rem ${LIBRARY_DIRECTORY})
include_directories(rem ${SOURCE_DIRECTORY})

add_library(rem STATIC ${SOURCES} ${HEADERS})

target_include_directories(rem PUBLIC ${SOURCE_DIRECTORY})
