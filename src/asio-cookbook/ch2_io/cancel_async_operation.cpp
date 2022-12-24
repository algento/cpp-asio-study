/**
 * @file cancel_async_operation.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <memory>

#include "asio/error_code.hpp"
#include "asio/system_error.hpp"

#ifdef _WIN32
    #define _WIN32_WINNT 0x0501

    #if _WIN32_WINNT <= 0x0502  // Windows Server 2003 or earlier.
        #define BOOST_ASIO_DISABLE_IOCP
        #define BOOST_ASIO_ENABLE_CANCELIO
    #endif
#endif

#include <iostream>
#include <thread>

#include <asio.hpp>

int main() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num    = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address),
                                   port_num);
        asio::io_context ioc;

        std::shared_ptr<asio::ip::tcp::socket> sock =
            std::make_shared<asio::ip::tcp::socket>(ioc, ep.protocol());

        //* async_connect는 main thread에서 호출하고 io_context::run()은 worker thread에서 호출된다.
        //* async_connect는 즉시 반환되고 연결이 완료되거나 에러가 발생하면 등록한 handler가 호출된다.
        //* https://devdockr.tistory.com/227
        sock->async_connect(ep, [sock](const asio::error_code& ec) {
            // If asynchronous operation has been cancelled or an error occured during
            // executon, ec contains corresponding error code.
            if (ec.value() != 0) {
                if (ec == asio::error::operation_aborted) {
                    std::cout << "Operation cancelled!";
                } else {
                    std::cout << "Error occured in async_connect!"
                              << " Error code = " << ec.value()
                              << ". Message: " << ec.message();
                }

                return;
            }
            // At this point the socket is connected and can be used
            // for communication with remote application.
            std::cout << "Connected" << std::endl;
        });

        // Starting a thread, which will be used to call the callback
        // when asynchronous operation completes.
        //* worker thread는 ioc.run()을 수행하여 event loop를 통해 모든 비동기 연산이 종료되었다는 이벤트를 기다린다.
        std::thread worker_thread([&ioc]() {
            try {
                std::cout << "Run()" << std::endl;
                ioc.run();
            } catch (asio::system_error& e) {
                std::cout << "Error occured in worker thread!"
                          << " Error code = " << e.code()
                          << ". Message: " << e.what();
            }
        });
        // Emulating delay.
        //* 이 지연을 넣으면 main thread가 2초동안 블럭되므로 아래의 비동기 연산 취소가 2초 동안 지연된다.
        //* 이 코드가 있으면 async_connect에서 연결 시도 오류가 2초 이전에 발생하기 때문에 async_connect의
        //* handler가 호출되고, 없으면 cancle이 먼저 호출되어 operation aborted가 출력된다.
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Cancelling the initiated operation.
        sock->cancel();

        // Waiting for the worker thread to complete.
        worker_thread.join();

    } catch (asio::system_error& e) {
        std::cout << "Error occured in main thread! Error code = " << e.code()
                  << ". Message: " << e.what();

        return e.code().value();
    }
}
