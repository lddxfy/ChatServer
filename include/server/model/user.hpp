#ifndef USER_H
#define USER_H
#include <iostream>
using namespace std;
#include<string>

class User
{
public:
    User(int id = -1,string name = "",string password = "",string state = "offline")
    {
        this->id = id;
        this->passwd = password;
        this->name = name;
        this->state = state;
    }

    string getName(){ return this->name; }
    string getPasswd(){ return this->passwd; }
    string getState(){ return this->state; }
    int getId(){ return this->id; }

    void setName(string name){ this->name = name;}
    void setPasswd(string password){ this->passwd = password;}
    void setId(int id){this->id = id;}
    void setState(string state){this->state = state;}
private:
    int id;
    string name;
    string passwd;
    string state;
};

#endif