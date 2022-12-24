/**
 * @file sync_udp_client.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>

#include <asio.hpp>

#include "asio/io_context.hpp"
#include "asio/ip/address.hpp"

//* 별도의 연결을 수립하지 않는 UDP 프로토콜의 특성 때문에 이 클래스 하나로도 여러 서버와 통신할 수 있다.
class SyncUDPClient {
 public:
    SyncUDPClient() : m_sock(m_ioc) { m_sock.open(asio::ip::udp::v4()); }

    std::string EmulateLongComputationOp(unsigned int duration_sec,
                                         const std::string& raw_ip_address,
                                         unsigned short port_num) {
        std::string request =
            "EMULATE_LONG_COMP_OP " + std::to_string(duration_sec) + "\n";

        asio::ip::udp::endpoint ep(asio::ip::make_address(raw_ip_address),
                                   port_num);

        SendRequest(ep, request);
        return ReceiveResponse(ep);
    };

 private:
    void SendRequest(const asio::ip::udp::endpoint& ep,
                     const std::string& request) {
        //* 아래의 메서드는 버퍼 내용을 전부 송신하거나 오류가 발생할 때까지 호출한 스레드를 블럭한다.
        //* 하지만 메서드가 반환되었다고 해서 서버에 요청 메세지가 정상적으로 도착했다는 것은 알 수 없다.
        m_sock.send_to(asio::buffer(request), ep);
    }

    std::string ReceiveResponse(asio::ip::udp::endpoint& ep) {
        //* 응답을 수신할 버퍼
        char response[6];
        //* 서버에서 온 응답을 버퍼에 복사한다. 동기 방식이기 때문에 데이터가 들어올 때까지 호출한 스레드가 블럭된다.
        std::size_t bytes_recieved =
            m_sock.receive_from(asio::buffer(response), ep);

        //* 요청에 대한 응답을 받은 후, 소켓을 셧다운한다.
        m_sock.shutdown(asio::ip::udp::socket::shutdown_both);
        return {response, bytes_recieved};
    }

    asio::io_context m_ioc;
    asio::ip::udp::socket m_sock;
};

int main() {
    const std::string server1_raw_ip_address = "127.0.0.1";
    const unsigned short server1_port_num    = 3333;

    const std::string server2_raw_ip_address = "192.168.1.10";
    const unsigned short server2_port_num    = 3334;

    try {
        SyncUDPClient client;

        std::cout << "Sending request to the server #1 ... " << std::endl;

        std::string response = client.EmulateLongComputationOp(
            10, server1_raw_ip_address, server1_port_num);

        std::cout << "Response from the serever #1 received: " << response
                  << std::endl;

        std::cout << "Sending request to the server #2... " << std::endl;

        response = client.EmulateLongComputationOp(10, server2_raw_ip_address,
                                                   server2_port_num);

        std::cout << "Response from the server #2 received: " << response
                  << std::endl;
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();

        return e.code().value();
    }

    return 0;
}