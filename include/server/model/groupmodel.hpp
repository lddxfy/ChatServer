#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"

class GroupModel
{
public:
    //加入群组
    void AddGroup(int userid,int groupid,string role);
    //创建群组
    bool CreateGroup(Group &group);
    //查询用户所在群组信息
    vector<Group> queryGroups(int userid);
    // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
    vector<int> queryGroupUsers(int userid, int groupid);

};

#endif