/**
 * @file dens_asyn_server.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-23
 * https://dens.website/tutorials/cpp-asio/async-tcp-server
 * @copyright Copyright (c) 2022
 * 
 */

#include <iostream>
#include <optional>

#include <asio.hpp>

class session : public std::enable_shared_from_this<session> {
 public:
    session(asio::ip::tcp::socket&& socket) : socket(std::move(socket)) {}

    void start() {
        //* reads the data from the socket until it gets "\n" character,
        //* and after that it writes the data received into stdout.
        //* the lambda function captures the self shared point, so wthe session object keeps alive until the completion handler is invoked.
        asio::async_read_until(
            socket, streambuf, '\n',
            [self = shared_from_this()](asio::error_code error,
                                        std::size_t bytes_transferred) {
                std::cout << std::istream(&self->streambuf).rdbuf();
            });
    }

 private:
    asio::ip::tcp::socket socket;
    asio::streambuf streambuf;
};

class server {
 public:
    server(asio::io_context& io_context, std::uint16_t port)
        : io_context(io_context),
          acceptor(io_context,
                   asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

    void async_accept() {
        socket.emplace(io_context);

        acceptor.async_accept(*socket, [&](asio::error_code error) {
            std::make_shared<session>(std::move(*socket))->start();
            async_accept();
        });
    }

 private:
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;
    //* socket은 나중에 생성될 수 있으므로 std::optional을 사용한다.
    std::optional<asio::ip::tcp::socket> socket;
};

int main() {
    asio::io_context io_context;
    server srv(io_context, 15001);

    //* The server starts to wait for the next connection right away,
    //* without any concerns of what is going on with the previous connection.
    //* Accepted connections perform in background.
    //* There can be any (well, almost any, limited by the OS open file descriptors limit)
    //*  amount of simultaneous connections performing in background — that's how asynchronous I/O does work.
    srv.async_accept();

    //*  sort of event loop function that manages all I/O operations.
    //* Call of run blocks the caller thread until all asynchronous operations
    //* associated with its io_context are completed.
    io_context.run();
    return 0;
}