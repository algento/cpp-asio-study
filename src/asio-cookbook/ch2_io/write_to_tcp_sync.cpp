/**
 * @file write_to_tcp_sync.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>

#include <asio.hpp>

#include "asio/io_context.hpp"
#include "asio/ip/address.hpp"
#include "asio/system_error.hpp"

void WriteToSocketEnhanced(asio::ip::tcp::socket& socket) {
    // Allocating and filling the buffer.
    std::string buf = "Hello";
    asio::write(socket, asio::buffer(buf));
}

void WriteToSocket(asio::ip::tcp::socket& socket) {
    // Allocating and filling the buffer.
    std::string buf = "Hello";
    // Run the loop until all data is written to the socket.
    std::size_t total_bytes_written = 0;
    while (total_bytes_written != buf.length()) {
        total_bytes_written +=
            socket.write_some(asio::buffer(buf.c_str() + total_bytes_written,
                                           buf.length() - total_bytes_written));
        std::cout << total_bytes_written << std::endl;
    }
}

int main() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num    = 3333;

    try {
        asio::ip::address ip_address = asio::ip::make_address(raw_ip_address);

        asio::ip::tcp::endpoint ep(ip_address, port_num);
        asio::io_context ioc;

        asio::ip::tcp::socket sock(ioc, ep.protocol());

        sock.connect(ep);

        WriteToSocket(sock);

    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }
    return 0;
}