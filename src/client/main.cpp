#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 控制主菜单页面程序
bool isMainMenuRunning = false;

// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSuccess{false};

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int);
// 显示当前登录成功用户的基本信息
void showCurrentUserData();

// 处理注册的响应逻辑
void doRegResponse(json &responsejs)
{
    if (0 != responsejs["errno"].get<int>())
    {
        cerr << "用户名已存在,注册失败" << endl;
    }
    else
    {
        cout << "用户注册成功,id号为:" << responsejs["id"] << ",请务必牢记！" << endl;
    }
}
// 处理登录的响应逻辑
void doLoginResponse(json &js);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "命令存在问题！请以如下格式输入：./ChatClient 127.0.0.1 8888" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "客户端socket创建失败" << endl;
        exit(-1);
    }

    sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(sockaddr_in));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr *)&serveraddr, sizeof(sockaddr_in)))
    {
        cerr << "连接服务器失败" << endl;
        close(clientfd);
        exit(-1);
    }

    // 连接服务器成功，启动接收子线程
    thread readTask(readTaskHandler, clientfd);
    readTask.detach();
    // main线程用于接收用户输入，负责发送数据
    while (true)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. 登录" << endl;
        cout << "2. 注册" << endl;
        cout << "3. 退出" << endl;
        cout << "========================" << endl;
        cout << "你的选择:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "id号:" << endl;
            cin >> id;
            cin.get();
            cout << "密码:" << endl;
            cin.getline(pwd, 50); // 可以接收空格

            json js;
            js["msgid"] = LOGIN_ID;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            g_isLoginSuccess = false;

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            
            if (len == -1)
            {
                cerr << "发送登录请求失败:" << request << endl;
            }
            sem_wait(&rwsem);// 等待信号量，由子线程处理完登录的响应消息后，通知这里

            if(g_isLoginSuccess)
            {
                // 进入聊天主菜单页面
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }

            
        }
        break;
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "用户名:";
            cin.getline(name, 50);
            cout << "密码：";
            cin.getline(pwd, 50);
            json js;
            js["msgid"] = REG_ID;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "发送注册消息失败:" << request << endl;
            }
            sem_wait(&rwsem);
        }
        break;
        case 3:
        {
            close(clientfd);
            exit(0);
        }

        default:
            cerr << "非法输入！" << endl;
            break;
        }
    }
    return 0;
}

void readTaskHandler(int clientfd)
{
    while (true)
    {
        char buf[1024] = {0};
        int len = recv(clientfd, buf, 1024, 0);
        if (len < 0)
        {
            cerr << "接收服务器消息失败";
            close(clientfd);
            exit(-1);
        }
        json js = json::parse(buf);
        //cout << js << endl;
        int msgidtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgidtype)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
        }

        if (GROUP_CHAT_MSG == msgidtype)
        {
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
        }

        if (LOGIN_ID_ACK == msgidtype)
        {
            doLoginResponse(js);
            sem_post(&rwsem);// 通知主线程，登录结果处理完成
            continue;
        }

        if (REG_ID_ACK == msgidtype)
        {
            doRegResponse(js);
            sem_post(&rwsem);// 通知主线程，注册结果处理完成
            continue;
        }
    }
}
// 登录响应
void doLoginResponse(json &js)
{
    int _errno = js["errno"].get<int>();
    if (_errno != 0)
    {
        // 登录失败
        cerr << js["errmsg"] << endl;
        g_isLoginSuccess = false;
    }
    else
    {
        g_currentUser.setId(js["id"].get<int>());
        g_currentUser.setName(js["name"]);

        // 记录当前用户的好友列表信息
        if (js.contains("friendinfo"))
        {
            g_currentUserFriendList.clear();
            cout << "test1" << endl;
            vector<string> vec = js["friendinfo"];
            for (string &str : vec)
            {
                json js1 = json::parse(str);
                User user;
                user.setId(js1["friendid"].get<int>());
                user.setName(js1["friendname"]);
                user.setState(js1["friendstate"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        // 记录当前用户的群组信息
        if (js.contains("groups"))
        {
            g_currentUserGroupList.clear();

            vector<string> vec = js["groups"];
            for (string &str : vec)
            {
                json js1 = json::parse(str);
                Group group;
                group.setId(js1["id"]);
                group.setgroupname(js1["groupname"]);
                group.setgroupdesc(js1["groupdesc"]);

                vector<string> vec2 = js1["users"];
                for (string &userstr : vec2)
                {
                    json userjs = json::parse(userstr);
                    GroupUser user;
                    user.setId(userjs["id"].get<int>());
                    user.setName(userjs["name"]);
                    user.setState(userjs["state"]);
                    user.setRole(userjs["role"]);
                    group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
        }

        showCurrentUserData();

        // 显示当前用户的离线消息
        if (js.contains("offlinemsg"))
        {
            cout << "test3" << endl;
            vector<string> vec = js["offlinemsg"];

            for (string &str : vec)
            {
                json js1 = json::parse(str);
                if (ONE_CHAT_MSG == js1["msgid"].get<int>())
                {
                    cout << js1["time"].get<string>() << " [" << js1["id"] << "]" << js1["name"].get<string>()
                         << " said: " << js1["msg"].get<string>() << endl;
                }
                else if (GROUP_CHAT_MSG == js1["msgid"].get<int>())
                {
                    cout << "群消息[" << js1["groupid"] << "]:" << js1["time"].get<string>() << " [" << js1["id"] << "]" << js1["name"].get<string>()
                         << " said: " << js1["msg"].get<string>() << endl;
                }
            }
        }
        g_isLoginSuccess = true;
    }
}
// 展示当前登录用户的信息
void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "当前登录用户 => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getgroupname() << " " << group.getgroupdesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

void mainMenu(int clientfd)
{
    help();
    //用于接收命令的缓冲区
    char buf[1024] = {0};
    while(isMainMenuRunning)
    {
        cin.getline(buf,1024);
        string commandbuf(buf);
        string command;
        int idx = commandbuf.find(":");
        if(-1 == idx)    //help或者是loginout
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0,idx); //分离出函数名
        }
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr<<"输入的命令不合法！"<<endl;
            continue;
        }
        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        string args = commandbuf.substr(idx+1,commandbuf.size() - idx);
        it->second(clientfd,args);
    }
}
//查看所有命令
void help(int fd, string str)
{
    cout << "命令列表：" << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}
//json:{"msgid":5,"name":"cjq","id":1,"toid":2,"msg":"hello123!"}    friendid:message
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if(-1 == idx)
    {
        cerr<<"聊天命令有误"<<endl;
        return;
    }

    int friendid = atoi(str.substr(0,idx).c_str());
    string msg = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["name"] = g_currentUser.getName();
    js["id"] = g_currentUser.getId();
    js["toid"] = friendid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len)
    {
        cerr<<"发送消息失败！"<<endl;
    }
}
//json:{"msgid":6,"userid":1,"friendid":2}   command:  friendid
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["userid"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len)
    {
        cerr<<"添加好友失败！"<<endl;
    }

}
// json: {"msgid":7,"id":1,"groupname":"","groupdesc":""}  command: groupname:groupdesc
void creategroup(int clientfd, string str)
{
    int id = g_currentUser.getId();
    int idx = str.find(":");
    if(-1 == idx)
    {
        cerr<<"创建群聊命令有误"<<endl;
        return;
    }
    string groupname = str.substr(0,idx);
    string groupdesc = str.substr(idx+1,str.size()-idx);
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = id;
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len)
    {
        cerr<<"创建群聊失败！"<<endl;
    }

    

}
//json: {"msgid":8,"id":1,"groupid":""}  command: groupid
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
   
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    cout<<"test1"<<endl;
    if(-1 == len)
    {
        cerr<<"添加群聊失败！"<<endl;
    }

}
//json: {"msgid":9,"name":"","id":1,"groupid":,"message":""}  command: groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if(-1 == idx)
    {
        cerr<<"聊天命令有误"<<endl;
        return;
    }

    int groupid = atoi(str.substr(0,idx).c_str());
    string msg = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    js["name"] = g_currentUser.getName();
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len)
    {
        cerr<<"发送群聊消息失败！"<<endl;
    }
}
//json: {"msgid":10,"id":}  command:  loginout
void loginout(int clientfd, string str)
{
    int id = g_currentUser.getId();
    json js;
    js["msgid"] = LOGIN_OUT_MSG;
    js["id"] = id;
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len)
    {
        cerr<<"账户注销失败！"<<endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

string getCurrentTime()
{
    auto tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}
