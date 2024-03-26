#ifndef PUBLIC_H
#define PUBLIC_H

enum msgtype{
    LOGIN_ID = 1,   //用户登录消息
    REG_ID,      //用户注册消息
    LOGIN_ID_ACK,  //用户登录响应
    REG_ID_ACK,     //用户注册响应
    ONE_CHAT_MSG,   //一对一聊天消息    {"msgid":5,"name":"cjq","id":1,"toid":2,"msg":"hello123!"}
    ADD_FRIEND_MSG, //添加好友消息
    CREATE_GROUP_MSG,   //创建群组消息
    ADD_GROUP_MSG,      //加入群组消息   
    GROUP_CHAT_MSG,     //群聊消息
    LOGIN_OUT_MSG,      //账户注销消息
};


#endif