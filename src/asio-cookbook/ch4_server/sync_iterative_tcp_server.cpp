/**
 * @file sync_iterative_tcp_server.cpp
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

#include "asio/io_context.hpp"

/**
 * @brief 서버 프로그램이 제공하는 서비스, 여기서는 실제 연산을 수행하지 않고 수행하는 척 할 뿐이다.
 * 
 */
class Service {
 public:
    Service() = default;

    /**
     * @brief 입력받은 소켓을 이용하여 클라이언트에서 요청을 읽고 ('\n'까지) 500 ms 동안 대기하고 응답을 보낸다.
     * 내부에서 asio가 예외를 던진다면 해당 함수가 예외를 처리하기 때문에 상위 호출자에게 전파되지 않는다.
     * 이 때문에 서비스를 처리하다가 문제가 생기더라도 서버는 계속 일할 수 있다.
     * @param sock 
     */
    void HandleClient(asio::ip::tcp::socket& sock) {
        try {
            asio::streambuf request;
            asio::read_until(sock, request, '\n');

            // Emulate request processing.
            int i = 0;
            while (i != 1000000) {
                i++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // Sending response.
            std::string response = "Response\n";
            asio::write(sock, asio::buffer(response));
        } catch (asio::system_error& e) {
            std::cout << "Error occured! Error code = " << e.code()
                      << ". Message: " << e.what();
        }
    }
};

/**
 * @brief 고수준의 수용자 클래스, 클라이언트에서 들어오는 연결 요청을 받아들이고 연결된 클라이언트에 제공할 서비스를 생성한다.
 * 
 */
class Acceptor {
 public:
    /**
     * @brief io_context와 포트 번호를 입력받아서 수용자 소켓을 생성하고 들어오는 연결이 있는지 listen한다.
     * 
     * @param ioc 
     * @param port_num 
     */
    Acceptor(asio::io_context& ioc, unsigned short port_num)
        : m_ioc(ioc),
          m_acceptor(m_ioc, asio::ip::tcp::endpoint(asio::ip::address_v4::any(),
                                                    port_num)) {
        m_acceptor.listen();
    }

    /**
     * @brief 수용자 소켓에서 연결 요청을 대기하다가 들어오면 논리적 연결을 수립하고 서비스를 제공한다.
     * 
     */
    void Accept() {
        asio::ip::tcp::socket sock(m_ioc);

        m_acceptor.accept(sock);

        Service svc;
        svc.HandleClient(sock);
    }

 private:
    asio::io_context& m_ioc;
    asio::ip::tcp::acceptor m_acceptor;
};

/**
 * @brief 서버 클래스
 * 
 */
class Server {
 public:
    Server() : m_stop(false) {}

    /**
     * @brief 서버를 시작한다.스레드를 생성하고 서버 동작 (수용자를 만들고 클라이언트에서 오는 요청을 처리)을 시작한다.
     * 
     * @param port_num 
     */
    void Start(unsigned short port_num) {
        m_thread = std::make_unique<std::thread>(
            [this, port_num]() { Run(port_num); });
    }

    /**
     * @brief 서버를 멈춘다. 서버가 멈출 때까지 스레드를 블럭한다. 특정상황에서는 스레드가 끝나지 않을 수 있다.
     * 예를 들어, 클라이언트의 연결 요청을 대기하고 있는 상황에서 호출되면 새로운 연결 요청이 도착하기 전에는 
     * 종료되지 않는 응답없음(hang) 상태에 머물게 된다.이를 해결하기 위해서는 동기 연산에 시간 제헌울 거는 것이
     * 제일 좋지만 해당 기능은 asio에서 지원하지 않는다. (native socket을 받아서 timeout을 설정하는 것은 가능할까?)
     * asio의 지원을 받는 상황에서 이를 해결하는 방법은 다음의 방법 이 있다.
     * - 수용자 소켓이 열어놓은 포트로 무의미한 연결 요청을 보내는 것 (자기 자신에게 메세지 보내기)
     * - 서버를 종료시킬 수 있도록 메세지를 보내는 특수 클라이언트를 구현
     */
    void Stop() {
        m_stop.store(true);
        // TODO: 개선 코드 추가
        m_thread->join();
    }

 private:
    /**
     * @brief 수용자 객체를 생성하고 Stop() 호출되지 않는 한 클라이언트의 요청을 처리한다. 
     * 
     * @param port_num 
     */
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
