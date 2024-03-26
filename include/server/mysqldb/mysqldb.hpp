#ifndef MYSQLDB_H
#define MYSQLDB_H
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include <string>
#include <iostream>
using namespace std;


class MySQL
{
public:
    // 初始化数据库连接
    MySQL();

    // 释放数据库连接资源
    ~MySQL();

    // 连接数据库
    bool connect();

    // 更新操作
    bool update(string sql);

    // 查询操作
    MYSQL_RES *query(string sql);

    MYSQL* getConnection(){ return _conn;};

private:
    MYSQL *_conn;
};

#endif