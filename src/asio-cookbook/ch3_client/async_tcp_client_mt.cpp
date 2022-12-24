/**
 * @file async_tcp_client_mt.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include <asio.hpp>

#include "asio/detail/noncopyable.hpp"
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include "asio/executor.hpp"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/address.hpp"
#include "asio/streambuf.hpp"
#include "asio/system_error.hpp"

using Callback = void (*)(unsigned int request_id, const std::string& response,
                          const asio::error_code& ec);

/* Function pointer type that points to the callback function which is called when a request is complete. */
// typedef void (*Callback)(unsigned int request_id, const std::string& response,
//                          const system::error_code& ec);

using work_guard_type =
    asio::executor_work_guard<asio::io_context::executor_type>;

// Structure represents a context of a single request.
struct Session {
    Session(asio::io_context& ioc, const std::string& raw_ip_address,
            unsigned short port_num, const std::string& request,
            unsigned int id, Callback callback)
        : m_sock(ioc),
          m_ep(asio::ip::make_address(raw_ip_address), port_num),
          m_request(request),
          m_id(id),
          m_callback(callback) {}

    asio::ip::tcp::socket m_sock;
    asio::ip::tcp::endpoint m_ep;
    std::string m_request;

    asio::streambuf m_response_buf;
    std::string m_response;
    asio::error_code m_ec;

    unsigned int m_id;  // Unique ID assigned to the request.
    Callback
        m_callback;  //Pointer to the function to be called when the request completes.

    bool m_was_canceled = false;
    std::mutex m_cancel_guard;
};

class AsyncTCPClientMT : public asio::noncopyable {
 public:
    /**
     * @brief work_guard와 스레드를 시작한다.
     * work_guard는 남은 비동기 연산이 없더라도 io_context가 이벤트 루프를 끝내지 않도록 막는다.
     */
    explicit AsyncTCPClientMT(unsigned char num_of_threads) {
        m_work = std::make_unique<work_guard_type>(m_ioc.get_executor());
        for (auto i = 1; i <= num_of_threads; i++) {
            m_threads.push_back(
                std::make_unique<std::thread>([this]() { m_ioc.run(); }));

            //? 아래와 차이가 있나?
            // std::unique_ptr<std::thread> th(
            //     new std::thread([this]() { m_ioc.run(); }));
            // m_threads.push_back(std::move(th));
        }
    }

    /**
     * @brief 서버로 요청을 보낸다.
     * 
     * @param duration_sec 서버에 요청하는데 필요한 파라미터
     * @param raw_ip_address 스트링으로 구성된 ip 주소
     * @param port_num  포트 번호
     * @param callback 요청이 완료되면 호출할 콜백 함수 (함수 포인터)
     * @param request_id 요청 고유 식별자
     */
    void EmulateLongComputationOp(unsigned int duration_sec,
                                  const std::string& raw_ip_address,
                                  unsigned short port_num, Callback callback,
                                  unsigned int request_id) {
        // Preparing the request string.
        std::string request =
            "EMULATE_LONG_CALC_OP " + std::to_string(duration_sec) + "\n";

        //* 세션 객체 생성 후,
        std::shared_ptr<Session> session = std::make_shared<Session>(
            m_ioc, raw_ip_address, port_num, request, request_id, callback);

        //* 세션의 소켓 오픈
        session->m_sock.open(session->m_ep.protocol());

        // Add new session to the list of active sessions so
        // that we can access it if the user decides to cancel
        // the corresponding request before it completes.
        // Because active sessions list can be accessed from
        // multiple threads, we guard it with a mutex to avoid
        // data corruption.

        // 중간에 요청을 취소할 수 있도록 새 세션을 만들면 m_active_session에 등록한다.
        // 여러 세션을 관리하는 컨테이너를 두고 여러 스레드에서 이 컨테이너에 안전하게 접근할 수 있도록 mutex lock을 한다.
        std::unique_lock<std::mutex> lock(m_active_sessions_guard);
        m_active_sessions[request_id] = session;
        lock.unlock();

        // 세션의 소켓을 이용해서 비동기 연결을 시도한다.
        // 비동기 연결 완료 시 수행하는 핸들러에서는 async_write을 시도한다.
        session->m_sock.async_connect(
            session->m_ep, [this, session](const asio::error_code& ec) {
                if (ec.value() != 0) {
                    // 비동기 연결이 비정상 종료된 경우, OnRequestComplete을 수행한다.
                    session->m_ec = ec;
                    OnRequestComplete(session);
                    return;
                }

                // 연산 취소 플래그를 확인하기 위해 mutext lock을 수행한다.
                std::unique_lock<std::mutex> cancel_lock(
                    session->m_cancel_guard);

                // 요청이 취소되지 않았으면 OnRequestComplete를 수행한다.
                if (session->m_was_canceled) {
                    OnRequestComplete(session);
                    return;
                }

                // 연산 취소 플래그가 거짓인 경우, 비동기 쓰기를 시도한다.
                asio::async_write(
                    session->m_sock, asio::buffer(session->m_request),
                    [this, session](const asio::error_code& ec,
                                    std::size_t bytes_transferred) {
                        if (ec.value() != 0) {
                            session->m_ec = ec;
                            OnRequestComplete(session);
                            return;
                        }

                        std::unique_lock<std::mutex> cancel_lock(
                            session->m_cancel_guard);

                        if (session->m_was_canceled) {
                            OnRequestComplete(session);
                            return;
                        }

                        asio::async_read_until(
                            session->m_sock, session->m_response_buf, '\n',
                            [this, session](const asio::error_code& ec,
                                            std::size_t bytes_transferred) {
                                if (ec.value() != 0) {
                                    session->m_ec = ec;
                                } else {
                                    std::istream strm(&session->m_response_buf);
                                    std::getline(strm, session->m_response);
                                }

                                OnRequestComplete(session);
                            });
                    });
            });
    }

    /**
     * @brief 입력 인자로 받은 아이디로 지정된 요청을 취소한다.
     * 
     * @param request_id 
     */
    void CancelRequest(unsigned int request_id) {
        std::unique_lock<std::mutex> lock(m_active_sessions_guard);
        //* 들어온 요청 ID에 해당하는 세션이 있는지 찾는다.
        auto it = m_active_sessions.find(request_id);

        //* 세션을 찾았으면, 요청 취소를 위한 mutex lock을 설정한다.
        //* 콜백함수에서 플래그를 검사하기 전에 요청이 취소되거나, 다음 비동기 연산이 시작된 후에 요청을 취소하도록 순서를 강제한다.?
        if (it != m_active_sessions.end()) {
            std::unique_lock<std::mutex> cancel_lock(
                it->second->m_cancel_guard);

            // 해당 요청이 취소되었다는 플래그가 없으면, 비동기 연산이 완료되었지만
            // 다음 비동기 연산이 시작되지 않은 중간 단계에 대응할 수 없다.
            // 콜백 함수에서 실제 비동기 연산을 수행하기 전에 이 플래그를 확인하고
            // 플래그가 설정되어 있다면 요청 실행을 중지하고 OnRequestComplete()를 실행한다.
            it->second->m_was_canceled = true;

            // cancel한 소켓에서 처리하고 있던 비동기 연산이 취소된다.
            it->second->m_sock.cancel();
        }
    }

    /**
     * @brief 현재 실행 중이 요청이 모두 완료될 때까지 객체의 멤버인 스레드를 블럭하고 모두 종료되면 클라이언트를 정리한다.
     * 
     */
    void Close() {
        // Destroy work object. This allows the I/O thread to exits the event loop when there are no more pending asynchronous operations.
        // work를 리셋하면 모든 비동기 연산이 종료되었을 때, I/O 스레드가 이벤트 메세지 루프를 벗어날 수 있다.
        m_work.reset(nullptr);

        // Wait for the I/O thread to exit.
        for (auto& thread : m_threads) {
            thread->join();
        }
    }

 private:
    //* 요청이 어떤 식으로든 종료되었을 때 호출된다.
    /**
     * @brief 요청이 어떤 식으로든 종료되었을 때 호출하는 함수
     * 소켓을 shutdown하고 세션을 m_active_session 컨테이너에서 삭제한다.
     * 마지막으로 세션의 콜백을 호출해서 요청 처리를 완료한다.
     * 
     * @param session 요청을 취소할 세션
     */
    void OnRequestComplete(std::shared_ptr<Session> session) {
        // Shutting down the connection. This method may fail in case socket is not connected.
        // We don't care about the error code if this function fails.
        asio::error_code ignored_ec;

        //* 예외를 던지지 않도록 shutdown()을 사용하여 소켓을 종료한다.
        session->m_sock.shutdown(asio::ip::tcp::socket::shutdown_both,
                                 ignored_ec);

        //* m_active_sessions에 있는 세션 중 요청이 종료되어 더이상 필요하지 않은 세션을 찾아서 제거한다.
        // Remove session form the map of active sessions.
        std::unique_lock<std::mutex> lock(m_active_sessions_guard);

        auto it = m_active_sessions.find(session->m_id);

        if (it != m_active_sessions.end()) {
            m_active_sessions.erase(it);
        }

        lock.unlock();

        asio::error_code ec;

        if (session->m_ec.value() == 0 && session->m_was_canceled) {
            ec = asio::error::operation_aborted;
        } else {
            ec = session->m_ec;
        }

        //* 콜백 함수를 호출하여 요청 처리를 완료한다. (세션에 들어있던 정보를 가져와서 처리?)
        // Call the callback provided by the user.
        session->m_callback(session->m_id, session->m_response, ec);
    }

    asio::io_context m_ioc;
    /**
     * @brief 실행 중인 요청과 관련된 모든 세션 객체의 포인터가 저장된 컨테이너
     * <ID, 세션 포인터> 맵으로 구성되어 있다. 요청을 취소할 필요가 없다면 필요없는 멤버 변수
     * 
     */
    std::map<int, std::shared_ptr<Session>> m_active_sessions;
    std::mutex m_active_sessions_guard;
    // https://chelseafandev.github.io/2022/01/11/prevent-io-context-run-from-returning/
    std::unique_ptr<work_guard_type> m_work = nullptr;

    /**
     * @brief 각 스레드가 io_context::run()을 실행하는 스레드 풀
     * 동일한 io_context로 제어할 경우, 풀에 있는 모든 스레드가 비동기 연산 완료 후 콜백을 호출할 때 사용된다.
     * 단일 스레드는 비동기 연산이 하나 씩 실행된다면 스레드 풀을 사용할 경우 스레드 수만큼의 비동기 연산이 동시에 실행된다.
     */
    std::list<std::unique_ptr<std::thread>> m_threads;
};

/**
 * @brief EmulateLongComputationOp()에 인자로 전달할 콜백 함수, 요청이 성공적으로 끝나면 요청 실행 결과와 응답 메세지를 표준 출력스트림으로 출력한다.
 * 
 * @param request_id 요청의 고유 식별자
 * @param response 응답 데이터, 요청이 정상적으로 처리되었을 때만 정상적인 값을 갖는다.
 * @param ec 요청을 처리하는 동안 발생한 오류가 저장되는 코드
 */
void handler(unsigned int request_id, const std::string& response,
             const asio::error_code& ec) {
    if (ec.value() == 0) {
        std::cout << "Request #" << request_id
                  << " has completed. Response: " << response << std::endl;
    } else if (ec == asio::error::operation_aborted) {
        std::cout << "Request #" << request_id
                  << " has been cancelled by the user." << std::endl;
    } else {
        std::cout << "Request #" << request_id
                  << " failed! Error code = " << ec.value()
                  << ". Error message = " << ec.message() << std::endl;
    }
}

int main() {
    try {
        //* main 스레드가 사용자 입력을 처리하고 서버에 요청을 시작하는 UI 스레드 역할을 한다.
        AsyncTCPClientMT client(4);

        // Here we emulate the user's behavior.

        // User initiates a request with id 1.
        client.EmulateLongComputationOp(10, "127.0.0.1", 3333, handler, 1);
        // Then does nothing for 5 seconds.
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // Then initiates another request with id 2.
        client.EmulateLongComputationOp(11, "127.0.0.1", 3334, handler, 2);
        // Then decides to cancel the request with id 1.
        client.CancelRequest(1);
        // Does nothing for another 6 seconds.
        std::this_thread::sleep_for(std::chrono::seconds(6));
        // Initiates one more request assigning ID 3 to it.
        client.EmulateLongComputationOp(12, "127.0.0.1", 3335, handler, 3);
        // Does nothing for another 15 seconds.
        std::this_thread::sleep_for(std::chrono::seconds(15));
        // Decides to exit the application.
        client.Close();
    } catch (asio::system_error& e) {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what();

        return e.code().value();
    }
}