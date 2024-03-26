#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include <iostream>
#include <vector>

using namespace std;
class OfflineMsgModel
{
public:
    //插入离线消息
    void insert(int userid,string msg);

    //删除离线消息
    void remove(int userid);

    //读取离线消息
    vector<string> query(int userid);

private:
    vector<string> offlinemsg;

};

#endif