/**
 * @file read_from_tcp_sync.cpp
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

std::string ReadFromSocketDelim(asio::ip::tcp::socket& sock) {
    asio::streambuf buf;

    // Synchronously read data from the socket until '\n' symbol is encountered.
    asio::read_until(sock, buf, '\n');

    std::string message;

    // Because buffer 'buf' may contain some other data
    // after '\n' symbol, we have to parse the buffer and
    // extract only symbols before the delimiter.
    std::istream input_stream(&buf);
    std::getline(input_stream, message);

    return message;
}

std::string ReadFromSocketEnhanced(asio::ip::tcp::socket& sock) {
    const unsigned char MESSAGE_SIZE = 7;
    char buf[MESSAGE_SIZE];

    asio::read(sock, asio::buffer(buf, MESSAGE_SIZE));

    return {buf, MESSAGE_SIZE};
}

std::string ReadFromSocket(asio::ip::tcp::socket& sock) {
    const unsigned char MESSAGE_SIZE = 7;
    char buf[MESSAGE_SIZE];
    std::size_t total_bytes_read = 0;

    while (total_bytes_read != MESSAGE_SIZE) {
        total_bytes_read += sock.read_some(asio::buffer(
            buf + total_bytes_read, MESSAGE_SIZE - total_bytes_read));
    }

    return {buf, total_bytes_read};
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

        ReadFromSocket(sock);
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what() << std::endl;

        return e.code().value();
    }

    return 0;
}
