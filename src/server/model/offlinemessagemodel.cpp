#include "offlinemessagemodel.hpp"
#include "mysqldb.hpp"

// 插入离线消息
void OfflineMsgModel::insert(int userid, string msg, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

    mysql->update(sql);
}

// 删除离线消息
void OfflineMsgModel::remove(int userid, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);

    mysql->update(sql);
}

// 读取离线消息
vector<string> OfflineMsgModel::query(int userid, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);
    vector<string> vec;

    MYSQL_RES *res = mysql->query(sql);
    if (res != nullptr)
    {
        // 查询成功
        // 获取查询出来的行
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            vec.push_back(row[0]);
        }
        mysql_free_result(res);
        return vec;
    }

    return vec;
}