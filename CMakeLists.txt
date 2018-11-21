
project(GmshReader LANGUAGES CXX)

cmake_minimum_required(VERSION 3.4)

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${BOOST_INCLUDE})

set(EXT_PROJECTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

add_subdirectory(${EXT_PROJECTS_DIR}/jsoncpp)
include_directories(${EXT_PROJECTS_DIR}/jsoncpp/dist/)

set(CMAKE_CXX_STANDARD 14)
set(GMSH_READER_LIBRARIES_INSTALL_PATH "lib")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

option(ENABLE_DEBUG "enables debug information" OFF)
option(ENABLE_COVERAGE "Set compiler flag for coverage analysis" OFF)

option(ENABLE_TEST "Enable the test suite" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts ${CMAKE_MODULE_PATH})
if(ENABLE_COVERAGE)
    include(Coverage)
endif()

# set build type
IF(ENABLE_DEBUG)
  add_definitions("-DDEBUG")
  set(CMAKE_BUILD_TYPE "Debug")
ELSE(ENABLE_DEBUG)
  IF(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
  ENDIF(NOT CMAKE_BUILD_TYPE)
ENDIF(ENABLE_DEBUG)

if (ENABLE_TEST)
    include(CTest)

    find_package(Catch2 2.3)

    if (NOT ${Catch2_FOUND})
        add_subdirectory(${EXT_PROJECTS_DIR}/catch)
    endif()

    add_subdirectory(tests)
endif()

add_subdirectory(src)
add_subdirectory(examples)

add_executable(gmshreader src/main.cpp)

target_link_libraries(gmshreader GmshReader ${Boost_LIBRARIES})

install(TARGETS gmshreader
        RUNTIME DESTINATION bin)
