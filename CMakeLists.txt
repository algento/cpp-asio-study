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

include(FetchContent)
find_package(Threads)
# Find ASIO
find_package(Asio 1.24.0 QUIET)
if (NOT Asio_FOUND)
    FetchContent_Declare(asio 
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git 
        GIT_TAG asio-1-24-0
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/asio
        BINARY_DIR ${CMAKE_SOURCE_DIR}/lib
        )
    FetchContent_GetProperties(asio)
    if (NOT asio_POPULATED)
        FetchContent_Populate(asio )
        add_library(asio INTERFACE)
        target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
        target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
        target_link_libraries(asio INTERFACE Threads::Threads)
    endif ()
endif()

link_libraries(asio)

option(BUILD_COOKBOOK "build aiso cookbook" ON)
if (BUILD_COOKBOOK)
    add_subdirectory(src/asio-cookbook)
endif(BUILD_COOKBOOK)

option(BUILD_OLC "build OneLoneCoder" ON)
if (BUILD_OLC)
    add_subdirectory(src/one_lone_coder)
endif(BUILD_OLC)