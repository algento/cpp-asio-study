link_libraries(asio)

## ch1. Intro.
add_executable(asio1_endpoint_client ch1_intro/endpoint_client.cpp)
add_executable(asio1_endpoint_server ch1_intro/endpoint_server.cpp)
add_executable(asio1_active_tcp_socket ch1_intro/active_tcp_socket.cpp)
add_executable(asio1_active_udp_socket ch1_intro/active_udp_socket.cpp)
add_executable(asio1_passive_tcp_socket ch1_intro/passive_tcp_socket.cpp)
add_executable(asio1_resolving_dns_tcp ch1_intro/resolving_dns.cpp)
add_executable(asio1_bind_to_endpoint ch1_intro/bind_to_endpoint.cpp)
add_executable(asio1_connect_socket ch1_intro/connect_socket.cpp)
add_executable(asio1_connect_dns ch1_intro/connect_dns.cpp)
add_executable(asio1_accept_connection ch1_intro/accept_connection.cpp)

## ch2. I/O
add_executable(asio2_fixed_length_buffer ch2_io/fixed_length_buffer.cpp)
add_executable(asio2_extensible_streambuffer ch2_io/extensible_streambuffer.cpp)
