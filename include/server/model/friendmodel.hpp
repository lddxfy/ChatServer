#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <iostream>
#include <vector>
#include "user.hpp"
#include "mysqldb.hpp"
using namespace std;
class FriendModel
{
public:
    //添加好友
    void addfrined(int userid,int friendid,shared_ptr<MySQL> mysql);
    //查询好友信息
    vector<User> query(int userid,shared_ptr<MySQL> mysql);
};


#endif