#include "usermodel.hpp"
#include "mysqldb.hpp"
#include <iostream>
#include <string>
using namespace std;
bool UserModel::insert(User &user, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name,password,state) values('%s', '%s', '%s')", user.getName().c_str(), user.getPasswd().c_str(), user.getState().c_str());

    if (mysql->update(sql))
    {
        // 获取插入成功数据库自动生成User对象的主键id
        user.setId(mysql_insert_id(mysql->getConnection()));
        return true;
    }

    return false;
}

User UserModel::query(int id, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = '%d'", id);

    MYSQL_RES *res = mysql->query(sql);
    if (res != nullptr)
    {
        // 查询成功
        // 获取查询出来的行
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row != nullptr)
        {
            User user;
            user.setId(stoi(row[0]));
            user.setName(row[1]);
            user.setPasswd(row[2]);
            user.setState(row[3]);
            mysql_free_result(res);
            return user;
        }
    }

    return User();
}

bool UserModel::updateState(User &user, shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where id = '%d'", user.getState().c_str(), user.getId());

    if (mysql->update(sql))
    {
        return true;
    }

    return false;
}

void UserModel::resetState(shared_ptr<MySQL> mysql)
{
    // 1.组装sql语句
    char sql[1024] = "update User set state = 'offline' where state = 'online'";
    mysql->update(sql);
}