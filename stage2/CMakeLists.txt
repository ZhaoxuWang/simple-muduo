cmake_minimum_required(VERSION 3.5)

set(SRCS
    utils.cc
    Buffer.cc
    InetAddress.cc
    Socket.cc
    Acceptor.cc
    TcpConnection.cc
    TcpServer.cc
    Channel.cc
    Epoll.cc
    EventLoop.cc
    EventLoopThread.cc
    test1.cc
    timer.cc
    base/Thread.cc
)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/base)

add_executable(test1 ${SRCS})

target_link_libraries(test1 -lpthread -luuid)
