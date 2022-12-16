/**
 * @file fixed_length_buffer.cpp
 * @author Sejong Heo (tromberx@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>

#include <asio.hpp>

#include "asio/read_at.hpp"

int main() {
    //* ---------------------------------------------------------------------*//
    //* output buffer
    //* ---------------------------------------------------------------------*//
    std::string buf1;  // 'buf' is the raw buffer.
    buf1 = "Hello";    // Step 1 and 2 in single line.

    // Step 3. Creating buffer representation that satisfies
    // ConstBufferSequence concept requirements.
    //* 책에서는 mutable_buffers_1/const_buffers_1이란 adapter가 등장하지만
    //* 현재 asio 버전에서는 deprecated되었고 그냥 mutable_buffer/const_buffer를 쓰면 된다.
    asio::const_buffer output_buf = asio::buffer(buf1);

    // Step 4. 'output_buf' is the representation of the
    // buffer 'buf' that can be used in Boost.Asio output
    // operations.

    std::cout << "size: " << output_buf.size() << std::endl;
    for (size_t i = 0; i < output_buf.size(); ++i) {
        std::cout << *(static_cast<const char *>(output_buf.data()) + i);
    }
    std::cout << std::endl;

    //! 아래 코드는 출력이 이상하다. 그 이유는 string의 마지막 '\0'은 복사하지 않기 때문이다.
    // std::cout << *(static_cast<const std::string*>(output_buf.data()))
    //           << ", size: " << output_buf.size() << std::endl;

    //* ---------------------------------------------------------------------*//
    //* input buffer
    //* ---------------------------------------------------------------------*//
    // We expect to receive a block of data no more than 20 bytes
    // long.
    const size_t BUF_SIZE_BYTES = 20;

    // Step 1. Allocating the buffer.
    std::unique_ptr<char[]> buf2(new char[BUF_SIZE_BYTES]);

    // Step 2. Creating buffer representation that satisfies
    // MutableBufferSequence concept requirements.
    //* asio::buffer(void*, size_t) overloading
    asio::mutable_buffer input_buf =
        asio::buffer(static_cast<void *>(buf2.get()), BUF_SIZE_BYTES);

    // Step 3. 'input_buf' is the representation of the buffer
    // 'buf' that can be used in Boost.Asio input operations.
    return 0;
}