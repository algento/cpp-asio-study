# Server Application

서버는 일반적으로 클라이언트와의 통신 과정에서 클라이언트의 요청을 대기하다가 요청이 들어오면 응답을 전송하는 역할을 한다.
서버는 제공하는 서비스에 따라 요청처리 과정이 다르다. HTTP 서버는 요청 메세지에 따라 원하는 파일을 읽어서 응답으로 보낸다.
프록시 서버는 클라이언트의 요청을 실제로 연산할 서버에 전달하는 역할을 한다.
모든 서버가 수동적인 역할을 하는 것은 아니다. 일부 서버는 클라이언트의 요청을 기다리지 않고 notifier처럼 유용한 이벤트가 발생했을 때, 클라이언트에게 알려준다. 이 경우, 클라이언트는 수동적으로 서버가 알림을 보내기를 기다리며 데이터가 오면 그에 맞춰 동작한다. 이런 방식을 push-style이라고 하며 좀 더 유연하게 동작할 수있기 때문에 최근 인기를 얻고 있다.

또한 서버는 사용하는 프로토콜에 따라 TCP, UDP 그리고 둘 다 혼합해서 사용하는 다중 프로토콜 서버로 구분할 수 있다.
또다르게 서버를 구분하는 방법은 반복(iterative) 서버 과 병렬(parallel) 서버로 구분하는 것이다. 반복 서버는 클라이언트를 한번에 하나씩 처리하고 병렬 서버는 다수의 클라이언트를 동시에 처리한다.

동기 방식과 비동기 방식으로도 구분할 수 있다. 장/단점은 앞에서 설명한 것과 같다. 가지고 있는 특성 때문에 동기 서버는 안전하게 보호받을 수 있는 네부 네트워크나 프로세스 간의 통신사이에 주로 사용된다.

## Iterative Synchronous TCP Server

## Parallel Synchronous TCP Server

## Asynchronous TCP Server
