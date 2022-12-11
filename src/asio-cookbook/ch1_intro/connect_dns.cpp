/**
 * @file connect_dns.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//* Connecting a Socket with a DNS name
#include <iostream>

#include <asio.hpp>

int main() {
    // Step1. Assume that the client application has already
    // obtained the DNS name and protocol port number and
    // represented them as strings.
#if 0  //! error 예외 발생 확인 가능
    // std::string host     = "samplehost.book";
    // std::string port_num = "3333";
#else
    std::string host     = "naver.com";
    std::string port_num = "https";
#endif

    // Used by a 'resolver' and a 'socket'.
    asio::io_context ioc;

    // Creating a resolver's query.

    // Creating a resolver.
    asio::ip::tcp::resolver resolver(ioc);

    try {
        // Step 2. Resolving a DNS name.
        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(host, port_num);

        // Step 3. Creating a socket.
        asio::ip::tcp::socket sock(ioc);

        //* socket을 만들지만 open할 필요없다. asio::connect 메서드가 알아서 처리해준다.
        //* IP 주소가 어떤 프로토콜인지 모르기 때문에 일일이 확인해보고 열어야 하는데, 위의 메서드가
        //* 해당작업을 해준다.
        // Step 4. asio::connect() method iterates over
        // each endpoint until successfully connects to one
        // of them. It will throw an exception if it fails
        // to connect to all the endpoints or if other
        // error occurs.
        asio::connect(sock, endpoints);

    }
    // Overloads of asio::ip::tcp::resolver::resolve and
    // asio::connect() used here throw
    // exceptions in case of error condition.
    catch (asio::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what() << std::endl;

        return e.code().value();
    }

    return 0;
}