cmake_minimum_required(VERSION 3.15)
project(stw)

set(CMAKE_CXX_STANDARD 20)

option(PLATFORM_IS_WINDOWS "Use Windows platform definition" OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0")
endif()

file(GLOB_RECURSE SOURCES src/*.cpp src/*.c)

include_directories(
    "${PROJECT_SOURCE_DIR}/include/"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if(WIN32 OR PLATFORM_IS_WINDOWS)
    target_link_libraries(${PROJECT_NAME} PRIVATE -static ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE -ldl pthread)
endif()