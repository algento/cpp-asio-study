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
//* server가 준비될 때까지 반복해서 async_connect를 수행해서 접속하는 코드 구현함.
//* this_thread::sleep_for를 asio::timer로 변경하는 방법 추가 구현 필요

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

void ConnCallback(const asio::error_code& ec,
                  std::shared_ptr<asio::ip::tcp::socket> sock,
                  asio::ip ::tcp::endpoint& ep,
                  std::shared_ptr<asio::io_context> ioc) {
    if (ec.value() != 0) {
        if (ec == asio::error::operation_aborted) {
            std::cout << "Operation cancelled!" << std::endl;
            ;
        } else {
            std::cout << "In async_connect!"
                      << " Error code = " << ec.value()
                      << ". Message: " << ec.message() << std::endl;
            ;
        }
    } else {
        std::cout << "Connected" << std::endl;

        Communicate(*sock);
        return;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(200));
    std::cout << "Retry connect" << std::endl;

    sock = std::make_shared<asio::ip::tcp::socket>(*ioc, ep.protocol());
    sock->async_connect(
        ep, std::bind(ConnCallback, std::placeholders::_1, sock, ep, ioc));
}

int main() {
    std::string raw_ip_address = "121.139.55.100";
    unsigned short port_num    = 3333;
    std::shared_ptr<asio::io_context> ioc =
        std::make_shared<asio::io_context>();
    asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address),
                               port_num);

    //* 1. 소켓 할당
    std::shared_ptr<asio::ip::tcp::socket> sock =
        std::make_shared<asio::ip::tcp::socket>(*ioc, ep.protocol());
    // asio::ip::tcp::socket::reuse_address option_ra(true);
    // sock->set_option(option_ra);
    // asio::ip::tcp::socket::keep_alive option_ka(true);
    // sock->set_option(option_ka);

    //* 2. 소켓 연결
    //! ioc의 경우, shared_ptr이 아닌 reference로 할 경우 오류발생함.
    sock->async_connect(
        ep, std::bind(ConnCallback, std::placeholders::_1, sock, ep, ioc));
    ioc->run();
    return 0;
}
