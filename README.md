# ChatServer
使用c++开发的基于muduo网络库，Nginx tcp负载均衡实现的集群聊天服务器和客户端
## 项目所涉及的技术栈
Json序列化和反序列化
muduo网络库开发
nginx的tcp负载均衡器配置
redis缓存服务器编程
基于发布-订阅的服务器中间件redis消息队列编程
MySQL数据库编程
数据库连接池
CMake构建编译环境
## 模块信息
### 服务端：
src/server/model:存放各个表的model的实现方法
src/server/mysqldc:项目中调用数据库的实现方法
src/server/redis:用于集群服务器进行通信的redis消息队列的实现方法
src/server/chatserver.cpp:项目网络层
src/server/chatservice.cpp:项目业务层
include:存放各个模块的头文件
### 客户端：
include:与服务端公用头文件
src/client/main.cpp:客户端的主要代码
### 其他：
bin:可执行文件目录
build：存放编译链接文件目录
autubuild.sh：自动编译脚本
## 运行环境
ubuntu linux环境，安装boost+muduo网络库，安装redis环境，mysql环境，nginx环境，cmake环境
## 单机运行
./autobuild
cd bin
./ChatServer [ip][port]
./ChatClient [ip][port]
## 项目缺陷
1.没有图形化界面
2.json库暂时不支持中文聊天输入中文时可能会报错或者无法收到
3.还有许多的业务可以扩展，例如：添加好友时的验证，好友在线信息的及时更新和提示等等。
## 我的体会
通过该项目使我理解掌握服务器的网络I/O模块，业务模块，数据模块分层的设计思想以及C++ muduo网络库的编程应用
和nginx配置部署tcp负载均衡器的应用以及原理更加熟悉，也对服务器中间件的应用场景和基于发布-订阅的redis
编程实践以及应用原理更加熟悉




