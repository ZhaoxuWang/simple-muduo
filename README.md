# simple-muduo
这是在阅读了《Linux多线程服务端编程》后自己写的一个简单的多线程服务器框架

- stage1 实现类基本的Reactor框架 https://blog.csdn.net/weixin_38880337/article/details/116532533
- stage2 实现一个单线程非阻塞TCP网络编程库 https://blog.csdn.net/weixin_38880337/article/details/116533001
- stage3 将单线程扩展到多线程 https://blog.csdn.net/weixin_38880337/article/details/116533053


在example文件夹中，有几个测试用例：

- chat_server,chat_client：是一个单聊天室的用例
- pub，sub，hub：是一个提供订阅功能的广播服务
- multi_chat:是一个简单的多聊天室例子(待完善)

待完成：

- 使用定时器关闭空闲连接
- 日志系统
- 压力测试
