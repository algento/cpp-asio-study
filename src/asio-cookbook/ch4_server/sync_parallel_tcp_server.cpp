/**
 * @file sync_parallel_tcp_server.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <atomic>
#include <iostream>
#include <memory>
#include <thread>

#include <asio.hpp>
/**
 * @brief 클라이언트에서 들어오는 요청을 별도의 스레드로 대기하고 요청이 들어오면 응답한 후 해제되는 클래스
 * 
 */
class Service {
 public:
    Service() = default;
    /**
     * @brief 소켓을 입력받아 메인 스레드와 분리된 별도의 스레드로 HandleClient를 실행한다. 
     * 분리된 스레드는 프로세스가 종료되면 자체적으로 반환된다.
     * 
     * @param sock 
     */
    void StartHandlingClient(std::shared_ptr<asio::ip::tcp::socket> sock) {
        std::thread th(([this, sock]() { HandleClient(sock); }));

        th.detach();
    }

 private:
    /**
     * @brief 별도의 스레드로 호출되어 클라이언트에서 요청을 읽고 대기 후 응답한다. 응답한 후 힙에 있는 자기자신를 해제한다.
     * 
     * @param sock 
     */
    void HandleClient(std::shared_ptr<asio::ip::tcp::socket> sock) {
        try {
            asio::streambuf request;
            asio::read_until(*sock.get(), request, '\n');

            // Emulate request processing.
            int i = 0;
            while (i != 1000000) i++;

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // Sending response.
            std::string response = "Response\n";
            asio::write(*sock.get(), asio::buffer(response));
        } catch (asio::system_error& e) {
            std::cout << "Error occured! Error code = " << e.code()
                      << ". Message: " << e.what();
        }

        // Clean-up.
        delete this;
    }
};

/**
 * @brief 서비스 객체를 보유하지 않는다. 서비스 객체가 자기 할일 이 끝나면 스스로 자신을 소멸시킨다.
 * 
 */
class Acceptor {
 public:
    Acceptor(asio::io_context& ioc, unsigned int port_num)
        : m_ioc(ioc),
          m_acceptor(m_ioc, asio::ip::tcp::endpoint(asio::ip::address_v4::any(),
                                                    port_num)) {
        m_acceptor.listen();
    }

    /**
     * @brief 소켓을 힙에 생성하고 클라이언트와 연결을 수립힌다. (연결들어올 때까지 대기)
     * 연결이 수립되면 클라이언트에서 요청을 받아 처리한다. 힙에 생성한 서비스는 StartHandlingClient에서 해제된다.
     * 
     */
    void Accept() {
        std::shared_ptr<asio::ip::tcp::socket> sock(
            new asio::ip::tcp::socket(m_ioc));

        m_acceptor.accept(*sock);

        (new Service)->StartHandlingClient(sock);
    }

 private:
    asio::io_context& m_ioc;
    asio::ip::tcp::acceptor m_acceptor;
};

class Server {
 public:
    Server() : m_stop(false) {}

    void Start(unsigned short port_num) {
        m_thread = std::make_unique<std::thread>(
            [this, port_num]() { Run(port_num); });
    }

    void Stop() {
        m_stop.store(true);
        m_thread->join();
    }

 private:
    void Run(unsigned short port_num) {
        Acceptor acc(m_ioc, port_num);

        while (!m_stop.load()) {
            acc.Accept();
        }
    }
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_stop;
    asio::io_context m_ioc;
};

int main() {
    unsigned short port_num = 3333;

    try {
        Server srv;
        srv.Start(port_num);

        std::this_thread::sleep_for(std::chrono::seconds(60));

        srv.Stop();
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
    }

    return 0;
}