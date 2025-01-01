cmake_minimum_required(VERSION 3.14)

set(PROJECT rem)
set(SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src/)
set(LIBRARY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libraries/)

project(${PROJECT})

include(FetchContent)

add_definitions(-std=c++17)
add_compile_definitions(XBYAK64)

set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE SOURCES "${SOURCE_DIRECTORY}*.cpp")
file(GLOB_RECURSE HEADERS "${SOURCE_DIRECTORY}*.h")

include_directories(${PROJECT} ${LIBRARY_DIRECTORY})
include_directories(${PROJECT} ${SOURCE_DIRECTORY})

add_executable(${PROJECT} ${SOURCES} ${HEADERS})

FetchContent_Declare(
    unicorn
    GIT_REPOSITORY https://github.com/unicorn-engine/unicorn.git
    GIT_SHALLOW ON
)

FetchContent_Declare(
    capstone
    GIT_REPOSITORY https://github.com/capstone-engine/capstone.git
    GIT_SHALLOW ON
)

FetchContent_Declare(
    keystone
    GIT_REPOSITORY https://github.com/keystone-engine/keystone.git
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(unicorn capstone keystone)

target_link_libraries(rem PUBLIC unicorn)
target_link_libraries(rem PUBLIC capstone)
target_link_libraries(rem PUBLIC keystone)