/**
 * @file simple_example1.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-15
 * https://github.com/OneLoneCoder/Javidx9/tree/master/PixelGameEngine/BiggerProjects/Networking
 * https://www.youtube.com/watch?v=2hNdkYInj4g&list=PLrOv9FMX8xJEEQFU_eAvYTsyudrewymNl&index=15
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>
#include <thread>

#include <asio.hpp>

#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/address.hpp"
#include "asio/ip/tcp.hpp"

//* 우선 우리가 사용하기에 충분히 큰 버퍼를 만든다.
std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket) {
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
                           [&](std::error_code ec, std::size_t length) {
                               //!    [&](std::error_code& ec, std::size_t length) {
                               if (!ec) {
                                   std::cout << "\n\nRead " << length
                                             << "bytes\n\n";
                                   for (int i = 0; i < length; i++) {
                                       std::cout << vBuffer[i];
                                       GrabSomeData(socket);
                                   }
                               }
                           });
}

int main() {
    // 오류 확인이 매우 편리하게 되어있다.
    asio::error_code ec;

    // (플랫폼과 관련된 특수 인터페이스에 대한) "context"를 생성한다. "context"는 등록된 핸들을 처리할 수 있는 능력을 가지고 있다.
    asio::io_context context;

    //* Give some fake tasks to asio so the context doesn't finish
    //* https://chelseafandev.github.io/2022/01/11/prevent-io-context-run-from-returning/
    using work_guard_type =
        asio::executor_work_guard<asio::io_context::executor_type>;
    work_guard_type work_guard(context.get_executor());

    //! asio::io_context::work idleWork(context); // 에전 코드

    //* Start Context
    std::thread thrContext = std::thread([&]() { context.run(); });

    // 종료점 생성
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec),
                                     80);

    asio::ip::tcp::socket socket(context);

    socket.connect(endpoint, ec);

    if (ec.value() == 0) {
        std::cout << "Connected " << std::endl;
    } else {
        std::cout << "Error Code :" << ec.value()
                  << " Failed to connect to address: \n"
                  << ec.message() << std::endl;
    }

    if (socket.is_open()) {
        GrabSomeData(socket);

        std::string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        // 소켓 버퍼에 위의 sRequest를 쓰고 일부를 보낸다.
        size_t bytes_written = socket.write_some(
            asio::buffer(sRequest.data(), sRequest.size()), ec);

        //! 연결은 되지만 바로 종료된다. async_read_some은 호출되자 마자 종료되기 때문이다.
        // GrabSomeData(socket);

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2000ms);

        context.stop();
        if (thrContext.joinable()) {
            thrContext.join();
        }
    }
    return 0;
}