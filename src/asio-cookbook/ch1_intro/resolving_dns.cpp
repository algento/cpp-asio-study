/**
 * @file resolving_dns.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-10
 * https://nanxiao.gitbooks.io/boost-asio-network-programming-little-book/content/posts/dns-query.html
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>

#include <asio.hpp>

#include "asio/io_context.hpp"

int main() {
    // Step 1. 클라이언트 프로그램이 서버 프로그램의 DNS 이름과 포트 번호를 안다고 가정
    std::string host   = "naver.com";
    std::string sevice = "https";

    // Step 2.
    asio::io_context ioc;

    // 소켓을 여는 동안 발생할 수 있는 오류 정보 저장
    asio::error_code ec;

    // Step 3. resolver 생성
    asio::ip::tcp::resolver resolver(ioc);
    // asio::ip::udp::resolver resolver(ioc);

    // Step 4. host, service 등록하여 endpoints 수신
    asio::ip::tcp::resolver::results_type endpoints =
        // asio::ip::udp::resolver::results_type endpoints =
        resolver.resolve(host, sevice, ec);

    for (const auto& endpoint : endpoints) {
        std::cout << endpoint.endpoint() << '\n';
    }

    return 0;
}