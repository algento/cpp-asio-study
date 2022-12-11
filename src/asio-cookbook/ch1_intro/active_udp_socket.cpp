/**
 * @file active_udp_socket.cpp
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

#include "asio/system_error.hpp"

int main() {
    // Step 1. io_context 인스턴스 생성
    // - 운영체제가 제공하는 네트워크 I/O 서비스에 접근 허용
    asio::io_context ioc;

    // Step 2. udp 객체 생성 (프로토콜 선택)
    // - 프로토콜 타입 값을 갖는 data structure
    // - asio::ip::udp::v4()/asio::ip::udp::v6()
    asio::ip::udp protocol = asio::ip::udp::v6();

    // Step 3. active tcp 소켓 객체 인스턴스 생성
    // - asio 소켓 객체가 생성될 뿐이지 실제 운영체제에서 제공하는 소켓은 할당되지 않음
    asio::ip::udp::socket sock(ioc);

    // 소켓을 여는 동안 발생할 수 있는 오류 정보 저장
    asio::error_code ec;

    // Step 4. 소켓 오픈
    // - 실제 운영체제의 소켓 할당
    // - 소켓에 관련 파라미터 연결
    sock.open(protocol, ec);

    if (ec.value() != 0) {
        std::cout << "Failed to open the socket! Error code = " << ec.value()
                  << ". Message: " << ec.message();
        return ec.value();
    }
}