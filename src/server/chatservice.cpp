#include "chatservice.hpp"
#include "public.hpp"
#include <functional>
#include "muduo/base/Logging.h"
ChatService *ChatService::GetInstance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    //向消息处理表插入对应的键值{消息，回调函数}
    msgHandlermap.insert({LOGIN_ID, bind(&ChatService::login, this, _1, _2, _3)});
    msgHandlermap.insert({REG_ID, bind(&ChatService::reg, this, _1, _2, _3)});
    msgHandlermap.insert({ONE_CHAT_MSG, bind(&ChatService::oneChat, this, _1, _2, _3)});
    msgHandlermap.insert({ADD_FRIEND_MSG, bind(&ChatService::addfriend, this, _1, _2, _3)});
    msgHandlermap.insert({CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, _1, _2, _3)});
    msgHandlermap.insert({ADD_GROUP_MSG, bind(&ChatService::addGroup, this, _1, _2, _3)});
    msgHandlermap.insert({GROUP_CHAT_MSG, bind(&ChatService::GroupChat, this, _1, _2, _3)});
    msgHandlermap.insert({LOGIN_OUT_MSG, bind(&ChatService::LoginOut, this, _1, _2, _3)});
    //建立redis缓存服务器连接，并初始化 处理返回订阅信息 的回调函数
    if(_redis.connect())
    {
        _redis.init_notify_handler(bind(&ChatService::HandleRedisSubscribeMessage,this,_1,_2));
    }
}
//处理登录业务，返回登录响应
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int id = js["id"].get<int>();
    string passwd = js["password"];
    User user;
    user = usermodel.query(id);
    if (user.getId() == id && user.getPasswd() == passwd)
    {
        if (user.getState() == "online")
        {
            json response;
            response["msgid"] = LOGIN_ID_ACK;
            response["errmsg"] = "该用户已登录，请重新输入";
            response["errno"] = 3;
            conn->send(response.dump());
        }
        else
        {
            // 登录成功,作出响应
            // 保证线程安全
            {
                lock_guard<mutex> lock(connlock);
                userconnmap.insert({user.getId(), conn});
            }
            // id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);

            // 更新用户状态
            user.setState("online");
            usermodel.updateState(user);
            json response;
            response["msgid"] = LOGIN_ID_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 发送离线消息
            vector<string> vec = offlinemsgmodel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                offlinemsgmodel.remove(user.getId());
            }

            // 展示好友信息
            vector<User> uservec = friendmodel.query(id);

            if (!uservec.empty())
            {
                vector<string> userfriendinfovec;
                for (auto user : uservec)
                {
                    json js;
                    js["friendid"] = user.getId();
                    js["friendname"] = user.getName();
                    js["friendstate"] = user.getState();
                    userfriendinfovec.push_back(js.dump());
                }
                response["friendinfo"] = userfriendinfovec;
            }

            // 查询用户的群组信息
            vector<Group> groupuservec = groupmodel.queryGroups(id);
            if (!groupuservec.empty())
            {
                vector<string> groupv;
                for (Group &group : groupuservec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getgroupname();
                    grpjson["groupdesc"] = group.getgroupdesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupv.push_back(grpjson.dump());
                }
                response["groups"] = groupv;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败,作出响应
        json response;
        response["msgid"] = LOGIN_ID_ACK;
        response["errmsg"] = "id或密码错误";
        response["errno"] = 2;
        conn->send(response.dump());
    }
}
//处理注册业务，返回注册响应
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    // 获取外部传来的数据
    string name = js["name"];
    string password = js["password"];
    // 将数据封装
    User user;
    user.setName(name);
    user.setPasswd(password);
    // 插入数据库中
    bool flag = usermodel.insert(user);
    if (flag)
    {
        // 插入用户成功,作出响应
        json response;
        response["msgid"] = REG_ID_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 插入用户失败,作出响应
        json response;
        response["msgid"] = REG_ID_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}
//获取对应事件的处理器
msgHandler ChatService::GetHandler(int msgId)
{
    if (msgHandlermap.find(msgId) == msgHandlermap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgId:" << msgId << "处理器没有找到";
        };
    }
    return msgHandlermap[msgId];
}
// 从用户连接表中去除连接，然后修改数据库中的用户状态
void ChatService::clientExitException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(connlock);
        for (auto it = userconnmap.begin(); it != userconnmap.end(); it++)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                userconnmap.erase(it);
            }
        }
    }
    _redis.unsubscribe(user.getId());
    user.setState("offline");
    usermodel.updateState(user);
}
// 一对一聊天服务 从json中解析出要发送给哪个id,根据id在连接表中找到相应连接向其发送消息
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(connlock);
        auto it = userconnmap.find(toid);
        if (it != userconnmap.end())
        {
            // toid在线，转发消息,服务器主动推送消息给toid
            it->second->send(js.dump());
            return;
        }
    }
    //因为可能不在一个服务器，所以从数据库查询toid是否在线
    User user = usermodel.query(toid);
    if(user.getState() == "online")
    {
        _redis.publish(toid,js.dump());
        return;
    }

    // toid不在线，存储离线消息
    offlinemsgmodel.insert(toid, js.dump());
}
// 添加好友
void ChatService::addfriend(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["userid"].get<int>();
    int frinedid = js["friendid"].get<int>();
    friendmodel.addfrined(userid, frinedid);
}
//重新设置用户的在线状态
void ChatService::reset()
{
    usermodel.resetState();
}
//创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1, name, desc);
    if (groupmodel.CreateGroup(group))
    {
        groupmodel.AddGroup(userid, group.getId(), "creator");
    }
}
//加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    cout<<"test2"<<endl;
    int userid = js["id"].get<int>();   
    int groupid = js["groupid"].get<int>();
    groupmodel.AddGroup(userid, groupid, "normal");
}
//进行群聊
void ChatService::GroupChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> groupusersid = groupmodel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(connlock);
    for (int id : groupusersid)
    {
        auto it = userconnmap.find(id);
        if (it != userconnmap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            //因为可能不在一个服务器，所以从数据库查询toid是否在线
            User user = usermodel.query(id);
            if(user.getState()=="online")
            {
                _redis.publish(id,js.dump());
            }
            else
            {
                offlinemsgmodel.insert(id,js.dump());
            }
        }
    }
    
}
//用户注销
void ChatService::LoginOut(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int id = js["id"].get<int>();
    {
        lock_guard<mutex> lock(connlock);
        auto it = userconnmap.find(id);
        if (it != userconnmap.end())
        {
            userconnmap.erase(it);
        }
    }
    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(id); 
    User user(id,"","","offline");
    usermodel.updateState(user);
}

//转发其他服务器向订阅的通道发送的消息
void ChatService::HandleRedisSubscribeMessage(int id,string msg)
{
    lock_guard<mutex> lock(connlock);
    auto it = userconnmap.find(id);
    if(it != userconnmap.end())
    {
        it->second->send(msg);
        return;
    }

    offlinemsgmodel.insert(id,msg);
}