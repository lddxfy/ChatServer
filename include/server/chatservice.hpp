#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <unordered_map>
#include <iostream>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
#include <mutex>
using json = nlohmann::json;
using namespace muduo::net;
using namespace muduo;
using namespace std;
using msgHandler = function<void(const TcpConnectionPtr& conn, json& js,Timestamp)>;
class ChatService
{
public:
    
    static ChatService* GetInstance();
    void login(const TcpConnectionPtr &conn, json &js,Timestamp);
    void reg(const TcpConnectionPtr &conn, json &js,Timestamp);
    void clientExitException(const TcpConnectionPtr &conn);
    void oneChat(const TcpConnectionPtr& conn, json& js,Timestamp);
    void reset();   //重置用户数据
    void addfriend(const TcpConnectionPtr &conn, json &js,Timestamp);
    void createGroup(const TcpConnectionPtr &conn, json &js,Timestamp);
    void addGroup(const TcpConnectionPtr &conn, json &js,Timestamp);
    void GroupChat(const TcpConnectionPtr &conn, json &js,Timestamp);
    void LoginOut(const TcpConnectionPtr &conn, json &js,Timestamp);
    void HandleRedisSubscribeMessage(int id,string msg);
    msgHandler GetHandler(int msgId);
private:
    ChatService();
    unordered_map<int,msgHandler> msgHandlermap;
    UserModel usermodel;
    mutex connlock;
    unordered_map<int,TcpConnectionPtr> userconnmap;    //存储每个用户连接的信息
    OfflineMsgModel offlinemsgmodel;
    FriendModel friendmodel;
    GroupModel groupmodel;
    Redis _redis;
};

#endif