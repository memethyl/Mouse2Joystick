cmake_minimum_required(VERSION 3.21)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

project(M2JWin32
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(${CMAKE_SOURCE_DIR}/submodules/ViGEmClient ${CMAKE_BINARY_DIR}/submodules/ViGEmClient)
include_directories(
    ${CMAKE_SOURCE_DIR}/submodules/ViGEmClient/include
    ${CMAKE_SOURCE_DIR}/src
)

set(SOURCES Backend.cpp)

add_library(M2JWin32 SHARED ${SOURCES})
add_dependencies(M2JWin32 ViGEmClient)
