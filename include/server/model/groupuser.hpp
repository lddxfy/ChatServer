#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"
#include <iostream>
#include <string>
using namespace std;
class GroupUser : public User
{
public:
    void setRole(string role){ this->grouprole = role;}
    string getRole(){ return this->grouprole; }
protected:
    string grouprole;
};

#endif