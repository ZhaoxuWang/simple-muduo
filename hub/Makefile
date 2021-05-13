all: pub sub hub multi_chat clean
.PHONY : all 

VPATH = net:net/base
tail = -lpthread -std=c++11
objects = Acceptor.o Buffer.o Channel.o Connector.o Epoll.o EventLoop.o EventLoopThread.o \
          EventLoopThreadPool.o InetAddress.o Socket.o TcpClient.o TcpConnection.o TcpServer.o \
          timer.o utils.o Thread.o

pub: $(objects) pubsub.o codec.o pub.cc
	g++ -o pub $(objects) pubsub.o codec.o pub.cc $(tail)
sub: $(objects) pubsub.o codec.o sub.cc
	g++ -o sub $(objects) pubsub.o codec.o sub.cc $(tail)
hub: $(objects) pubsub.o codec.o hub.cc
	g++ -o hub $(objects) pubsub.o codec.o hub.cc $(tail)
multi_chat: $(objects) pubsub.o codec.o multi_chat.cc
	g++ -o multi_chat $(objects) pubsub.o codec.o multi_chat.cc $(tail)

pubsub.o: pubsub.cc pubsub.h codec.h TcpClient.h noncopyable.h
	g++ -c $< $(tail)
codec.o: codec.cc codec.h noncopyable.h Buffer.h TcpConnection.h
	g++ -c $< $(tail)

Acceptor.o: Acceptor.cc Acceptor.h EventLoop.h InetAddress.h Socket.h
	g++ -c $< $(tail)
Buffer.o: Buffer.cc Buffer.h
	g++ -c $< $(tail)
Channel.o: Channel.cc Channel.h EventLoop.h
	g++ -c $< $(tail)
Connector.o: Connector.cc Connector.h InetAddress.h timer.h noncopyable.h
	g++ -c $< $(tail)
Epoll.o: Epoll.cc Epoll.h EventLoop.h
	g++ -c $< $(tail)
EventLoop.o: EventLoop.cc EventLoop.h timer.h Channel.h Epoll.h utils.h
	g++ -c $< $(tail)
EventLoopThread.o: EventLoopThread.cc EventLoopThread.h EventLoop.h
	g++ -c $< $(tail)
EventLoopThreadPool.o: EventLoopThreadPool.cc EventLoopThreadPool.h EventLoop.h EventLoopThread.h
	g++ -c $< $(tail)
InetAddress.o: InetAddress.cc InetAddress.h
	g++ -c $< $(tail)
Socket.o: Socket.cc Socket.h InetAddress.h
	g++ -c $< $(tail)
TcpClient.o: TcpClient.cc TcpClient.h Connector.h EventLoop.h Socket.h
	g++ -c $< $(tail)
TcpConnection.o: TcpConnection.cc TcpConnection.h Channel.h EventLoop.h Socket.h
	g++ -c $< $(tail)
TcpServer.o: TcpServer.cc TcpServer.h Acceptor.h EventLoop.h Socket.h Callbacks.h \
             EventLoopThreadPool.h
	g++ -c $< $(tail)
timer.o: timer.cc timer.h EventLoop.h utils.h
	g++ -c $< $(tail)
utils.o: utils.cc utils.h 
	g++ -c $< $(tail)
Thread.o: Thread.cc Thread.h CurrentThread.h
	g++ -c $< $(tail)


clean:
	rm *.o