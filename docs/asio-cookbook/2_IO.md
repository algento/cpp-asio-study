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

asio는 TCP 소켓에 동기적으로 데이터를 쓸 수 있다. asio에서 제공하는 쓰기 함수가 호출되면 호출한 스레드는 블럭된다. 쓰기함수는 데이터가 일부라도 소켓에 쓰이거나 오류가 발생할 경우 호출한 스레드의 블럭을 푼다.
asio::ip::tcp::socket에서 제공하는 기본 쓰기 연산 방법은 write_some() 메서드이다. 이 메서드는 이름에서 알 수 있듯이 데이터의 일부를 소켓에 쓰고 성공할 경우 쓰는 데 성공한 바이트의 크기를 반환한다. 일반적으로 써야할 데이터가 큰 경우, 이 메서드를 여러 번 호출해야한다.
asio::ip::tcp::socket에서 제공하는 send()라는 메서드는 소켓 연산을 제어하는 bit maskd인 flag를 반환해준다는 장점이 있지만 기본적으로는 write_some 메서드와 동작은 동일하다. 기본적으로 아래의 함수 정의 이외에도 다양한 오버로딩을 제공하니 확인해보자.

```c++
template<typename ConstBufferSequence>
std::size_t write_some(const ConstBufferSequence &buffer);

template<typename ConstBufferSequence>
std::size_t send(const ConstBufferSequence &buffer, socket_base::message_flags flags);
```

하지만 위의 write_some 메서드는 얼마나 썼는지 확인하면서 여러 번 호출해야하기 때문에 사용자가 직접 코드를 작성할 경우 코드가 복잡하며, 오류가 발생하기 쉽다. 이를 보완하기 위해 asio::write() 메서드가 제공된다.

```c++
template<typename SyncWriteStream, typename ConstBufferSequence>
std::size_t write(SyncWriteStream &s, const ConstBufferSequence &buffer);
```

SyncWriteStream이라는 명세를 만족하는 인자와 버퍼를 받아서 해당 스트림에 입력받은 버퍼를 쓰는 메서드이다.
asio::ip::tcp::socket이 해당 명세를 만족하니 우리는 소켓을 첫번째 인자로 넣으면 된다. 이 메서드는 버퍼에 있느 모든 데이터를 소켓에 쓰는 작업을 해주기 때문에 코드가 간단명료하다.

## Reading from a TCP socket synchronously

asio는 TCP 소켓에서 동기적으로 데이터를 읽는 기능을 지원한다. 동기적인 소켓 읽기 연산은 호출되면 호출한 스레드를 블럭하고 소켓에서 일부라도 데이터를 읽거나 오류가 발생하면 블럭을 해제하고 반환한다.

asio::ip::tcp::socket에서 제공하는 기본 일기 연산 방법은 read_some() 메서드이다. 이 메서드는 이름에서 알 수 있듯이 데이터의 일부를 소켓에서 읽고 성공할 경우 읽는 데 성공한 바이트의 크기를 반환한다. 일반적으로 읽어야 할 데이터가 큰 경우, 이 메서드를 여러 번 호출해야한다.
asio::ip::tcp::socket에서 제공하는 receive() 메서드는 소켓 연산을 제어하는 bit maskd인 flag를 반환해준다는 장점이 있지만 기본적으로는 read_some 메서드와 동작은 동일하다. 기본적으로 아래의 함수 정의 이외에도 다양한 오버로딩을 제공하니 확인해보자.

```c++
template<typename MutableBufferSequence>
std::size_t read_some(const MutableBufferSequence &buffer);

template<typename ConstBufferSequence>
std::size_t receive(const MutalbeBufferSequence &buffer, socket_base::message_flags flags);
```

하지만 위의 read_some 메서드는 소켓 버퍼에서 얼마나 읽는지 확인하면서 여러 번 호출해야 전체 데이터를 읽을 수 있기 때문에 사용자가 직접 코드를 작성할 경우 코드가 복잡하며, 오류가 발생하기 쉽다. 이를 보완하기 위해 asio::read() 메서드가 제공된다.

```c++
template<typename SyncReadStream, typename MutalbeBufferSequence>
std::size_t read(SyncReadStream &s, const MutableBufferSequence &buffer);
```

SyncReadStream이라는 명세를 만족하는 인자와 버퍼를 받아서 해당 스트림에 입력받은 버퍼를 쓰는 메서드이다.
asio::ip::tcp::socket이 해당 명세를 만족하니 우리는 소켓을 첫번째 인자로 넣으면 된다. 이 메서드는 소켓 버퍼에 있는 모든 데이터를 읽는 작업을 해주기 때문에 코드가 간단명료하다.

소켓에 쓰여진 데이터를 스트림 기반의 확장 가능한 버퍼인 streambuf로 받아서 특정 구분자 (delimeter)를 만날 때까지 읽는 함수도 제공된다. 소켓 버퍼에서 데이터를 읽어서 해당 데이터를 버퍼인 b에 쓴다. 이 때, (일부) 읽은 데이터에 구분자가 포함되면 동작을 중지한다. 당연히 구분자 다음에도 데이터가 일부 들어가 있기 때문에 읽어들인 데이터 전체가 아니라 적절히 해석 (parsing)하여 데이터를 추출해야 한다.

```c++
template<typename SyncReadStream, typename Allocator>
std::size_t read_until(SyncReadStream &s, asio::basic_streambuf<Allocator> &b, char delim);
```

SyncReadStream이라는 명세를 만족하는 인자와 버퍼를 받아서 해당 스트림에 입력받은 버퍼를 쓰는 메서드이다.
asio::ip::tcp::socket이 해당 명세를 만족하니 우리는 소켓을 첫번째 인자로 넣으면 된다. 이 메서드는 소켓 버퍼에 있는 모든 데이터를 읽는 작업을 해주기 때문에 코드가 간단명료하다.

소켓에 쓰여진 데이터를 스트림 기반의 확장 가능한 버퍼인 streambuf로 받아서 특정 구분자 (delimeter)를 만날 때까지 읽는 함수도 제공된다. 소켓 버퍼에서 데이터를 읽어서 해당 데이터를 버퍼인 b에 쓴다. 이 때, (일부) 읽은 데이터에 구분자가 포함되면 동작을 중지한다. 당연히 구분자 다음에도 데이터가 일부 들어가 있기 때문에 읽어들인 데이터 전체가 아니라 적절히 해석 (parsing)하여 데이터를 추출해야 한다.

소켓에서 데이터를 읽을 때 지정한 오프셋부터 읽을 수 있는 함수인 read_at 메서드도 제공된다. 하지만 잘 사용하지 않으므로 간단히 정리한다.

```c++
template <typename SyncRandomAccessReadDevice, typename MutableBufferSequence>
inline std::size_t read_at(SyncRandomAccessReadDevice& d,
    uint64_t offset, const MutableBufferSequence& buffers)
```

## Writing to a TCP socket asynchronously

비동기 쓰기 연산은 원격 앱에 데이터를 보내는 유연하고 효율적인 방법이다. 비동기 쓰기 연산은 호출한 스레드를 블럭하지 않고 바로 반환되기 때문에 호출 스레드에서 다른 연산을 수행할 수 있다. 비동기 쓰기 연산은 호출과 동시에 호출한 스레드 정보를 라이브러리로 전달하고 비동기 쓰기 연산의 인자로 전달된 콜백함수를 실행한다. 콜백함수가 종료되면 이벤트에 의하여 반환값이 라이브러리에서 저장하고 있던 스레드로 전달된다.

asio에서 제공하는 기본 비동기 쓰기 연산은 async_write_some 메서드이다. 이 메서드는 호출과 동시에 곧바로 반환된다. 입력인자는 비동기르 쓸 데이터가 저장된 버퍼와 비동기 쓰기 연산에 사용할 콜백 함수 2가지이다. 동기 연산과 마찬가지로 오류없이 끝난다면 적어도 한 바이트는 쓰였다는 것만 보장하므로 쓰려는 데이터가 클 경우 동일하게 여러번 호출해야 한다.

```c++
template<typenamce ConstBufferSequence, typename WriteHandler>
void async_write_some(const ConstBufferSequence &buffer, WriteHandler handler);
```

콜백함수인 WriteHandler는 다음과 같은 서명을 가져야 한다.

```c++
void wirte_handler(const asio::error_code &ec, std::size_t bytes_transffered);
```

WriteHandler가 동작을 할 때 필요한 정보들이 있는 경우가 많다. 이 경우는 콜백 함수에 해당 정보를 인자로 추가한 함수를 만들고 WriteHandler에는 해당 인자를 비동기 연산 호출 시 std::bind를 이용하여 전달하는 방법을 사용하면 매우 편리하다.

asio의 비동기 연산은 asio::io_context::run() 메서드와 함께 사용해야 한다. 해당 메서드는 비동기 연산을 호출한 스레드가 비동기 연산을 실제 수행하는 스레드보다 먼저 종료되지 않도록 호출 스레드를 블럭하고 비동기 연산이 모두 종료되면 블럭을 해제한다.

## Reading from a TCP socket asynchronously

비동기 읽기 연산은 원격 앱에 보내어 서버의 소켓 버퍼에 저장되어 있는 데이터를 읽는 유연하고 효율적인 방법이다. 비동기 읽기 연산은 호출한 스레드를 블럭하지 않고 바로 반환되기 때문에 호출 스레드에서 다른 연산을 수행할 수 있다. 비동기 읽기 연산은 호출과 동시에 호출한 스레드 정보를 라이브러리로 전달하고 비동기 읽기 연산의 인자로 전달된 콜백함수를 실행한다. 콜백함수가 종료되면 이벤트에 의하여 반환값이 라이브러리에서 저장하고 있던 스레드로 전달된다.

asio에서 제공하는 기본 비동기 읽기 연산은 async_read_some 메서드이다. 이 메서드는 호출과 동시에 곧바로 반환된다. 입력인자는 비동기르 읽을 데이터를 저장할 버퍼와 비동기 읽기 연산에 사용할 콜백 함수 2가지이다. 동기 연산과 마찬가지로 오류없이 끝난다면 적어도 한 바이트는 읽었다는 것만 보장하므로 쓰려는 데이터가 클 경우 동일하게 여러번 호출해야 한다.

```c++
template<typenamce MutableBufferSequence, typename ReadHandler>
void async_write_some(const MutableBufferSequence &buffer, ReadHandler handler);
```

콜백함수인 ReadHandler는 다음과 같은 서명을 가져야 한다.

```c++
void read_handler(const asio::error_code &ec, std::size_t bytes_transffered);
```

ReadHandler가 동작을 할 때 필요한 정보들이 있는 경우가 많다. 이 경우는 콜백 함수에 해당 정보를 인자로 추가한 함수를 만들고 ReadHandler에는 해당 인자를 비동기 연산 호출 시 std::bind를 이용하여 전달하는 방법을 사용하면 매우 편리하다.

asio의 비동기 연산은 asio::io_context::run() 메서드와 함께 사용해야 한다. 해당 메서드는 비동기 연산을 호출한 스레드가 비동기 연산을 실제 수행하는 스레드보다 먼저 종료되지 않도록 호출 스레드를 블럭하고 비동기 연산이 모두 종료되면 블럭을 해제한다.

## Canceling asynchronous operations

동기 연산과 달리 비동기 연산은 연산이 필요없거나 시간이 너무 오래 걸리는 경우 안전하게 종료할 수 있는 기능을 지원해야한다. asio에서는 비동기 연산을 언제든지 취소할 수 있는 기능을 지원한다.
비동기 연산을 시작하고 취소하는 과정은 다음과 같다.

1. 소켓을 할당하고 연다.
2. 비동기 연산에서 사용할 callback function이나 functor를 정의한다. 필요하다면 콜백함수 내에서 연산이 취소된 상황을 처리하는 코드를 추가한다.
3. 하나 또는 그 이상의 비동기 연산을 시작하고 앞에서 정의한 callback 함수나 functor를 등록한다.
4. 새로운 스레드를 만들어 asio 이벤트 루프를 활성화시킨다.
5. 비동기 연산을 취소하기 위해 cancel() 메서드를 호출한다.

### Note

윈도우 XP 혹은 서버 2003에서 실행하려면 윈도우가 지원할 수 있도록 별도의 플래그를 정의해야 한다.

- BOOST_ASIO_ENABLE_CANCELIO
- BOOST_ASIO_DISABLE_IOCP

## Shutting down and closing a socket

TCP로 통신하는 분산 앱은 메세지의 크기가 고정되어 있지 않고, 특정 바이트 순영로 메세지가 끝나지도 않는다.
이를 극복하기 위해 논리적은 헤더 영역과 본문 영역으로 메세지를 구성해서 끝을 알릴 수 있다. 헤더는 고정된 크기로 본문 영역에 대한 정보 (크기 등)를 알려준다. 또다른 방법은 앱이 각각의 상대와 통신할 때 매번 새로운 소켓을 사용하도록 하여 메세지를 전송한 측에서 소켓의 전송 부분을 shutdown하는 것이다. 이럴 경우, 소켓을 shutdown했다는 특수 신호가 수신 측에 전송되므로 이를 이용해서 송신 측에서 모든 메세지를 보냈다는 것을 알 수 있다.
소켓 close는 소켓 shutdown과는 달리 소켓 및 소켓과 관련된 다른 자원들을 운영체제로 반환한다.