# I/O OPeration

I/O 연산은 데이터 교환과 관련되어 원격 앱에서 데이터를 읽을 때 입력 연산 (read)을 수행하고 데이터를 보낼 때 출력 연산 (write)을 수행한다.
통신을 하는 두 프로그램 입장에서 보면, I/O 연산은 결국 메모리의 읽기/쓰기이다. 네트워크 통신도 운영체제에서 제공하는 네트워크 I/O가 사용하는 메모리에 접근하여 데이터를 읽고 쓰는 과정으로 치환된다.

asio에서는 동기/비동기 I/O 연산을 지원한다. 여기서 동기 I/O 연산이란 I/O 연산을 호출한 스레드가 해당 연산이 종료될 때까지 블록되는 연산을 말한다. 비동기 연산 I/O 연산은 호출하자 마자 반환하여 해당 연산을 호출한 스레드가 블록되지 않는다. 실제 I/O 연산은 콜백 함수 혹은 함수 객체 (functor)를 통해서 별도의 스레드에서 실행된다. 콜백 함수가 완료되면 이벤트가 발생하여 호출한 스레드로 결과가 반환된다. 비동기 연산은 실제 I/O 연산이 백그라운드로 실행되는 동안 다른 작업을 할 수 있다는 장점이 있다. 또한 비동기 연산은 취소가 가능해야 한다. 그래야만 이미 실행된 비동기 연산의 자원을 실시간으로 회수할 수 있다. 만약 취소가 되지 않는다면 필요없는 연산이 계속 실행되고 자원을 소모하게 된다.

asio는 소켓을 중단하거나 (shutdown) 닫는 (close) 작업의 수행을 지원한다. 소켓의 중단은 소켓의 동작을 중지시키는 것이지 소켓이 가지고 있는 자원을 반환하지는 않으며, 한 앱에서 통신하는 다른 앱에 보낼 데이터를 모두 보냈다는 것을 알리는데 쓰일 수 있다.소켓의 닫음은 소켓이 가지고 있던 모든 자원을 반환하는 것을 말한다.

## Using Fixed length I/O buffers

크기가 고정된 버퍼는 보내거나 받는 메세지의 크기를 알고 있을 때 유용하다. asio에서는 고정 크기 버퍼로 다음의 두 클래스를 제공한다.

1. asio::mutable_buffer
1. asio::const_buffer

mutable_buffer는 읽기/쓰기가 가능한 메모리 버퍼이며, const_buffer는 읽기만 가능한 메모리 버퍼이다. 각각 MutableBufferSequence, ConstBufferSequence 개념(concept)을 만족해야 하며, stl에서 제공하는 컨테이너는 기본적으로 두 개념를 모두 만족한다. 여기서 concept는 템플릿 타입이 만족해야하는 명세로 간단히 이해하면 된다. 컨테이너는 연속된 메모리 공간이어도 되고 여러 개의 메모리가 묶인 결합 버퍼(composite buffer)여도 된다. 단순 사용자 정의 타입은 해당 명세를 만족하지 못하니 주의해라.

asio::buffer()함수는 MutableBufferSequence/ConstBufferSequence 명세를 만족하는 클래스 객체를 전달받아 asio::mutable_buffer/asio::const_buffer를 반환한다. 해당 버퍼들은 asio I/O에서 제공하는 함수의 입력 인자로 편하게 사용할 수 있다.
예를 들어 asio::send 함수는 다음의 명세를 가진다.

```c++
template<typename ConstBufferSequence>
std::size_t send(const ConstBufferSequence &buffer);
```

여기에 사용자 정의 타입 T의 객체를 정식으로 넣으려면, 아래와 같이 하면 된다.

```c++
T buf;
std::vector<T> buffers_sequence;
buffers_sequnece.push_back(buf);
```

하지만 이렇게 하면 간단하다.

```c++
T buf;
asio::const_buffer asio_buf = asio::buffer(&T, sizeof(T));
```

## Using extensible stream-oriented I/O buffers

데이터가 추가될 때마다 동적으로 크기가 커지는 버퍼를 확장가능한 버퍼라고 하며, 들어오는 데이터의 정확한 크기를 알 수 없는 경우에 사용한다.
예를 들어, HTTP 프로토콜의 경우, 응답/요청 메세지의 헤더 크기가 가변이기 때문에 메세지가 끝날 때마다 `<CR>, <LF>, <CR>, <LF>`의 4개의 문자를 전송한다. 이런 경우에 확장가능 버퍼를 사용하는 것이 좋으며 asio에서 해당 기능을 제공한다.
asio::streambuf는 std::streambuf를 상속하여 std의 스트림 클래스가 필요한 상황에 사용가능하다. 예를 들어 std::istream, std::ostream, std::iostream에 할당하여 사용할 수 있다.

## Writing to a TCP socket synchronously

## Reading from a TCP socket synchronously

## Writing to a TCP socket asynchronously

## Reading from a TCP socket asynchronously

## Canceling asynchronous operations

## Shutting down and closing a socket
