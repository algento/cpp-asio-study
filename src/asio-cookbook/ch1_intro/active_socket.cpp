/**
 * @file active_socket.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>

#include <asio.hpp>

int main() {
    // Step 1. io_context 인스턴스 생성
    asio::io_context ioc;

    // Step 2. tcp 객체 생성 (프로토콜 선택)
    asio::ip::tcp protocol = asio::ip::tcp::v4();

    // Step 3. active tcp 소켓 객체 인스턴스 생성
    asio::ip::tcp::socket sock(ioc);

    // 소켓을 여는 동안 발생할 수 있는 오류 정보 저장
    asio::error_code ec;

    // Step 4. 소켓 오픈
    sock.open(protocol, ec);

    if (ec.value() != 0) {
        std::cout << "Failed to open the socket! Error code = " << ec.value()
                  << ". Message: " << ec.message();
        return ec.value();
    }
}