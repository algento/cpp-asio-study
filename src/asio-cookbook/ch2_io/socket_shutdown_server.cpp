/**
 * @file socket_shutdown_server.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <iostream>

#include <asio.hpp>

#include "asio/io_context.hpp"
#include "asio/ip/address.hpp"

void ProcessRequest(asio::ip::tcp::socket& sock) {
    // We use extensibel buffer because we don't know the size of the request message.
    asio::streambuf request_buf;

    asio::error_code ec;

    // Receiving the request.
    asio::read(sock, request_buf, ec);
    std::string request_string((std::istreambuf_iterator<char>(&request_buf)),
                               std::istreambuf_iterator<char>());
    std::cout << request_string << std::endl;

    if (ec != asio::error::eof) {
        throw asio::system_error(ec);
    }

    // Request received. Sending response. Allocating and filling the buffer with binary data.
    const char response_buf[] = {0x48, 0x69, 0x21};

    // Sending the request data.
    asio::write(sock, asio::buffer(response_buf));
    std::string response_string(response_buf);
    std::cout << response_string << std::endl;

    // Shutting down the socket to let the client know that we've sent the whole response.
    sock.shutdown(asio::socket_base::shutdown_send);
}

int main() {
    unsigned short port_num = 3333;

    try {
        // asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
        asio::ip::tcp::endpoint ep(asio::ip::make_address("121.139.55.100"),
                                   port_num);

        asio::io_context ioc;

        asio::ip::tcp::acceptor acceptor(ioc, ep);

        asio::ip::tcp::socket sock(ioc);

        acceptor.accept(sock);

        ProcessRequest(sock);
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();

        return e.code().value();
    }

    return 0;
}
