#include "chatserver.hpp"
#include "chatservice.hpp"
#include <signal.h>

void resetHandler(int)
{
    ChatService::GetInstance()->reset();
    exit(0);
};

int main(int argc,char **argv){
    
    if(argc < 3)
    {
        cerr<<"command invalid ! example: ./ChatServer 127.0.0.1 6000";
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    //服务器异常退出，重置用户数据
    signal(SIGINT,resetHandler);

    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    return 0;
}