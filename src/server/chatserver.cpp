#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
#include <iostream>

using namespace placeholders;
using namespace std;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : _server(loop, listenAddr, nameArg), _loop(loop)
{
    //设置连接事件回调
    _server.setConnectionCallback(bind(&ChatServer::OnConnection, this, _1));
    //设置收发消息事件回调
    _server.setMessageCallback(bind(&ChatServer::OnMessage, this, _1, _2, _3));
    _server.setThreadNum(4);
}
//收发消息事件回调
void ChatServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buffer,Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //cout<<buf<<endl;
    json js = json::parse(buf);
    //解耦 在网络层中没有出现业务层方法
    auto handler = ChatService::GetInstance()->GetHandler(js["msgid"].get<int>());
    handler(conn,js,time);
}
//连接事件回调
void ChatServer::OnConnection(const TcpConnectionPtr &conn)
{
    if(!conn->connected()){
        ChatService::GetInstance()->clientExitException(conn);
        conn->shutdown();
    }
}