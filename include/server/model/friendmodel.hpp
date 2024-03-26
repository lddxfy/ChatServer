#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <iostream>
#include <vector>
#include "user.hpp"
using namespace std;
class FriendModel
{
public:
    //添加好友
    void addfrined(int userid,int friendid);
    //查询好友信息
    vector<User> query(int userid);
};


#endif