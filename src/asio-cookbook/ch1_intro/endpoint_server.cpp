#include <iostream>

#include <asio.hpp>

#include "asio/ip/address.hpp"

int main() {
    // Step 1. 서버 프로그램이 포트 번호를 알고 있다고 가정
    unsigned short port_num = 3333;

    // Step 2. asio::ip::address 클래스 생성
    // 모든 v6 프로토콜에 대한 IP를 수신가능하도록 any로 설정 (서버가 ip6로 동작한다고 가정)
    // 서버 프로그램은 프로토콜을 명시해야 한다.
    asio::ip::address ip_address = asio::ip::address_v6::any();

    // Step 3.
    // asip::ip::tcp = asio::ip::basic_endpoint<tcp>
    asio::ip::tcp::endpoint ep(ip_address, port_num);

    // Step 4. The endpoint is created and can be used to
    // specify the ip-addresses and a port number on which
    // the server application wants to listen to incoming
    // connections.

    return 0;
}
