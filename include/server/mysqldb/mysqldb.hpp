#ifndef MYSQLDB_H
#define MYSQLDB_H
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include <string>
#include <iostream>
#include <thread>
using namespace std;


class MySQL
{
public:
    // 初始化数据库连接
    MySQL();

    // 释放数据库连接资源
    ~MySQL();

    // 连接数据库
    bool connect(string server,int port,string user,string password,string dbname);

    // 更新操作
    bool update(string sql);

    // 查询操作
    MYSQL_RES *query(string sql);

    MYSQL* getConnection(){ return _conn;};

    // 刷新一下连接的起始的空闲时间点
	void refreshAliveTime() { _alivetime = clock(); }
	// 返回存活的时间
	clock_t getAliveeTime()const { return clock() - _alivetime; }

private:
    MYSQL *_conn;
    clock_t _alivetime;
};

#endif