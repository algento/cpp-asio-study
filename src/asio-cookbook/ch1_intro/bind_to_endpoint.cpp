/**
 * @file bind_to_endpoint.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <iostream>

#include <asio.hpp>

#include "asio/io_context.hpp"

#define IS_TCP 0

int main() {
    // Step 1. Here we assume that the server application has already
    // obtained the protocol port number.
    unsigned short port_num = 3333;

// Step 2. Creating an endpoint.
#if IS_TCP
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
#else
    asio::ip::udp::endpoint ep(asio::ip::address_v4::any(), port_num);
#endif

    // Step 3. Create and open an socket
    // - UDP는 능동소켓으로만 만들 수 있다.
    asio::io_context ioc;

#if IS_TCP
    asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
#else
    asio::ip::udp::socket sock(ioc, ep.protocol());
#endif

    asio::error_code ec;

// Step 4. Binding the acceptor socket.
#if IS_TCP
    acceptor.bind(ep, ec);
#else
    sock.bind(ep, ec);
#endif

    // Handling errors if any.
    if (ec.value() != 0) {
        // Failed to bind the acceptor socket. Breaking execution.
        std::cout << "Failed to bind the socket."
                  << "Error code = " << ec.value()
                  << ". Message: " << ec.message();

        return ec.value();
    }

    return 0;
}