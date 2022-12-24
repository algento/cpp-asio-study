/**
 * @file socket_shutdown_client.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include <asio.hpp>

#include "asio/error_code.hpp"
#include "asio/io_context.hpp"

void Communicate(asio::ip::tcp::socket& sock) {
    // Allocating and filling the buffer with binary data. ("Hello")
    const char request_buf[] = {0x48, 0x65, 0x0, 0x6c, 0x6c, 0x6f};

    //* 3. 데이터 전송
    // Sending the request data.
    asio::write(sock, asio::buffer(request_buf));

    //* 4. 소켓의 전송 부분만 shutdown한다.
    // Shutting down the socket to let the server know that we've sent the whole request.
    sock.shutdown(asio::socket_base::shutdown_send);

    // We use extensible buffer for response because we don't know the size of the
    // response message.
    asio::streambuf response_buf;

    asio::error_code ec;
    //* 5. 서버 응답 수신
    asio::read(sock, response_buf, ec);

    if (ec == asio::error::eof) {
        // Whole response message has been received.
        // Here we can handle it.
        std::string s((std::istreambuf_iterator<char>(&response_buf)),
                      std::istreambuf_iterator<char>());
        std::cout << s << std::endl;
        std::cout << "asio::error::eof" << std::endl;
    } else {
        throw asio::system_error(ec);
    }
}

int main() {
    std::string raw_ip_address = "121.139.55.100";
    unsigned short port_num    = 3333;

    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address),
                                   port_num);

        asio::io_context ioc;

        //* 1. 소켓 할당
        asio::ip::tcp::socket sock(ioc, ep.protocol());

        //* 2. 소켓 연결
        sock.connect(ep);

        Communicate(sock);
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
        return e.code().value();
    }

    return 0;
}
