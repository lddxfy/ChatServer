#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include <iostream>
#include <vector>
#include "mysqldb.hpp"

using namespace std;
class OfflineMsgModel
{
public:
    //插入离线消息
    void insert(int userid,string msg,shared_ptr<MySQL> mysql);

    //删除离线消息
    void remove(int userid,shared_ptr<MySQL> mysql);

    //读取离线消息
    vector<string> query(int userid,shared_ptr<MySQL> mysql);

private:
    vector<string> offlinemsg;

};

#endif