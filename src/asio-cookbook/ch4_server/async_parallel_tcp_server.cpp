/**
 * @file async_parallel_tcp_server.cpp
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

#include "asio/error_code.hpp"
#include "asio/io_context.hpp"
#include "asio/read_until.hpp"
#include "asio/streambuf.hpp"

using work_guard_type =
    asio::executor_work_guard<asio::io_context::executor_type>;

class Service {
 public:
    explicit Service(std::shared_ptr<asio::ip::tcp::socket> sock)
        : m_sock(sock) {}

    /**
     * @brief 클라이언트에서 들어오는 비동기 요청을 읽고 OnRequestReceived를 호출하여 요청을 처리한다.
     * 
     */
    void StartHandling() {
        asio::async_read_until(
            *m_sock, m_request, '\n',
            [this](const asio::error_code& ec, std::size_t bytes_transfered) {
                OnRequestReceived(ec, bytes_transfered);
            });
    }

 private:
    /**
     * @brief 요청을 받아들여 비정상이면 서비스를 종료하고 정상이면 ProcessRequest를 호출하여 요청을 처리한다. 
     * 처리한 요청에 대한 응답을 비동기 쓰기 연산으로 서버로 보내고 비동기 연산이 어떻게든 종료되면 OnResponsesSent를 호출한다.
     * 
     * @param ec 
     * @param bytes_transfered 
     */
    void OnRequestReceived(const asio::error_code& ec,
                           std::size_t bytes_transfered) {
        if (ec.value() != 0) {
            std::cout << "Error occured! Error code = " << ec.value()
                      << ". Message: " << ec.message();

            OnFinish();
            return;
        }

        // Process the request.
        m_response = ProcessRequest(m_request);

        // Initiate asynchronous write operation.
        asio::async_write(
            *m_sock, asio::buffer(m_response),
            [this](const asio::error_code& ec, std::size_t bytes_transferred) {
                OnResponseSent(ec, bytes_transferred);
            });
    }

    /**
     * @brief 응답이 정상적으로 보내졌으면 서비스를 종료하고 아니면 추가로 오류를 출력한다.
     * 
     * @param ec 
     * @param bytes_transfered 
     */
    void OnResponseSent(const asio::error_code& ec,
                        std::size_t bytes_transfered) {
        if (ec.value() != 0) {
            std::cout << "Error occured! Error "
                         "code = "
                      << ec.value() << ". Message: " << ec.message();
        }

        OnFinish();
    }

    /**
     * @brief 서비스 객체를 헤제한다.
     * 
     */
    void OnFinish() { delete this; }

    /**
     * @brief 요청에 대한 처리를 모사한다.
     * 
     * @param request 
     * @return std::string 
     */
    std::string ProcessRequest(asio::streambuf& request) {
        // In this method we parse the request, process it and prepare the request.

        // Emulate CPU-consuming operations.
        int i = 0;
        while (i != 1000000) i++;

        // Emulate operations that block the thread
        // (e.g. synch I/O operations).
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Prepare and return the response message.
        std::string response = "Response\n";
        return response;
    }

    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    std::string m_response;
    asio::streambuf m_request;
};

class Acceptor {
 public:
    Acceptor(asio::io_context& ioc, unsigned short port_num)
        : m_ioc(ioc),
          m_acceptor(m_ioc, asio::ip::tcp::endpoint(asio::ip::address_v4::any(),
                                                    port_num)),
          m_is_stopped(false) {}

    /**
     * @brief 수용자 소켓이 클라이언트의 연결 수립 요청을 들을 수 있도록 listen 상태로 변경하고 InitAccept을 호출한다.
     * 
     */
    void Start() {
        m_acceptor.listen();
        InitAccept();
    }

    void Stop() { m_is_stopped.store(true); }

 private:
    /**
     * @brief 소켓 객체를 힙에 만들고 async_accept을 수행한다.
     * 
     */
    void InitAccept() {
        std::shared_ptr<asio::ip::tcp::socket> sock(
            new asio::ip::tcp::socket(m_ioc));

        m_acceptor.async_accept(*sock,
                                [this, sock](const asio::error_code& error) {
                                    OnAccept(error, sock);
                                });
    }

    /**
     * @brief async_accept가 어떻게든 종료되었을 때 호출되는 콜백함수, 정상이면 서비스 객체를 생성하여 비동기 요청 읽기 연산을 시작한다.
     * 중단 명령이 들어왔는지 플래그를 확인하고 중단 명령이 들어오지 않았으면 다시 연결 수립할 수 있도록 대기히기 위해 InitAccept()를 호출한다. 중단 명령이 들어왔으면 수용자 소켓을 닫는다.
     * @param ec 
     * @param sock 
     */
    void OnAccept(const asio::error_code& ec,
                  std::shared_ptr<asio::ip::tcp::socket> sock) {
        if (ec.value() == 0) {
            (new Service(sock))->StartHandling();
        } else {
            std::cout << "Error occured! Error code = " << ec.value()
                      << ". Message: " << ec.message();
        }

        // Init next async accept operation if
        // acceptor has not been stopped yet.
        if (!m_is_stopped.load()) {
            InitAccept();
        } else {
            // Stop accepting incoming connections
            // and free allocated resources.
            m_acceptor.close();
        }
    }

    asio::io_context& m_ioc;
    asio::ip::tcp::acceptor m_acceptor;
    std::atomic<bool> m_is_stopped;
};

class Server {
 public:
    Server() {
        m_work = std::make_unique<work_guard_type>(m_ioc.get_executor());
    }

    /**
     * @brief 서버를 시작한다. 수용자 객체를 생성하고 시작한다. 스레드 풀을 이용하여 비동기 연산이 병렬로 처리될 수 있도록 한다.
     * 
     * @param port_num 
     * @param thread_pool_size 
     */
    void Start(unsigned short port_num, unsigned int thread_pool_size) {
        assert(thread_pool_size > 0);

        // Create and start Acceptor.
        acc = std::make_unique<Acceptor>(m_ioc, port_num);
        acc->Start();

        // Create specified number of threads and
        // add them to the pool.
        for (unsigned int i = 0; i < thread_pool_size; i++) {
            std::unique_ptr<std::thread> th(
                new std::thread([this]() { m_ioc.run(); }));

            m_thread_pool.push_back(std::move(th));
        }
    }

    /**
     * @brief 서버를 중단시칸다. 수용자 객체를 중단하고 io_context를 중단한 후, 스레드 풀의 스레드들을 join한다.
     * 
     */
    void Stop() {
        acc->Stop();
        m_ioc.stop();

        for (auto& th : m_thread_pool) {
            th->join();
        }
    }

 private:
    asio::io_context m_ioc;
    std::unique_ptr<work_guard_type> m_work = nullptr;
    std::unique_ptr<Acceptor> acc;
    std::vector<std::unique_ptr<std::thread>> m_thread_pool;
};

const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;

int main() {
    unsigned short port_num = 3333;

    try {
        Server srv;

        unsigned int thread_pool_size = std::thread::hardware_concurrency() * 2;

        if (thread_pool_size == 0) {
            thread_pool_size = DEFAULT_THREAD_POOL_SIZE;
        }

        srv.Start(port_num, thread_pool_size);

        std::this_thread::sleep_for(std::chrono::seconds(60));

        srv.Stop();
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();
    }

    return 0;
}