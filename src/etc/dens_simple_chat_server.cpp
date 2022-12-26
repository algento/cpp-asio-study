/**
 * @file dens_simple_chat_server.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <optional>
#include <queue>
#include <unordered_set>

#include <asio.hpp>

//* telnet을 호출하여 테스트 가능 
//* 별도로 ./server 실행하고 별도의 터미널에 telnet localhost 15001 실행
namespace io     = asio;
using tcp        = io::ip::tcp;
using error_code = io::error_code;
using namespace std::placeholders;

using message_handler = std::function<void(std::string)>;
using error_handler   = std::function<void()>;

/**
 * @brief 세션 클래스, 클라이언트와 연결된 소켓을 이용해여 비동기 읽기 연산을 수행한다.
 * 
 */
class session : public std::enable_shared_from_this<session> {
 public:
    session(tcp::socket&& socket) : socket(std::move(socket)) {}

    /**
     * @brief 세션 시작 (비동기 읽기 연산을 수행한다.)
     * 
     * @param on_message 메세지가 들어왔을 때 실행할 핸들러 (함수 객체)
     * @param on_error 에러가 발생했을 때 실행할 핸들러
     */
    void start(message_handler&& on_message, error_handler&& on_error) {
        this->on_message = std::move(on_message);
        this->on_error   = std::move(on_error);
        async_read();
    }

    /**
     * @brief 인자로 들어온 메세지를 큐에 저장한다. 
     * 현재 메세지를 보내고 있는 중이 아니면 메세지를 비동기 쓰기 연산으로 전송한다.
     * 
     * @param message 
     */
    void post(std::string const& message) {
        bool idle = outgoing.empty();
        outgoing.push(message);

        if (idle) {
            async_write();
        }
    }

 private:
    /**
     * @brief asio 비동기 읽기 연산을 수행한다.'\n'이 들어올 때까지 읽고 스트림버퍼에 저장한다.
     * asio 비동기 읽기 연산이 완료되면 on_read 함수를 호출한다.
     */
    void async_read() {
        io::async_read_until(
            socket, streambuf, "\n",
            std::bind(&session::on_read, shared_from_this(), _1, _2));
        /* [capture0 = shared_from_this()](auto&& PH1, auto&& PH2) {
                capture0->on_read(std::forward<decltype(PH1)>(PH1),
                                  std::forward<decltype(PH2)>(PH2));
            }); */
    }

    /**
     * @brief asio 비동기 읽기 연산 완료 후 호출되는 핸들러
     * 메세지를 만들고 그것을 메세지 핸들러인 on_message에 전달하고 다시 비동기 읽기 연산을 시작하여 다음 메세지를 읽는다.
     * 
     * @param error 
     * @param bytes_transferred 
     */
    void on_read(error_code error, std::size_t bytes_transferred) {
        if (!error) {
            std::stringstream message;
            message << socket.remote_endpoint(error) << ": "
                    << std::istream(&streambuf).rdbuf();
            streambuf.consume(bytes_transferred);
            on_message(message.str());
            async_read();
        } else {
            socket.close(error);
            on_error();
        }
    }

    /**
     * @brief 비동기 쓰기 연산을 수행한다. 큐의 맨 앞에 있는 버퍼에서 데이터를 가져와서 전송한다.
     * 비동기 쓰기 연산이 완료되면 on_write를 호출한다.
     */
    void async_write() {
        io::async_write(
            socket, io::buffer(outgoing.front()),
            std::bind(&session::on_write, shared_from_this(), _1, _2));
    }

    /**
     * @brief 큐에서 메세지를 꺼내고 (pop) 큐에 전송할 메세지가 남아있다면 다시 비동기 쓰기 연산을 시작한다.
     * 
     * @param error 
     * @param bytes_transferred 
     */
    void on_write(error_code error, std::size_t bytes_transferred) {
        if (!error) {
            outgoing.pop();

            if (!outgoing.empty()) {
                async_write();
            }
        } else {
            socket.close(error);
            on_error();
        }
    }

    tcp::socket socket;       /// 클라이언트 소켓
    io::streambuf streambuf;  /// incoming data를 수신하는 스트림 버퍼
    std::queue<std::string> outgoing;  /// outgoing message를 저장하는 큐
    message_handler on_message;        /// 메세지 핸들러
    error_handler on_error;            /// 오류 핸들러
};

class server {
 public:
    server(io::io_context& io_context, std::uint16_t port)
        : io_context(io_context),
          acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {}

    /**
     * @brief 새로 접속한 사람에게 환영 문구를 전달하고 그 사람을 이미 접속한 다른 사람에게 소개한다.
     * 순서는 다음과 같다.
     * 1. 소켓 생성
     * 2. asio 비동기 연결 수립 호출
     * 3. 연결이 수립되면 새 클라이언트 세션을 만들고 새 클라이언트에게 환영 메세지 전송하고 다른 모든 클라이언트에 새 인원이 접속했다고 알림
     * 4. 새 클라이언트 세션에 셋에 추가
     * 5. 새 클라이언트 시작 (클라이언트에서 보내는 메세지를 수신)
     * 6. 다시 비동기 연결 수립 호출하여 새 연결에 대비
     */
    void async_accept() {
        socket.emplace(io_context);

        acceptor.async_accept(*socket, [&](error_code error) {
            auto client = std::make_shared<session>(std::move(*socket));
            client->post("Welcome to chat\n\r");
            post("We have a newcomer\n\r");

            clients.insert(client);

            client->start(std::bind(&server::post, this, _1),
                          [&, weak = std::weak_ptr(client)] {
                              if (auto shared = weak.lock();
                                  shared && clients.erase(shared)) {
                                  post("We are one less\n\r");
                              }
                          });

            async_accept();
        });
    }

    /**
     * @brief 연결된 모든 클라이언트에 세션을 통해서 메세지를 전송한다. 
     * 
     * @param message 
     */
    void post(std::string const& message) {
        for (const auto& client : clients) {
            client->post(message);
        }
    }

 private:
    io::io_context& io_context;
    tcp::acceptor acceptor;
    std::optional<tcp::socket> socket;
    std::unordered_set<std::shared_ptr<session>>
        clients;  /// 다수의 클라이언트와의 연결을 처리하기 위한 셋
};

//* 목표로 하는 형태의 코드
/* using message_type = std::string;
using session_type = chat::session<message_type>;
using server_type = chat::server<session_type>;

class server
{
public:

    server(io::io_context& io_context, std::uint16_t port)
    : srv(io_context, port)
    {
        srv.on_join([&] (session_type& client)
        {
            client.post("Welcome to chat");
            srv.broadcast("We have a newcomer");
        });

        srv.on_leave([&]
        {
            srv.broadcast("We are one less");
        })

        srv.on_message([&] (message_type const& message)
        {
            srv.broadcast(message);
        })
    }

    void start()
    {
        srv.start();
    }

private:

    server_type srv;
}; */


/**
 * @brief 서버 객체를 생성하고 비동기 연결을 수립한다.
 * 
 * @return int 
 */
int main() {
    io::io_context io_context;
    server srv(io_context, 15001);
    srv.async_accept();
    io_context.run();
    return 0;
}