/**
 * @file active_tcp_socket.cpp
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
    // - 운영체제가 제공하는 네트워크 I/O 서비스에 접근 허용
    asio::io_context ioc;

    // Step 2. tcp 객체 생성 (프로토콜 선택)
    // - 프로토콜 타입 값을 갖는 data structure
    // - asio::ip::tcp::v4()/asio::ip::tcp::v6()
    asio::ip::tcp protocol = asio::ip::tcp::v4();

    // Step 3. active tcp 소켓 객체 인스턴스 생성
    // - asio 소켓 객체가 생성될 뿐이지 실제 운영체제에서 제공하는 소켓은 할당되지 않음
    asio::ip::tcp::socket sock(ioc);

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

    //* Step 3 and 4: socket(io_context, protocol)로 한번에 처리할 수 있다.
    // try {
    //     asio::ip::tcp::socket sock(ioc, protocol);
    // } catch (asio::system_error& e) {
    //     std::cout << "Error occured! Error code = " << e.code()
    //               << ". Message: " << e.what() << std::endl;
    // }
    // return 0;
}