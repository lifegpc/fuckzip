cmake_minimum_required(VERSION 3.18)

project(fuckzip)

if (MSVC)
    add_compile_options(/utf-8)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(LIBZIP REQUIRED)

include(CheckIncludeFiles)

CHECK_INCLUDE_FILES(getopt.h HAVE_GETOPT_H)
if (NOT HAVE_GETOPT_H)
    add_subdirectory(getopt)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/getopt")
endif()

add_subdirectory(utils)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/utils")

add_subdirectory(src)
