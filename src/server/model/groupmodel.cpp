#include "groupmodel.hpp"
#include "mysqldb.hpp"

// 加入群组
void GroupModel::AddGroup(int userid, int groupid, string role, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values('%d','%d','%s')", groupid, userid, role.c_str());

    mysql->update(sql);
}
// 创建群组
bool GroupModel::CreateGroup(Group &group, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s', '%s')", group.getgroupname().c_str(), group.getgroupdesc().c_str());
    if (mysql->update(sql))
    {
        // 获取插入成功数据库自动生成User对象的主键id
        group.setId(mysql_insert_id(mysql->getConnection()));
        return true;
    }

    return false;
}
// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid, shared_ptr<MySQL> mysql)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on b.groupid = a.id where b.userid=%d", userid);
    vector<Group> groupvec;

    MYSQL_RES *res = mysql->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            Group group;
            group.setId(stoi(row[0]));
            group.setgroupname(row[1]);
            group.setgroupdesc(row[2]);
            groupvec.push_back(group);
        }
        mysql_free_result(res);
    }

    // 查询群组的用户信息
    for (Group &group : groupvec)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid = a.id where b.groupid=%d", group.getId());
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser groupuser;
                groupuser.setId(stoi(row[0]));
                groupuser.setName(row[1]);
                groupuser.setState(row[2]);
                groupuser.setRole(row[3]);
                group.getUsers().push_back(groupuser);
            }
            mysql_free_result(res);
        }
    }
    return groupvec;
}
// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid, shared_ptr<MySQL> mysql)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid=%d and userid != %d", groupid, userid);

    vector<int> usersidvec;

    MYSQL_RES *res = mysql->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            usersidvec.push_back(atoi(row[0]));
        }
        mysql_free_result(res);
    }

    return usersidvec;
}