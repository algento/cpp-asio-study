/**
 * @file passive_tcp_socket.cpp
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
int main() {
    // Step 1. io_context 인스턴스 생성
    asio::io_context ioc;

    // Step 2. tcp 객체 생성 (프로토콜 선택)
    asio::ip::tcp protocol = asio::ip::tcp::v6();

    // Step 3. acceptor socket 인스턴스화
    asio::ip::tcp::acceptor acceptor(ioc);

    // 소켓을 여는 동안 발생할 수 있는 오류 정보 저장
    asio::error_code ec;

    // Step 4. acceptor socket 오픈
    // - acceptor socket이 운영체제에서 할당되며 앞으로 들어오는 연결을 받아들이기 위해 listen을 시작할 수 있다.
    acceptor.open(protocol, ec);

    if (ec.value() != 0) {
        std::cout << "Failed to open the acceptor socket!"
                  << "Error code =" << ec.value()
                  << ". Message: " << ec.message();
        return ec.value();
    }
    return 0;
}