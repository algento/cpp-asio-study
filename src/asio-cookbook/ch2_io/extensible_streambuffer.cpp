/**
 * @file extensible_streambuffer.cpp
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

int main() {
    //* asio::streambuf로 streambuf를 만들고 이를 ostream에 전달하여 데이터를 쓴 후,
    //* istream에 전달하여 데이터를 읽을 수 있다.
    asio::streambuf buf;

    std::ostream output(&buf);

    // Writing the message to the stream-based buffer.
    output << "Message1\nMessage2";

    // Now we want to read all data from a streambuf until '\n' delimiter.
    // Instantiate an intput stream which uses our stream buffer.
    std::istream input(&buf);

    // We'll read data into this string.
    std::string message1;

    std::getline(input, message1);

    // Now message1 string contains 'Message1'.
    std::cout << message1 << std::endl;

    return 0;
}