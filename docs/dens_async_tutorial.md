# Asynchrononous I/O in C++

## Error Handling

## TCP chat server

## Prevent io_context::run from returning(<https://dens.website/tutorials/cpp-asio/work>)

io_context::run()은 이벤트 루프를 통해 등록된 비동기 연산이 모두 완료될 때까지 호출한 스레드를 블럭하고 대기한다. 따라서 등록된 비동기 연산이 없다면 해당 함수는 이벤트 루프를 종료하고 반환한다.

```c++
boost::asio::io_context io_context;
// Schedule some tasks
io_context.run();
std::cout << "Job's done! Continue the execution\n";
```

하지만 일부 어플리케이션에서는 등록된 비동기 연산이 없어도 이벤트 루프를 유지할 필요가 있다. 예를 들어 서버의 경우 async_accept을 수행하면 클라이언트가 접속할 때까지 이벤트 루프가 유지되기 때문에 io_context::run()이 반화되지 않고 유지된다. 하지만 client의 경우는 서버에 요청을 보내고 응답을 받게 되면 모든 이벤트 처리가 완료되어 io_context::run()이 반환된다. 이것을 막기 위해 asio::executator_work_guard (이전 io_service::work) 을 사용한다.

```c++
using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

boost::asio::io_context io_context;
work_guard_type work_guard(io_context.get_executor());
// Schedule some tasks or not
io_context.run();
std::cout << "Sorry, we'll never reach this!\n";
```

만약 어플리케이션의 동작을 중지하고 싶다면 io_context::stop()을 사용하면 된다. 아래의 코드는 io_context::stop()을 이용하여 중지시키면 io_context가 남은 task들을 삭제한 후 io_context를 중지한다.

```c++
boost::asio::io_context io_context;
work_guard_type work_guard(io_context.get_executor());
// Schedule some tasks or not
std::thread watchdog([&]
{
    std::this_thread::sleep_for(10s);
    io_context.stop(); // That's OK, io_context::stop is thread-safe
});
io_context.run();
std::cout << "We stopped after 10 seconds of running\n";
```

io_context::reset()을 사용한 경우, 더 이상의 이벤트 등록을 하지않고 등록된 비동기 연산을 모두 처리한 후 중지한다. 또한 io_context::run()이 반환된 뒤, 다시 io_context::run()을 사용하려면 호출 전에 반드시 io_context::reset()을 호출해야 한다.

```c++
boost::asio::io_context io_context;
auto work_guard = std::make_unique<work_guard_type>(io_context.get_executor());
// Schedule some tasks or not
std::thread watchdog([&]
{
    std::this_thread::sleep_for(10s);
    work_guard.reset(); // Work guard is destroyed, io_context::run is free to return
});
io_context.run();
std::cout << "We stopped after 10+ seconds of running\n";
```

## Post your own functors into io_context

io_context::run()을 실행하면 비동기 작업(task)을 수행하는 이벤트 루프가 실행되고 작업 완료 이벤트가 반환되면 task completion handler가 호출(invoke)된다.
만약 asio에서 지원하지 않는 비동기 연산을 io_context::run()의 event pooling loop에 넣고 실행하려면 asio::post()를 사용하면 된다. asio::post()는 io_context가 관리하는 executation queue에 함수 객체를 넣고 해당 함수 객체는 asio에 의해서 thread-safe하게 호출된다. 따라서 서로 다른 스레드에서 asio::post()를 호출해도 문제가 없다.

```c++
namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

io::io_context io_context;
auto work_guard = io::make_work_guard(io_context); // A facility that makes life easier
// Schedule some tasks or not
io_context.run();

// ...
// Somewhere else, another thread may be:
io::post(io_context, [&]
{
    std::cout << "Hey, I'm inside io_context::run polling loop!\n";
});
```

## Multithreaded Executation

단일 스레드가 아니라 다중 스레드로 비동기 연산을 수행하려면 두가지 방법이 있다 가장 간단한 방법은 단일 io_context를 이용해서 io_context::run() 함수를 스레드 풀에서 실행하는 것이다. 이렇게 할 경우, 비동기 연산 자체는 OS 영역에서 알아서 수행되고 completion handler는 io_context::run()이 호출된 스레드 풀에서 free thread가 생길 때 마다 실행된다. 즉, 비동기 연산 자체는 OS side에서 알아서 실행하고 completion handler가 다중 스레드로 실행된다는 의미이다. 하지만 completion handler 사이에 동기화가 필요하다면 이것을 잘 처리해야 한다.

```c++
io::io_context io_context;
// Prepare things
std::vector<std::thread> threads;
auto count = std::thread::hardware_concurrency() * 2;

for(int n = 0; n < count; ++n)
{
    threads.emplace_back([&]
    {
        io_context.run();
    });
}

for(auto& thread : threads)
{
    if(thread.joinable())
    {
        thread.join();
    }
}
```

io_context를 사용하는 경우, 이런 동기화는 io_context::strand 클래스를 사용하여 처리할 수 있다.
동일 io_context::strand에 연관된 completion handler는 서로 다른 스레드에서 실행되어도 순차적으로 호출된다. 따라서 동일한 공유 자원을 사용하는 completion handler들을 동기화하려면 해당 핸들러들을 동일한 strand와 연관(attach)하면 된다. asio::bind_executator()를 이용하여 completion handler를 wrapping하면 strand에 attach할 수 있다.

아래의 예제에서는 읽기/쓰기를 별도의 strand로 설정하였다 이것은 읽기/쓰기와 관련된 completion handler는 각각 동기화되어 실행된다는 의미이다. 즉, 스레드 풀을 사용해도 읽기/쓰기와 연관된 각각의 completion handler는 순차적으로 실행된다는 의미이다. io_context::post() 함수를 이용해서 해당 strand (여기서는 read)에 별도의 함수객체를 추가할 수 있다.

```c++
class session
{
    session(io::io_context& io_context)
    : socket(io_context)
    , read  (io_context)
    , write (io_context)
    {
    }

    void async_read()
    {
        io::async_read(socket, read_buffer, io::bind_executor(read, [&] (error_code error, std::size_t bytes_transferred)
        {
            if(!error)
            {
                // ...
                async_read();
            }
        }));
    }

    io::post(read, []
    {
        std::cout << "We're inside a read sequence, it's safe to access a read-related data here!\n";
    });

    void async_write()
    {
        io::async_read(socket, write_buffer, io::bind_executor(write, [&] (error_code error, std::size_t bytes_transferred)
        {
            if(!error)
            {
                // ...
                async_write();
            }
        }));
    }

private:

    tcp::socket socket;
    io::io_context::strand read;
    io::io_context::strand write;
}
```

위와 같이 N 개의 스레드에서 하나의 io_context를 공유하여 사용하는 것이 아니라 io_context하나와 스레드 하나를 짝으로 사용할 수도 있다.
이 경우 각 스레드는 자신만의 io_context를 가진다. 이 경우는 동일한 io_context에서 동일한 공유 자원에 접근한다면 별도의 strand를 사용한 동기화가 필요하지 않다.
동일 io_context에 묶인 completion handler는 단일 스레드로 동작하기 때문에 연산의 균형 (balancing)은 개발자가 직접 고려해야 한다. 따라서 기본적으로 전자의 방법을 고려하고 랜덤하고 가벼운 작업이 많지 않고 특수한 방식으로 사용하는 경우 후자의 방법을 고려해라.

```C++
namespace io = asio;
using tcp = io::ip::tcp;
using work_guard_type = io::executor_work_guard<io::io_context::executor_type>;
using error_code = asio::error_code;

class io_context_group
{
public:

    io_context_group(std::size_t size)
    {
        // Create io_context and work guard pairs
        for(std::size_t n = 0; n < size; ++n)
        {
            contexts.emplace_back(std::make_shared<io::io_context>());
            guards.emplace_back(std::make_shared<work_guard_type>(contexts.back()->get_executor()));
        }
    }

    void run()
    {
        // Create threads
        for(auto& io_context : contexts)
        {
            threads.emplace_back([&]
            {
                io_context->run();
            });
        }

        // Join threads
        for(auto& thread : threads)
        {
            thread.join();
        }
    }

    // Round-robin io_context& query
    // context 컨터이너를 반복하면서 io_context를 가져온다.
    // 소켓 객체를 생성할 때, query() 함수를 호출해서 io_context를 돌아가면 사용한다.
    io::io_context& query()
    {
        return *contexts[index++ % contexts.size()];
    }

private:

    template <typename T>
    using vector_ptr = std::vector<std::shared_ptr<T>>;

    /** io_context, work_guard, thread를 하나의 그룹으로 생각한다.
    */
    vector_ptr<io::io_context> contexts;
    vector_ptr<work_guard_type> guards;
    std::vector<std::thread> threads;

    std::atomic<std::size_t> index = 0;
};

int main()
{
    io_context_group group(std::thread::hardware_concurrency() * 2);
    tcp::socket socket(group.query());
    // Schedule some tasks
    group.run();
    return 0;
}
```

## Timer

asio가 제공하는 타이머에는 다음의 4가지 종류가 있지만 대부분 유사한 기능을 제공한다. 차이라면 deadline_timer는 posix_time을 이용한 것이고, 나머지는 std::chrono에 기반한 것이라는 점이다.

```C++
asio::deadline_timer
asio::high_resolution_timer
asio::steady_timer
asio::system_timer
```

아래의 예제에서 볼 수 있듯이 asio::timer는 io_context 객체에 연관하여 사용하고 타이머와 관련된 handler는 io_context::run()의 이벤트 루프에 등록된다.

```c++
namespace io = boost::asio;
using error_code = boost::system::error_code;

io::io_context io_context;

// A timer should be constructed from io_context reference
io::high_resolution_timer timer(io_context);

auto now()
{
    return std::chrono::high_resolution_clock::now();
}

auto begin = now();

void async_wait()
{
    // Set the expiration duration
    timer.expires_after(std::chrono::seconds(1));

    // Wait for the expiration asynchronously
    timer.async_wait([&] (error_code error)
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now() - begin).count();
        std::cout << elapsed << "\n";
        async_wait();
    });
}

int main()
{
    async_wait();
    io_context.run();
    return 0;
}
```
