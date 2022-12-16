/**
 * @file connect_socket.cpp
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

#include "asio/ip/address.hpp"

int main() {
    // Step 1. Assume that the client application has already
    // obtained the IP address and protocol port number of the
    // target server.
    //* 이 설정은 클라이언트가 가지고 있는 서버 종료점에 대한 정보이다.
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num    = 3333;

    asio::error_code ec;

#if 1
    // Step 2. Creating an endpoint designating
    // a target server application.
    asio::ip::address ip_address = asio::ip::make_address(raw_ip_address, ec);
    asio::ip::tcp::endpoint ep(ip_address, port_num);

    asio::io_context ioc;

    // Step 3. Creating and opening a socket.
    asio::ip::tcp::socket sock(ioc, ep.protocol());

    // Step 4. Connecting a socket.
    sock.connect(ep, ec);
    // Handling errors if any.
    if (ec.value() != 0) {
        // Failed to bind the acceptor socket. Breaking execution.
        std::cout << "Failed to connect  the socket."
                  << "Error code = " << ec.value()
                  << ". Message: " << ec.message() << std::endl;

        return ec.value();
    }

    //* error_code를 인자로 받지 않으면, system_error를 예외로 던지는 버전이 호출된다.
#else
    try {
        asio::ip::address ip_address = asio::ip::make_address(raw_ip_address);
        asio::ip::tcp::endpoint ep(ip_address, port_num);

        asio::io_context ioc;

        asio::ip::tcp::socket sock(ioc, ep.protocol());

        sock.connect(ep);
    } catch (asio::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();

        return e.code().value();
    }
#endif

    return 0;
}
