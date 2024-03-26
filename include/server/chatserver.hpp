#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <iostream>
#include <string>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace std;
using namespace muduo::net;
using namespace muduo;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);

    void start()
    {
        _server.start();
    }

private:

    void OnMessage(const TcpConnectionPtr &, Buffer *,
                      Timestamp);

    void OnConnection(const TcpConnectionPtr &);

    TcpServer _server;
    EventLoop *_loop;
};

#endif