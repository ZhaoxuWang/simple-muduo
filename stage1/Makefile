all: Thread.o Channel.o Epoll.o timer.o EventLoop.o test1

Thread.o: base/Thread.cc base/Thread.h 
	g++ -c base/Thread.cc -o base/Thread.o -lpthread -std=c++11
Channel.o: Channel.cc Channel.h
	g++ -c Channel.cc -o Channel.o -lpthread -std=c++11
Epoll.o: Epoll.cc Epoll.h
	g++ -c Epoll.cc -o Epoll.o -lpthread -std=c++11
timer.o: timer.cc timer.h utils.h
	g++ -c timer.cc -o timer.o -lpthread -std=c++11
EventLoop.o: EventLoop.cc EventLoop.h
	g++ -c EventLoop.cc -o EventLoop.o -lpthread -std=c++11
test1: base/CurrentThread.h Thread.o test1.cc Channel.o Epoll.o timer.o EventLoop.o
	g++ base/CurrentThread.h Channel.o Epoll.o timer.o EventLoop.o test1.cc -o test1 -lpthread -std=c++11

clean:
	rm *.o test1
