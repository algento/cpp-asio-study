/**
 * @file write_to_tcp_async.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>
#include <memory>

#include <asio.hpp>

#include "asio/completion_condition.hpp"
#include "asio/error_code.hpp"
#include "asio/ip/address.hpp"
#include "asio/write.hpp"

// Keeps objects we need in a callback to identify whether all data has been written
// to the socket and to initiate next async writing operatino if needed.
//* 콜백 함수를 사용할 때, 필요한 정보 (소켓, 쓸 데이터가 저장된 버퍼, 쓴 바이트 크기)를 저장하는 구조체
using asio::async_write;

struct Session {
    std::shared_ptr<asio::ip::tcp::socket> sock;
    std::string buf;
    std::size_t total_bytes_written;  // Enhanced 버전에서는 필요없다.
};

// Function used as a callback for asynchronous writing operation.
// Checks if all data from the buffer has been written to the socket and initiates
// new writting operation if needed.
void Callback(const asio::error_code &ec, std::size_t bytes_transfered,
              std::shared_ptr<Session> s) {
    if (ec.value() != 0) {
        std::cout << "Error occured! Error code = " << ec.value()
                  << ". Message: " << ec.message();

        return;
    }
    s->total_bytes_written += bytes_transfered;

    if (s->total_bytes_written == s->buf.length()) {
        return;
    }

    s->sock->async_write_some(
        asio::buffer(s->buf.c_str() + s->total_bytes_written,
                     s->buf.length() - s->total_bytes_written),
        std::bind(Callback, std::placeholders::_1, std::placeholders::_2, s));
}

void WriteToSocket(std::shared_ptr<asio::ip::tcp::socket> sock) {
    std::shared_ptr<Session> s(new Session);

    // Step 4. Allocating and filling the buffer.
    s->buf                 = std::string("Hello");
    s->total_bytes_written = 0;
    s->sock                = sock;

    // Step 5. Initiating asynchronous write opration.
    //* 해당 함수에서 async_write_some 함수로 콜백함수를 호출한다.
    //* 이 때, std::bind를 통해서 Session 구조테를 콜백함수로 전달한다.
    //* 콜백함수에서도 async_write_some 함수가 호출되어 반복적으로 모든 데이터를 쓸 때까지 콜백함수가 재귀적으로 호출된다.
    s->sock->async_write_some(
        asio::buffer(s->buf),
        std::bind(Callback, std::placeholders::_1, std::placeholders::_2, s));
}

void CallbackEnhanced(const asio::error_code &ec, std::size_t bytes_transfered,
                      std::shared_ptr<Session> s) {
    if (ec.value() != 0) {
        std::cout << "Error occured! Error code = " << ec.value()
                  << ". Message: " << ec.message();

        return;
    }
    // Here we know that all the data has
    // been written to the socket.
}

void WriteToSocketEnhanced(std::shared_ptr<asio::ip::tcp::socket> sock) {
    std::shared_ptr<Session> s(new Session);

    // Step 4. Allocating and filling the buffer.
    s->buf  = std::string("Hello");
    s->sock = sock;

    // Step 5. Initiating asynchronous write opration.
    asio::async_write(*sock.get(), asio::buffer(s->buf),
                      std::bind(CallbackEnhanced, std::placeholders::_1,
                                std::placeholders::_2, s));
    //* 다른 형태의 override (complete condition추가)
    // asio::async_write(*sock.get(), asio::buffer(s->buf), asio::transfer_all(),
    //                   std::bind(CallbackEnhanced, std::placeholders::_1,
    //                             std::placeholders::_2, s));
}

int main() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num    = 3333;

    try {
        asio::ip::address ip_address = asio::ip::make_address(raw_ip_address);
        asio::ip::tcp::endpoint ep(ip_address, port_num);
        asio::io_context ioc;

        // Step 3. Allocating, opening and connecting a socket.
        std::shared_ptr<asio::ip::tcp::socket> sock(
            new asio::ip::tcp::socket(ioc, ep.protocol()));

        sock->connect(ep);

        WriteToSocket(sock);

        ioc.run();

    } catch (asio::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what() << std::endl;

        return e.code().value();
    }
    return 0;
}