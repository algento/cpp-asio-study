/**
 * @file sync_tcp_client.cpp
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

// 별도의 오류처리를 하지 않는다.
// asio에서 사용하는 예외처리 기능을 사용할 뿐이므로 사용자가 이 예외를 잡아서 처리해야 한다.
class SyncTCPClient {
 public:
    SyncTCPClient(const std::string& raw_ip_address, unsigned short port_num)
        : m_ep(asio::ip::make_address(raw_ip_address), port_num),
          m_sock(m_ioc) {
        m_sock.open(m_ep.protocol());
    }

    void Connect() { m_sock.connect(m_ep); }

    void Close() { m_sock.shutdown(asio::ip::tcp::socket::shutdown_both); }

    std::string EmulateLongComputationOp(unsigned int duration_sec) {
        //* 요청 커맨드 생성
        std::string request =
            "EMULATE_LONG_COMP_OP " + std::to_string(duration_sec) + "\n";

        //* 요청 커멘드 전송
        SendRequest(request);

        //* 응답 수신
        return ReceiveResponse();
    }

 private:
    void SendRequest(const std::string& request) {
        asio::write(m_sock, asio::buffer(request));
    }

    std::string ReceiveResponse() {
        asio::streambuf buf;
        asio::read_until(m_sock, buf, '\n');

        std::istream input(&buf);

        std::string response;
        std::getline(input, response);

        return response;
    }

    asio::io_context m_ioc;
    asio::ip::tcp::endpoint m_ep;
    asio::ip::tcp::socket m_sock;
};

int main() {
    const std::string raw_ip_address = "127.0.0.1";
    const unsigned short port_num    = 3333;

    try {
        SyncTCPClient client(raw_ip_address, port_num);

        // Sync connect.
        client.Connect();

        std::cout << "Sending request to the server... " << std::endl;

        std::string response = client.EmulateLongComputationOp(10);

        std::cout << "Response received: " << response << std::endl;

        // Close the connection and free resources.
        client.Close();
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();

        return e.code().value();
    }

    return 0;
}