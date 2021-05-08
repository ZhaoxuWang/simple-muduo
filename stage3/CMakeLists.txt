cmake_minimum_required(VERSION 3.5)

set(SRCS
    utils.cc
    Buffer.cc
    InetAddress.cc
    Socket.cc
    Acceptor.cc
    Connector.cc
    TcpConnection.cc
    TcpClient.cc
    TcpServer.cc
    Channel.cc
    Epoll.cc
    EventLoop.cc
    EventLoopThread.cc
    EventLoopThreadPool.cc
    test2.cc
    timer.cc
    base/Thread.cc
)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/base)

add_executable(test2 ${SRCS})

target_link_libraries(test2 -lpthread -luuid)
