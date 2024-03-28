#include "friendmodel.hpp"
#include "mysqldb.hpp"
#include "connectionpool.hpp"

// 添加好友
void FriendModel::addfrined(int userid, int friendid, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d,%d)", userid, friendid);

    mysql->update(sql);
}
// 查询好友信息
vector<User> FriendModel::query(int userid, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid=%d", userid);

    vector<User> vec;

    MYSQL_RES *res = mysql->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            User user;
            user.setId(stoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            vec.push_back(user);
        }
        mysql_free_result(res);
        return vec;
    }

    return vec;
}