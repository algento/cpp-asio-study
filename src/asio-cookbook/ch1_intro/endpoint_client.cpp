#include <iostream>

#include <asio.hpp>

#include "asio/ip/address.hpp"

int main() {
    // Step1. 클라이언트 프로그램이 서버의 ip와 포트를 알고 있다고 가정
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num    = 3333;

    // raw ip 주소 파싱 중 발생하는 오류 정보를 저장하기 위해 asio::error_code 객체 생성
    asio::error_code ec;

    // Step2. asio::ip::address 객체 생성
    // 3가지 ip 주소 객체: ip::address_v4, ip::address_v6, ip::address (protocol 버전 무관)
    // make_address: 주소를 받아서 ip::address 객체 생성
    asio::ip::address ip_address = asio::ip::make_address(raw_ip_address, ec);

    if (ec.value() != 0) {
        // 오류 코드 발생 시 출력
        std::cout << "Failed to parse the IP address. Error code = "
                  << ec.value() << ". Message: " << ec.message();
        return ec.value();
    }

    // Step 3.
    asio::ip::tcp::endpoint ep(ip_address, port_num);

    // Step 4. The endpoint is ready and can be used to specify a
    // particular server in the network the client wants to
    // communicate with.

    return 0;
}
