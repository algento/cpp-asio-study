include(FetchContent)
find_package(Threads)
# Find ASIO
find_package(Asio 1.24.0 QUIET)
if (NOT Asio_FOUND)
    FetchContent_Declare(asio GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git GIT_TAG asio-1-24-0)
    FetchContent_GetProperties(asio)
    if (NOT asio_POPULATED)
        FetchContent_Populate(asio)
        add_library(asio INTERFACE)
        target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
        target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
        target_link_libraries(asio INTERFACE Threads::Threads)
    endif ()
endif()

add_executable(asio1_endpoint_client ch1_intro/endpoint_client.cpp)
target_link_libraries(asio1_endpoint_client asio)


add_executable(asio1_endpoint_server ch1_intro/endpoint_server.cpp)
target_link_libraries(asio1_endpoint_server asio)


add_executable(asio1_active_socket ch1_intro/active_socket.cpp)
target_link_libraries(asio1_active_socket asio)

