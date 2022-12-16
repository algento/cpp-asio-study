/**
 * @file simple_example.cpp
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

#include "asio/io_context.hpp"
#include "asio/ip/address.hpp"
#include "asio/ip/tcp.hpp"

int main() {
    // 오류 확인이 매우 편리하게 되어있다.
    asio::error_code ec;

    // (플랫폼과 관련된 특수 인터페이스에 대한) "context"를 생성한다.
    asio::io_context context;

    // 연결하기 원하는 곳에 대한 종료점을 생성한다.
    //* case 1
    //* "93.184.216.34"는 example.com이라는 도메인의 ip 주소이고 port 번호 80은 http를 의미한다. 수신되는 정보가 적어서 한번에 수신된다.
    // asio::ip::tcp::endpoint endpoint(
    //     asio::ip::make_address("93.184.216.34", ec), 80);

    //! case 2
    //! "127.0.0.1"은 local host로 자기 자신을 말한다. 여기에 연결하면 연결이 거부되므로 에러코드가 출력된다.
    // asio::ip::tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1", ec),
    //                                  80);

    //* case 3
    //* "51.38.81.49"는 community.onelonecoder.com이다. 수신되는 정보가 많아서 한번에 모두 읽지 못한다.
    //* 네트웨크에서 어떻게 데이터를 수신할 지 우리는 제어할 수 없다. 그것은 OS가 담당한다. 서버가 얼마만큼의 데이터를 회신할 지를 모른다면 async 동작을 고려해보자.
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec),
                                     80);

    // 소켓 생성, "context"가 구현을 플랫폼(OS)의 네트워크 드리아버에 전달한다.
    asio::ip::tcp::socket socket(context);

    // 논리적 연결 수립
    socket.connect(endpoint, ec);

    if (ec.value() == 0) {
        std::cout << "Connected " << std::endl;
    } else {
        std::cout << "Error Code :" << ec.value()
                  << " Failed to connect to address: \n"
                  << ec.message() << std::endl;
    }

    if (socket.is_open()) {
        std::cout << "Socket Open" << std::endl;
        std::string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        // 소켓 버퍼에 위의 sRequest를 쓰고 일부를 보낸다.
        size_t bytes_written = socket.write_some(
            asio::buffer(sRequest.data(), sRequest.size()), ec);

        if (ec.value() == 0) {
            std::cout << "Write " << bytes_written << " bytes in size "
                      << sRequest.size() << std::endl;
        } else {
            std::cout << "Error Code :" << ec.value() << " Failed to write: \n"
                      << ec.message() << std::endl;
        }

        //! 쓰자 마자 소켓의 가용한 버퍼 수를 가져오면 0이 된다. 디버깅을 하는 등의 스레드 블럭이 들어가면 정상동작한다. 따라서 아래의 코드를 추가한다.
        //* 아래처럼 강제로 대기 시간을 주는 하드코딩은 좋지 않다.
        // using namespace std::chrono_literals;
        // std::this_thread::sleep_for(1000ms);

        //* asio에서는 읽을 때까지 대기하는 함수를 제공한다.
        socket.wait(socket.wait_read);

        size_t bytes = socket.available();
        std::cout << "Bytes Available: " << bytes << std::endl;

        if (bytes > 0) {
            std::vector<char> vBuffer(bytes);
            socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
            for (auto c : vBuffer) {
                std::cout << c;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}