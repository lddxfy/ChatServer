#include "connectionpool.hpp"

static string ip = "127.0.0.1";
static int port = 3306;
static string username = "root";
static string password = "123456";
static string dbname = "chatserver";
static int initsize = 2;    // 连接池的初始连接量
static int maxsize = 1024;     // 连接池的最大连接量
static int maxIdleTime = 60; // 连接池的最大空闲时间
static int connectionTimeout = 100;

ConnectionPool::ConnectionPool() : _ip(ip),_port(port),_username(username),
_password(password),_dbname(dbname),_initsize(initsize),_maxsize(maxsize),_maxIdleTime(maxIdleTime),
_connectionTimeout(connectionTimeout),shutdownFlag(false)

{
   

    for (int i = 1; i <= _initsize; i++)
    {
        MySQL *conn = new MySQL();
        conn->connect(_ip, _port, _username, _password, _dbname);
        conn->refreshAliveTime();
        _connectionQue.push(conn);
        ConnCount++;
    }

    thread produce(bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();

    thread scanner(bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

ConnectionPool::~ConnectionPool()
{
    unique_lock<mutex> lock(queueMutex);
    if(_connectionQue.size() > 0)
    {
        while(!_connectionQue.empty())
        {
            MySQL *con = _connectionQue.front();
            _connectionQue.pop();
            delete con;
        }
    }

    shutdownFlag.store(true); // 通知线程退出
    // 通知所有等待的线程
    cv.notify_all();
}

void ConnectionPool::produceConnectionTask()
{
    while (!shutdownFlag.load())
    {
        unique_lock<mutex> lock(queueMutex);
        while (!_connectionQue.empty())
        {
            // 生产者等待
            cv.wait(lock);
        }

        if (ConnCount < _maxsize)
        {
            MySQL *conn = new MySQL();
            conn->connect(_ip, _port, _username, _password, _dbname);
            conn->refreshAliveTime();
            _connectionQue.push(conn);
            ConnCount++;
        }
        // 生产者通知消费者消费
        cv.notify_all();
    }
}

// 线程安全的懒汉模式
ConnectionPool *ConnectionPool::getConnectionPool()
{
    static ConnectionPool connpool;
    return &connpool;
}


// 消费者获取连接
shared_ptr<MySQL> ConnectionPool::getConnection()
{

    unique_lock<mutex> lock(queueMutex);
    while (_connectionQue.empty())
    {
        if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
        {
            if (_connectionQue.empty())
            {
                cout << "获取空闲连接超时,获取连接失败" << endl;
                return nullptr;
            }
        }
    }
    shared_ptr<MySQL> conn(_connectionQue.front(), [&](MySQL *conn)
                                {
                                    unique_lock<mutex> lock(queueMutex);
                                    conn->refreshAliveTime();
                                    _connectionQue.push(conn); });
    _connectionQue.pop();
    // 消费完后通知生产者进行检查，若队列为空，赶紧生产连接
    cv.notify_all();
    return conn;
}
// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
void ConnectionPool::scannerConnectionTask()
{
    while (!shutdownFlag.load())
    {
        // 通过sleep模拟定时效果 每60s扫描一次
        this_thread::sleep_for(chrono::seconds(60));
        unique_lock<mutex> lock(queueMutex);
        while (ConnCount > _initsize)
        {
            MySQL *conn = _connectionQue.front();
            if (conn->getAliveeTime() >= _maxIdleTime * 1000)
            {
                _connectionQue.pop();
                ConnCount--;
                delete conn;
            }
            else
            {
                // 队头的连接没有超过_maxIdleTime，其它连接肯定没有
                break;
            }
        }
    }
}