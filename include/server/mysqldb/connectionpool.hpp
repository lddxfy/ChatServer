#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <queue>
using namespace std;
#include "mysqldb.hpp"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <memory>

class ConnectionPool
{
public:
    static ConnectionPool* getConnectionPool();


     //获取一个连接
    shared_ptr<MySQL> getConnection();

    
    
private:
    ConnectionPool();

    ~ConnectionPool();

    // 负责生产的线程的回调函数
    void produceConnectionTask();

   
    // 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
    void scannerConnectionTask();

    string _ip;
    int _port;
    string _username;
    string _password;
    string _dbname;
    int _initsize;  //连接池的初始连接量
    int _maxsize;   //连接池的最大连接量
    int _maxIdleTime;   //连接池的最大空闲时间
    int _connectionTimeout; //连接池获取连接的超时时间
    queue<MySQL*> _connectionQue; //存储mysql连接的队列
    mutex queueMutex;  //  维护连接队列的线程安全互斥锁
    condition_variable cv;  //生产者-消费者条件变量
    atomic_int ConnCount;   //原子变量，记录连接数
    atomic<bool> shutdownFlag; // 控制线程退出的标志
};




#endif  