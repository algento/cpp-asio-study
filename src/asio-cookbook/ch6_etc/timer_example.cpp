/**
 * @file timer_example.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>

#include <asio/steady_timer.hpp>

#include "asio/high_resolution_timer.hpp"
#include "asio/system_timer.hpp"

// asio::system_timer: std::system_clock을 이용, 지정된 시각 알림에 사용
// asio::steady_timer: std::steady_clock을 이용, 시간측정 시 사용
// asio::high_resolution_timer: std::high_resolution_clock을 이용, 세밀한 시간 측정시 사용
// 두 개의 타이머를 만들로 하나는 2초 뒤에 다른 하나는 5초 뒤에 종료되게 한다.
// async_wait을 통해 2초 뒤 종료되는 타이머가 5초 뒤에 종료되는 타이머를 취소하도록 하였다.

int main() {
    asio::io_context ioc;

    asio::steady_timer t1(ioc);
    t1.expires_after(std::chrono::seconds(2));

    asio::steady_timer t2(ioc);
    t2.expires_after(std::chrono::seconds(5));

    t1.async_wait([&t2](asio::error_code ec) {
        if (ec.value() == 0) {
            std::cout << "Timer #2 has expired!" << std::endl;
        } else if (ec == asio::error::operation_aborted) {
            std::cout << "Timer #2 has been cancelled!" << std::endl;
        } else {
            std::cout << "Error occured! Error code = " << ec.value()
                      << ". Message: " << ec.message() << std::endl;
        }

        t2.cancel();
    });

    t2.async_wait([](asio::error_code ec) {
        if (ec.value() == 0) {
            std::cout << "Timer #2 has expired!" << std::endl;
        } else if (ec == asio::error::operation_aborted) {
            std::cout << "Timer #2 has been cancelled!" << std::endl;
        } else {
            std::cout << "Error occured! Error code = " << ec.value()
                      << ". Message: " << ec.message() << std::endl;
        }
    });

    ioc.run();

    return 0;
}