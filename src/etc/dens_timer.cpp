/**
 * @file dens_timer.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <chrono>
#include <iostream>
#include <thread>

#include <asio.hpp>

namespace io     = asio;
using error_code = asio::error_code;

io::io_context io_context;

// A timer should be constructed from io_context reference
io::high_resolution_timer timer(io_context);

auto now() { return std::chrono::high_resolution_clock::now(); }

auto begin = now();

void async_wait() {
    // Set the expiration duration
    timer.expires_after(std::chrono::seconds(1));
    // timer.expires_at(std::chrono::high_resolution_clock::now() +
    //                  std::chrono::seconds(1));

    // Wait for the expiration asynchronously
    timer.async_wait([&](error_code error) {
        if (error == asio::error::operation_aborted) {
            std::cout << "The timer is cancelled\n";
            return;
        }
        auto elapsed =
            std::chrono::duration_cast<std::chrono::seconds>(now() - begin)
                .count();
        std::cout << elapsed << "\n";
        async_wait();
    });
}

int main() {
    async_wait();

    //* 새로운 asio 타이머 이용
    // asio::steady_timer timer2(io_context);
    // timer2.expires_after(std::chrono::seconds(5));
    // timer2.async_wait([&](asio::error_code ec) {
    //     if (ec.value() == 0) {
    //         std::cout << "Timer #2 has expired!" << std::endl;
    //     } else if (ec == asio::error::operation_aborted) {
    //         std::cout << "Timer #2 has been cancelled!" << std::endl;
    //     } else {
    //         std::cout << "Error occured! Error code = " << ec.value()
    //                   << ". Message: " << ec.message() << std::endl;
    //     }

    //     timer.cancel();
    // });

    //* 별도의 thread 이용
    std::thread thd([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(4));
        std::cout << "cancel" << std::endl;
        timer.cancel();
    });

    io_context.run();
    thd.join();
    return 0;
}