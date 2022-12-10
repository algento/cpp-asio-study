cmake_minimum_required(VERSION 3.15)

project(cpp-asio-study)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Disalbe compiler extension
set(CMAKE_BUILD_TYPE Debug)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic -Wno-deprecated") # -std=c++20 -stdlib=libc++
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        add_compile_options("-stdlib=libc++")
    else()
    endif()
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

option(BUILD_COOKBOOK "build aiso cookbook" ON)
if (BUILD_COOKBOOK)
    add_subdirectory(src/asio-cookbook)
endif(BUILD_COOKBOOK)